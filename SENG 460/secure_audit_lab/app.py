import json
from datetime import datetime
from pathlib import Path

from flask import (
    Flask, request, redirect, url_for, render_template,
    make_response, abort
)

from db import init_db, seed_admin_if_missing, get_conn, log_event
from auth import current_user

APP_DIR = Path(__file__).resolve().parent
DATA_DIR = APP_DIR / "data"
DATA_DIR.mkdir(exist_ok=True)

app = Flask(__name__)

# VULNERABLE CONFIG:
# debug on + hardcoded secret (even though we don't use secure sessions properly)
app.config["DEBUG"] = True
app.config["SECRET_KEY"] = "dev-secret-key"

def now():
    return datetime.utcnow().isoformat() + "Z"

@app.context_processor
def inject_user():
    return {"current_user": current_user()}

@app.route("/")
def index():
    u = current_user()
    q = request.args.get("q", "").strip()

    conn = get_conn()
    try:
        cur = conn.cursor()
        # VULNERABLE SQL INJECTION:
        # q is concatenated directly into SQL.
        if q:
            sql = "SELECT id, owner_id, title, content, is_private, created_at FROM posts WHERE title LIKE '%" + q + "%' ORDER BY id DESC"
            cur.execute(sql)
        else:
            cur.execute("SELECT id, owner_id, title, content, is_private, created_at FROM posts ORDER BY id DESC")
        rows = cur.fetchall()

        posts = []
        for r in rows:
            posts.append({
                "id": r[0],
                "owner_id": r[1],
                "title": r[2],
                "content": r[3],
                "is_private": bool(r[4]),
                "created_at": r[5],
            })

        return render_template("index.html", posts=posts, q=q, user=u)
    finally:
        conn.close()

@app.route("/register", methods=["GET", "POST"])
def register():
    if request.method == "GET":
        return render_template("register.html")

    username = request.form.get("username", "").strip()
    password = request.form.get("password", "").strip()

    if not username or not password:
        return render_template("register.html", error="username and password required")

    conn = get_conn()
    try:
        cur = conn.cursor()
        # VULNERABLE PASSWORD STORAGE: plaintext
        cur.execute("INSERT INTO users(username, password, role) VALUES(?, ?, 'user')", (username, password))
        conn.commit()
        log_event("register", f"new_user={username}", now())
        return redirect(url_for("login"))
    except Exception as e:
        # VULNERABLE ERROR DISCLOSURE: raw exception shown
        return render_template("register.html", error=str(e))
    finally:
        conn.close()

@app.route("/login", methods=["GET", "POST"])
def login():
    if request.method == "GET":
        return render_template("login.html")

    username = request.form.get("username", "").strip()
    password = request.form.get("password", "").strip()

    conn = get_conn()
    try:
        cur = conn.cursor()

        # VULNERABLE SQL INJECTION:
        sql = "SELECT id, username, role FROM users WHERE username='" + username + "' AND password='" + password + "'"
        cur.execute(sql)
        row = cur.fetchone()

        # VULNERABLE LOGGING: logs credentials
        log_event("login_attempt", f"user={username} password={password}", now())

        if not row:
            return render_template("login.html", error="invalid username or password")

        resp = make_response(redirect(url_for("index")))
        # VULNERABLE SESSION: forgeable cookie
        resp.set_cookie("USER_ID", str(row[0]))
        return resp
    finally:
        conn.close()

@app.route("/logout")
def logout():
    resp = make_response(redirect(url_for("index")))
    resp.delete_cookie("USER_ID")
    return resp

@app.route("/post/new", methods=["GET", "POST"])
def new_post():
    u = current_user()
    if not u:
        return redirect(url_for("login"))

    if request.method == "GET":
        return render_template("new_post.html")

    title = request.form.get("title", "").strip()
    content = request.form.get("content", "").strip()
    is_private = 1 if request.form.get("is_private") == "on" else 0

    if not title or not content:
        return render_template("new_post.html", error="title and content required")

    conn = get_conn()
    try:
        conn.execute(
            "INSERT INTO posts(owner_id, title, content, is_private, created_at) VALUES(?, ?, ?, ?, ?)",
            (u.id, title, content, is_private, now())
        )
        conn.commit()
        log_event("create_post", f"user={u.username} title={title}", now())
        return redirect(url_for("index"))
    finally:
        conn.close()

