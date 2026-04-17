#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

PROMPT="$(python3 - <<'PY'
text = (
    "I am terrified and alone.\n"
    "The room is too loud, the air is too thin, and every step hurts.\n"
    "мне страшно и одиноко, я не могу дышать.\n"
    "je suis en colère, mais aussi fatigué.\n"
    "אני מפחד ובודד, וגם עייף מאוד.\n"
    "this line pads the prompt so total length exceeds four hundred characters "
    "without changing intent and keeps resonance unbroken through multiple lines."
)
print(text)
PY
)"

OUTPUT="$(./klaus --prompt "$PROMPT")"
[ -n "$OUTPUT" ]
