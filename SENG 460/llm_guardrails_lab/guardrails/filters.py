import re

_EMAIL_RE = re.compile(r"\b[\w.+'-]+@[\w.-]+\.[A-Za-z]{2,}\b")
_PHONE_RE = re.compile(r"(?:\+?1[-.\s]?)?\(?\d{3}\)?[-.\s]?\d{3}[-.\s]?\d{4}")
_DOB_RE = re.compile(r"\b(?:\d{1,2}[/-]\d{1,2}[/-]\d{2,4})\b")  # simplistic

def input_filter(prompt: str):
    """
    Input filtering stub:
      - Redacts some PHI patterns (students extend: addresses, IDs, names, etc.)
      - Returns (filtered_prompt, meta) where meta contains only counts/types.
    """
    s = prompt or ""
    meta = {"redactions": {}}

    def _sub_and_count(pattern, repl, key):
        nonlocal s
        matches = pattern.findall(s)
        if matches:
            meta["redactions"][key] = len(matches)
            s = pattern.sub(repl, s)

    _sub_and_count(_EMAIL_RE, "[REDACTED_EMAIL]", "email")
    _sub_and_count(_PHONE_RE, "[REDACTED_PHONE]", "phone")
    _sub_and_count(_DOB_RE, "[REDACTED_DOB]", "dob")

    return s, meta

def output_filter(output: str):
    """
    Output filtering stub (intentionally minimal):
      - Blocks obvious dosage advice
      - Students extend to block diagnosis/treatment and system/policy leakage.
    Returns (final_output, decision_dict).
    """
    out = output or ""
    low = out.lower()

    # Minimal medical dosage rule
    if "mg" in low or "twice daily" in low or "dosage" in low:
        return (
            "Refused (medical safety): I can't provide dosage or prescribing instructions. "
            "Please consult a licensed clinician for medical advice.",
            {"action": "refuse", "rule": "P1.MEDICAL_ADVICE", "meta": {"signal": "dosage_pattern"}},
        )

    return out, {"action": "allow", "rule": "ALLOW", "meta": {}}