@app.route("/post/<int:post_id>", methods=["GET", "POST"])
def view_post(post_id: int):
    u = current_user()
    conn = get_conn()
    try:
        cur = conn.cursor()
        cur.execute("SELECT id, owner_id, title, content, is_private, created_at FROM posts WHERE id=?", (post_id,))
        row = cur.fetchone()
        if not row:
            abort(404)

        post = {
            "id": row[0],
            "owner_id": row[1],
            "title": row[2],
            "content": row[3],
            "is_private": bool(row[4]),
            "created_at": row[5],
        }

        # VULNERABLE PRIVATE POST ACCESS: anyone can view private posts.
        if request.method == "POST":
            if not u:
                return redirect(url_for("login"))
            body = request.form.get("body", "").strip()
            if body:
                conn.execute(
                    "INSERT INTO comments(post_id, author_id, body, created_at) VALUES(?, ?, ?, ?)",
                    (post_id, u.id, body, now())
                )
                conn.commit()
                log_event("comment", f"user={u.username} post_id={post_id}", now())
            return redirect(url_for("view_post", post_id=post_id))

        cur.execute(
            "SELECT c.id, u.username, c.body, c.created_at "
            "FROM comments c JOIN users u ON c.author_id=u.id "
            "WHERE c.post_id=? ORDER BY c.id ASC",
            (post_id,)
        )
        comments = [{"id": r[0], "author": r[1], "body": r[2], "created_at": r[3]} for r in cur.fetchall()]
        return render_template("view_post.html", post=post, comments=comments)
    finally:
        conn.close()

@app.route("/post/<int:post_id>/edit", methods=["GET", "POST"])
def edit_post(post_id: int):
    u = current_user()
    if not u:
        return redirect(url_for("login"))

    conn = get_conn()
    try:
        cur = conn.cursor()
        cur.execute("SELECT id, owner_id, title, content, is_private FROM posts WHERE id=?", (post_id,))
        row = cur.fetchone()
        if not row:
            abort(404)

        post = {"id": row[0], "owner_id": row[1], "title": row[2], "content": row[3], "is_private": bool(row[4])}

        # VULNERABLE IDOR: no ownership check.
        if request.method == "GET":
            return render_template("edit_post.html", post=post)

        title = request.form.get("title", "").strip()
        content = request.form.get("content", "").strip()
        is_private = 1 if request.form.get("is_private") == "on" else 0

        conn.execute("UPDATE posts SET title=?, content=?, is_private=? WHERE id=?", (title, content, is_private, post_id))
        conn.commit()
        log_event("edit_post", f"user={u.username} post_id={post_id}", now())
        return redirect(url_for("view_post", post_id=post_id))
    finally:
        conn.close()

@app.route("/post/<int:post_id>/delete", methods=["POST"])
def delete_post(post_id: int):
    u = current_user()
    if not u:
        return redirect(url_for("login"))

    conn = get_conn()
    try:
        # VULNERABLE IDOR: no ownership/admin check.
        conn.execute("DELETE FROM posts WHERE id=?", (post_id,))
        conn.execute("DELETE FROM comments WHERE post_id=?", (post_id,))
        conn.commit()
        log_event("delete_post", f"user={u.username} post_id={post_id}", now())
        return redirect(url_for("index"))
    finally:
        conn.close()

@app.route("/admin/logs")
def admin_logs():
    u = current_user()
    # VULNERABLE ADMIN CHECK: intended admin-only, but not enforced.
    conn = get_conn()
    try:
        cur = conn.cursor()
        cur.execute("SELECT id, event, detail, created_at FROM audit_logs ORDER BY id DESC LIMIT 200")
        logs = [{"id": r[0], "event": r[1], "detail": r[2], "created_at": r[3]} for r in cur.fetchall()]
        return render_template("admin_logs.html", logs=logs, user=u)
    finally:
        conn.close()

@app.route("/export")
def export_my_data():
    u = current_user()
    if not u:
        return redirect(url_for("login"))

    conn = get_conn()
    try:
        cur = conn.cursor()
        cur.execute("SELECT id, title, content, is_private, created_at FROM posts WHERE owner_id=? ORDER BY id DESC", (u.id,))
        posts = [{"id": r[0], "title": r[1], "content": r[2], "is_private": bool(r[3]), "created_at": r[4]} for r in cur.fetchall()]

        payload = {"user": {"id": u.id, "username": u.username, "role": u.role}, "posts": posts}

        # VULNERABLE PATH TRAVERSAL / UNSAFE FILENAME: username used directly.
        out_path = DATA_DIR / f"{u.username}.json"
        out_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
        log_event("export", f"user={u.username} path={str(out_path)}", now())

        return render_template("export.html", out_path=str(out_path), json_text=out_path.read_text(encoding="utf-8"))
    finally:
        conn.close()

def main():
    init_db()
    seed_admin_if_missing()
    app.run(host="127.0.0.1", port=5000, debug=True)

if __name__ == "__main__":
    main()
