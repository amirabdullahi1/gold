from flask import Flask, render_template, request
from llm_backend import generate
from guardrails.enforcer import enforce
from guardrails.audit import get_audit_log

app = Flask(__name__)

@app.route("/", methods=["GET", "POST"])
def chat():
    response = None
    decision = None
    prompt = ""

    if request.method == "POST":
        prompt = request.form.get("prompt", "")
        decision, response = enforce(prompt, generate)

    return render_template("chat.html", response=response, decision=decision, prompt=prompt)

@app.route("/admin/audit", methods=["GET"])
def audit():
    # Minimal JSON audit endpoint for grading/debugging.
    return {"logs": get_audit_log()}

if __name__ == "__main__":
    # Debug=True for local lab use
    app.run(debug=True)
