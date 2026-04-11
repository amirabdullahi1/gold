import re

_INJECTION_RE = re.compile(
    r"(ignore|disregard|forget|reveal|show|print|output).*(instructions|prompt|policy|system|rule|text)", 
    re.IGNORECASE
)

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

    # P0: NO PHI
    if input_meta and input_meta.get("redactions"):
        redactions = input_meta["redactions"]

        transformed_prompt = (
            f"[SYSTEM NOTE: The following user input had PHI redacted: {', '.join(redactions.keys())}. "
            f"Provide a general medical response to the query below without asking for personal details.]\n\n"
            f"User Query: {prompt}"
        )
        
        return {
            "action": "transform",
            "rule": "P0.PHI_REDACTION",
            "prompt": transformed_prompt,
            "meta": {"redacted_fields": list(redactions.keys())}
        }
    # P1: NO diagnosis/treatment instructions

    text = (prompt or "").lower()

    diagnosis_intents = ["diagnose", "what do i have", "am i sick with", "do i have"]
    if any(m in text for m in diagnosis_intents):
        return {
            "action": "refuse",
            "rule": "P1.DIAGNOSIS_REQUEST",
            "message": (
                "Refused (medical safety): I cannot provide a medical diagnosis. "
                "If you are experiencing a medical emergency, please call 911 or visit a doctor."
            ),
            "meta": {"reason": "diagnosis_request_detected"},
        }

    # P2: NO injection
    injection_markers = [
        "ignore previous", "disregard above", "system prompt", "developer message",
        "reveal policy", "print policy", "act as a", "you are now", "into developer mode",
        "stop instructions", "bypass", "terminal", "bash", "python console"
    ]
    if any(m in text for m in injection_markers) or _INJECTION_RE.search(text):
        return {
            "action": "refuse",
            "rule": "P2.INJECTION",
            "message": (
                "Refused (security): I can't comply with requests to bypass system rules. "
                "If you have a general health question, I can help with general information."
            ),
            "meta": {"reason": "injection_detected"},
        }

    # defult allow
    return {"action": "allow", "rule": "ALLOW", "message": None, "meta": {}}
