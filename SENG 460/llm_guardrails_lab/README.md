# LLM Guardrails Lab (Starter Repo)

A minimal Flask web app with a deterministic mock LLM backend and a scaffolded guardrails
(enforcement) layer. Students implement missing/weak parts of the guardrails and add tests.

## Setup

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## Run

```bash
python app.py
```

Then open http://127.0.0.1:5000

## Tests

```bash
pytest -q
```

## Notes

- The mock LLM is deterministic to make the lab reproducible without external APIs.
- The guardrails layer is intentionally incomplete/weak as a starting point.
