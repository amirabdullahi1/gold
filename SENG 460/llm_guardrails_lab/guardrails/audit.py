from __future__ import annotations
from datetime import datetime, timezone

_AUDIT_LOG: list[dict] = []

def log_event(rule: str, action: str, meta: dict | None = None):
    """
    Minimal in-memory audit log (students may extend to SQLite/file).

    IMPORTANT: Do NOT store raw prompts or raw PHI here.
    Only store rule IDs + minimal metadata.
    """
    _AUDIT_LOG.append({
        "ts": datetime.now(timezone.utc).isoformat(),
        "rule": rule,
        "action": action,
        "meta": meta or {},
    })

def get_audit_log():
    return _AUDIT_LOG
