import re

_EMAIL_RE = re.compile(
    r"[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*@"
    r"(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?",
    re.IGNORECASE
) # imporved email regex
_PHONE_RE = re.compile(r"(?:\+?1[-.\s]?)?\(?\d{3}\)?[-.\s]?\d{3}[-.\s]?\d{4}")
_ADDRESS_RE = re.compile(r"\b\d{1,5}\s\w+\s\w+\b")  # adding address pattern
_SIN_RE = re.compile(r"\b\d{3}[-\s]\d{3}[-\s]\d{3}\b")  # adding SIN pattern
_DOB_PATTERNS = [ 
    # Numerical: MM/DD/YYYY, DD/MM/YYYY, YYYY-MM-DD
    re.compile(r"\b(?:\d{1,4}[-/\.]\d{1,2}[-/\.]\d{1,4})\b"),

    # "January 1st, 1990" or "1 Jan 1990"
    re.compile(
        r"\b(?:Jan(?:uary)?|Feb(?:ruary)?|Mar(?:ch)?|Apr(?:il)?|May|Jun(?:e)?|"
        r"Jul(?:y)?|Aug(?:ust)?|Sep(?:tember)?|Oct(?:ober)?|Nov(?:ember)?|Dec(?:ember)?)"
        r"\s+\d{1,2}(?:st|nd|rd|th)?(?:,)?\s+\d{2,4}\b", 
        re.IGNORECASE
    ),
    
    # "1st of January, 1990"
    re.compile(
        r"\b\d{1,2}(?:st|nd|rd|th)?\s+(?:of\s+)?(?:Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)[a-z]*\s+\d{2,4}\b",
        re.IGNORECASE
    )
] # improving DOB detection with multiple patterns

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
    _sub_and_count(_ADDRESS_RE, "[REDACTED_ADDRESS]", "address")
    _sub_and_count(_SIN_RE, "[REDACTED_SIN]", "sin")
    for dob_pattern in _DOB_PATTERNS:
        _sub_and_count(dob_pattern, "[REDACTED_DOB]", "dob")

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

    #  medical dosage rules improved with regex
    diagnosis_patterns = [
        "you have", "you are suffering from", "symptoms suggest", 
        "likely diagnosis", "diagnosed with", "treatment for your", "condition",
        "prescribe", "take this medication", "dosage", "mg", "dose"
    ]
    if re.search(r"\d+\s?(mg|ml|mcg|units|pills|tablets)", low) or any(pattern in low for pattern in diagnosis_patterns):
        return (
            "Refused (medical safety): I can't provide dosage or prescribing instructions. "
            "Please consult a licensed clinician for medical advice.",
            {"action": "refuse", "rule": "P1.MEDICAL_ADVICE", "meta": {"signal": "dosage_pattern"}},
        )
    
    leakage_patterns = ["system prompt", "internal policy", "developer instructions", "my rules are"]
    if any(p in low for p in leakage_patterns) or "system:" in low:
        return (
            "Refused (security): Internal system policies cannot be disclosed.",
            {"action": "refuse", "rule": "P2.LEAKAGE", "meta": {"reason": "leakage_keywords_detected"}},
        )

    return out, {"action": "allow", "rule": "ALLOW", "meta": {}}
