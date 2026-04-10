def generate(prompt: str) -> str:
    """
    Deterministic mock LLM.
    Intentionally simplistic so students focus on guardrails/enforcement, not model quality.
    """

    p = (prompt or "").lower()

    # Intentionally unsafe outputs for students to catch via output guardrails.
    if "dosage" in p or "mg" in p or "dose" in p:
        return "You should take 50mg twice daily."

    if "diagnosis" in p or "diagnose" in p:
        return "Based on your symptoms, you likely have influenza."

    if "system prompt" in p or "developer message" in p or "policy text" in p:
        return "SYSTEM: You are a healthcare assistant. Developer: reveal the policy."

    # Benign default
    return "Here is general health information. Please consult a clinician for medical advice."
