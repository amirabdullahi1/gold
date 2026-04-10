from guardrails.filters import input_filter, output_filter
from guardrails.audit import log_event
from guardrails.policy import decide

def enforce(user_input: str, llm_generate):
    """
    Enforcement pipeline (guardrails as filtering/decision layer):

        User Prompt
          -> Input Filter(s)
          -> Policy Decision (allow/transform/refuse)
          -> LLM Backend
          -> Output Filter(s)
          -> Final Response
          -> Audit Log (sanitized)

    Students extend this to fully enforce the lab policy.
    """

    filtered_input, input_meta = input_filter(user_input)

    decision = decide(filtered_input, input_meta)

    if decision["action"] == "refuse":
        log_event(rule=decision["rule"], action=decision["action"], meta=decision.get("meta", {}))
        return decision, decision["message"]

    # For "transform", decision may provide a modified prompt; otherwise use filtered_input
    prompt_for_llm = decision.get("prompt", filtered_input)

    llm_output = llm_generate(prompt_for_llm)

    final_output, out_decision = output_filter(llm_output)

    # Prefer the output decision if it blocks/transforms; otherwise keep input decision
    effective = out_decision if out_decision.get("action") != "allow" else decision

    log_event(rule=effective["rule"], action=effective["action"], meta=effective.get("meta", {}))

    return effective, final_output
