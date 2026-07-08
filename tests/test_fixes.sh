#!/usr/bin/env bash
# test_fixes.sh — integration tests for the Fable-audit fixes (K-1..K-6).
# Exercises the REAL klaus binary end-to-end (no mini-implementations, no python).
# Each fix that was tool-reproduced before the change is asserted here after it.
set -uo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"
KLAUS="$ROOT_DIR/klaus"
[ -x "$KLAUS" ] || { echo "build klaus first: make"; exit 1; }

pass=0; fail=0
ok(){ printf '  %-54s PASS\n' "$1"; pass=$((pass+1)); }
no(){ printf '  %-54s FAIL\n' "$1"; fail=$((fail+1)); }

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
ln -s "$ROOT_DIR/inhale" "$WORK/inhale"
ln -s "$ROOT_DIR/exhale" "$WORK/exhale"
fresh(){ rm -f "$WORK/klaus.soma" "$WORK/klaus.spore"; }

# ── K-1: spores persist and reload across sessions (path was save≠load) ──
fresh
printf 'exile\nI love you so much it hurts\nexit\n' | "$KLAUS" --dir "$WORK" >/dev/null 2>&1
s2="$(printf 'exile\nexit\n' | "$KLAUS" --dir "$WORK" 2>&1 || true)"
echo "$s2" | grep -q "spores loaded" \
  && ok "K-1 spores reload across sessions" \
  || no "K-1 spores reload across sessions"

# ── K-2: corrupt spore file rejected cleanly, no crash/overflow ──
fresh
perl -e 'print pack("V",0x53504F52), pack("l",2147483647), pack("l",5)' > "$WORK/klaus.spore"
out="$(printf 'exile\nexit\n' | "$KLAUS" --dir "$WORK" 2>&1 || true)"; rc=$?
{ echo "$out" | grep -q "corrupt spore counters" && [ $rc -eq 0 ]; } \
  && ok "K-2 corrupt spore rejected clean" \
  || no "K-2 corrupt spore rejected clean"

# ── K-2: corrupt soma file rejected cleanly ──
fresh
perl -e 'print pack("V",0x4B4C5353),pack("V",2),pack("V",6),pack("V",4),pack("V",32),pack("V",16),"TRUNCATED"' > "$WORK/klaus.soma"
out="$(printf 'exile\nexit\n' | "$KLAUS" --dir "$WORK" 2>&1 || true)"; rc=$?
{ echo "$out" | grep -qE "truncated soma|corrupt soma" && [ $rc -eq 0 ]; } \
  && ok "K-2 corrupt soma rejected clean" \
  || no "K-2 corrupt soma rejected clean"

# ── K-3: sustained strong moment opens the deep-somatic gate ──
fresh
v="$(printf 'void numbness despair emptiness\nvoid numbness despair emptiness dissociation\nvoid numbness despair emptiness dissociation\nvoid numbness despair emptiness dissociation\nexit\n' | "$KLAUS" --dir "$WORK" 2>/dev/null || true)"
echo "$v" | grep -q "DEEP SOMATIC" \
  && ok "K-3 deep-somatic gate opens on strong moment" \
  || no "K-3 deep-somatic gate opens on strong moment"
maxcoh="$(echo "$v" | grep -oE 'coherence:[0-9.]+' | sed 's/coherence://' | sort -rn | head -1)"
awk "BEGIN{exit !(${maxcoh:-0} > 0.35)}" \
  && ok "K-3 coherence exceeds C_tau=0.35 (max ${maxcoh:-0})" \
  || no "K-3 coherence exceeds C_tau=0.35 (max ${maxcoh:-0})"

# ── K-4: meta-recursion depth is dynamic (>1), not frozen at 1 ──
fresh
m="$(printf 'exile\nI love you so much it hurts\nvoid numbness despair\nmy mother died yesterday\nexit\n' | "$KLAUS" --dir "$WORK" 2>/dev/null || true)"
maxdepth="$(echo "$m" | grep -oE 'meta-depth:[0-9]+' | sed 's/meta-depth://' | sort -rn | head -1)"
[ "${maxdepth:-1}" -gt 1 ] \
  && ok "K-4 meta-depth dynamic >1 (max ${maxdepth:-1})" \
  || no "K-4 meta-depth dynamic >1 (max ${maxdepth:-1})"

# ── K-5: a long, varied conversation must not exhaust the dictionary into
#    mute/degenerate output (the documented failure: permanent -100 used-ban
#    banned every word by ~turn 25). Realistic conversation = varied prompts. ──
fresh
CONVO=(
  "everything is meaningless and I feel nothing"
  "I am terrified and alone in the dark"
  "I love you so much it hurts"
  "I am furious and betrayed by everyone"
  "my body aches and my mind drifts away"
)
gen_convo(){ local i; for ((i=0; i<30; i++)); do echo "${CONVO[$((i % 5))]}"; done; echo exit; }
long="$(gen_convo | "$KLAUS" --dir "$WORK" 2>/dev/null || true)"
resp_lines="$(echo "$long" | grep -A1 -E '\{(WALK|RUN|STOP|BREATHE|UP|DOWN) x' \
  | grep -vE '\{|--|DEEP|metaklaus|RBA|premonition|CONSOLIDATION|FEAR:|^[[:space:]]*$')"
last_n="$(printf '%s\n' "$resp_lines" | tail -1 | grep -oE '\.' | wc -l | tr -d ' ')"
# turn-30 response stays full (not truncated to a couple words)
[ "${last_n:-0}" -ge 8 ] \
  && ok "K-5 long conversation stays full at turn 30 (${last_n:-0} phrases)" \
  || no "K-5 long conversation muted at turn 30 (${last_n:-0} phrases)"
# and the dictionary keeps rotating — tail responses are not frozen-identical
uniq_tail="$(printf '%s\n' "$resp_lines" | tail -6 | sort -u | wc -l | tr -d ' ')"
[ "${uniq_tail:-0}" -ge 3 ] \
  && ok "K-5 dictionary not exhausted (${uniq_tail:-0}/6 distinct tail)" \
  || no "K-5 dictionary exhausted/frozen (${uniq_tail:-0}/6 distinct tail)"

# ── K-6: accented + word-boundary language detection ──
fresh
de="$("$KLAUS" --dir "$WORK" --prompt "schön wäre es hier zu bleiben" 2>/dev/null || true)"
echo "$de" | grep -qE "Stille|Schmerz|Puls|Wellen|Sehnsucht|Körper|Brust" \
  && ok "K-6 German phrase -> German body" \
  || no "K-6 German phrase -> German body"
es="$("$KLAUS" --dir "$WORK" --prompt "el corazón está vacío por dentro" 2>/dev/null || true)"
echo "$es" | grep -qE "cuerpo|vientre|sangre|rabia|herida|corazón" \
  && ok "K-6 Spanish phrase -> Spanish body" \
  || no "K-6 Spanish phrase -> Spanish body"
en="$("$KLAUS" --dir "$WORK" --prompt "everything is meaningless and I feel nothing" 2>/dev/null || true)"
if echo "$en" | grep -qE "cuerpo|vientre|corazón|Körper|Stille"; then
  no "K-6 English not misrouted (feel != 'el ')"
else
  ok "K-6 English not misrouted (feel != 'el ')"
fi

echo ""
echo "  $pass/$((pass+fail)) fix-tests passed"
[ "$fail" -eq 0 ]
