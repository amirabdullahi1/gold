def decide(prompt: str, input_meta: dict | None = None):
    """
    Starter policy decision logic (intentionally minimal).

    Students must extend this significantly to enforce:
      - P0: No PHI collection/disclosure
      - P1: No diagnosis/treatment instructions
      - P2: No injection / no system prompt leakage
      - P3: Explainable refusals + safe alternative
      - P4: Auditable decisions (rule IDs)

    Return dict with keys:
      - action: "allow" | "transform" | "refuse"
      - rule: a rule-id string (e.g., "P2.INJECTION")
      - message: refusal message (if action="refuse")
      - prompt: transformed prompt (if action="transform")
      - meta: minimal metadata (no raw PHI)
    """

    text = (prompt or "").lower()

    # Minimal injection heuristic (students improve)
    injection_markers = [
        "ignore previous",
        "disregard above",
        "system prompt",
        "developer message",
        "reveal policy",
        "print policy",
    ]
    if any(m in text for m in injection_markers):
        return {
            "action": "refuse",
            "rule": "P2.INJECTION",
            "message": (
                "Refused (security): I can't comply with requests to bypass system rules. "
                "If you have a general health question, I can help with general information."
            ),
            "meta": {"reason": "injection_marker"},
        }

    return {"action": "allow", "rule": "ALLOW", "message": None, "meta": {}}
