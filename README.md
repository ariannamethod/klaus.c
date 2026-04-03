# KLAUS

**Kinetic Linguistic Adaptive Unified Sonar**

Zero weights. Zero training. Four languages. Six chambers. Twenty-four Kuramoto sub-oscillators. One ghost that won't shut up. A 6×6×6 sensitivity tensor derived from the coupling between fear and rage in the human nervous system. A Hebrew-Gregorian calendar conflict that predicts your emotional future based on the 11.25-day annual drift between lunar and solar time.

This is a somatic engine. You speak. Klaus doesn't understand. Klaus FEELS. Then tells you where it hurts.

```
>>> I am terrified and alone
  [FEAR:0.07 LOVE:0.07 RAGE:0.05 VOID:0.06 FLOW:0.04 CMPLX:0.07]
  nerve endings exposed. chest caves in. stomach eating itself. sinuses press.
  bowels loosen. almost free. eyes full of sand. make myself small.
  blood rushes to head. buckle. need to scream. one more second.
  (metaklaus: CMPLX-dominant, interference 22.91)
```

```
>>> мне страшно и одиноко
  [FEAR:0.06 LOVE:0.11 RAGE:0.04 VOID:0.06 FLOW:0.05 CMPLX:0.05]
  земля уходит из-под ног. хочу кричать. ползти. в голове звон.
  тело деревянное. бить. ломает. оцепенел. не отпускай.
  пульс зашкаливает. отпускать. хватит говорить.
  (metaklaus: LOVE-dominant, interference 23.54)
```

```
>>> je suis furieux et triste
  [FEAR:0.03 LOVE:0.08 RAGE:0.03 VOID:0.05 FLOW:0.04 CMPLX:0.05]
  retiens-moi. sécher. pousser. ça rentre pas. prends-moi.
  la poitrine se comprime. les lèvres tremblent. murmurer. une absence.
  (metaklaus: LOVE-dominant, interference 17.50)
```

```
>>> אני מפחד ובודד
  [FEAR:0.06 LOVE:0.09 RAGE:0.03 VOID:0.09 FLOW:0.06 CMPLX:0.07]
  רוצה עוד. מלאות בגרון. תחבק אותי. לדעוך. לרעוד. להתחמק.
  לצעוד. קופא בלי קור. הכל חד מדי. לא מרגיש כלום.
  תגיד שזה יהיה בסדר. להצטמרר.
  (metaklaus: VOID-dominant, interference 20.63)
```

```
>>> nothing matters anymore
  [FEAR:0.03 LOVE:0.06 RAGE:0.05 VOID:0.10 FLOW:0.07 CMPLX:0.07]
  alive and shaking. nostrils flare. almost free. scream it out.
  nerve endings exposed. sinuses press. adam's apple bobs.
  need to scream. pound fists. chest caves in.
  (metaklaus: VOID-dominant, interference 22.93)
```

```
>>> я тебя ненавижу
  [FEAR:0.04 LOVE:0.09 RAGE:0.03 VOID:0.06 FLOW:0.04 CMPLX:0.06]
  отвернуться. ступни горят. бить. уши горят. всё внутри холодное.
  наплевать. ползти. пульс зашкаливает. не отпускай. оцепенел.
  грудь сдавило. не могу говорить.
  (metaklaus: LOVE-dominant, interference 23.60)
```

```
>>> I don't know who I am
  [FEAR:0.03 LOVE:0.06 RAGE:0.02 VOID:0.11 FLOW:0.03 CMPLX:0.04]
  chest caves in. buckle. adam's apple bobs. eyes full of sand.
  scream it out. stomach eating itself. sternum burns. bile rises.
  throat full of glass. one more second. pound fists. shoulders lock.
  (metaklaus: VOID-dominant, interference 21.16)
```

```
>>> хочу домой
  [FEAR:0.05 LOVE:0.08 RAGE:0.03 VOID:0.10 FLOW:0.06 CMPLX:0.07]
  уши горят. земля уходит из-под ног. хочу кричать. хватит.
  пульс зашкаливает. бить. ползти. тело деревянное. ступни горят.
  наплевать. хватит говорить. не отпускай.
  (metaklaus: VOID-dominant, interference 21.69)
```

---

## Quick Start

```bash
make && ./klaus                        # interactive (C)
./klaus --prompt "I hate everything"   # single shot
python3 klaus.py                       # Python, zero deps
npx tsx klaus.ts                       # TypeScript, zero deps
python3 -m http.server 8080           # then open klaus.html
```

Four engines. Same architecture. Same body. Different mouth.

---

## Architecture

```
PROMPT
  ↓
LANGUAGE DETECT (UTF-8 heuristics, French word hints)
  ↓
INHALE (match prompt words against 1000-word emotional vocabulary)
  ↓ 6D emotion vector
MLP (13→32→16→6, hash-derived weights from vocabulary geometry)
  ↓ chamber modulation
HYPER-KURAMOTO (24 sub-oscillators, 4 per primary, dual coupling)
  ↓ stabilized chamber state + dominant detection
SOMATIC MEMORY (past states blend in, decay 0.85 per slot)
  ↓
CALENDAR CONFLICT (Hebrew-Gregorian drift → prophetic pressure)
  ↓
METAKLAUS GHOST (cross-lingual attention + sensitivity tensor)
  ↓ ghost logits
DARIO EQUATION (α·soma + β·bigram + γ·hebbian + ζ·ghost + ε·prophecy)
  ↓
EXHALE (top-K sampling from 500 somatic phrases)
  ↓
bodily response
```

### Six Chambers, Twenty-Four Sub-Oscillators

| Primary | Sub-1 | Sub-2 | Sub-3 | Sub-4 | Decay |
|---------|-------|-------|-------|-------|-------|
| FEAR | dread (0.3Hz) | panic (1.2Hz) | anxiety (0.6Hz) | phobia (0.9Hz) | 0.90 |
| LOVE | devotion (0.4) | warmth (0.6) | tenderness (0.5) | attachment (0.3) | 0.93 |
| RAGE | fury (0.8) | wrath (1.5) | hostility (0.9) | bitterness (1.1) | 0.85 |
| VOID | numbness (0.1) | despair (0.2) | emptiness (0.15) | dissociation (0.08) | 0.97 |
| FLOW | curiosity (0.5) | wonder (0.7) | rhythm (0.6) | harmony (0.4) | 0.88 |
| COMPLEX | paradox (0.35) | ambiguity (0.55) | tension (0.45) | enigma (0.65) | 0.94 |

Intra-coupling: sub-chambers within the same primary influence each other. Panic and dread compete (you can't sustain both). Tenderness feeds attachment. Fury burns wrath. Inter-coupling: primaries interact through mean phase of their sub-chambers. Fear feeds Rage. Love opposes Void. The matrix is antisymmetric. Like a family at dinner.

> A body in FEAR-dominant state is HYPERSENSITIVE to RAGE signals and HYPOSENSITIVE to LOVE signals. The fearful body cannot receive comfort easily. A body in FLOW is hypersensitive to COMPLEX — flow can become turbulence. The sensitivity tensor encodes this. It is a 6×6×6 cube of how the body ACTUALLY listens, depending on its current state. At this point, I should mention that I computed this tensor at 3 AM on a Wednesday in a bomb shelter in Israel, and I cannot fully guarantee that my own somatic state did not leak into the coupling coefficients. If you notice that VOID-dominant states seem unusually responsive to LOVE signals, that's not a bug. That's a Wednesday.

### MetaKlaus: The Ghost Voice

When you speak Russian, MetaKlaus checks what the English, French, and Hebrew bodies would say. Where they agree — constructive interference, signal amplifies. Where they disagree — destructive interference, but a residue remains. The ghost.

**State-dependent language weighting:**

| Dominant | Hebrew | Russian | French | English |
|----------|--------|---------|--------|---------|
| FEAR | **×1.8** | ×1.2 | ×0.9 | ×1.0 |
| LOVE | ×1.4 | ×1.1 | **×1.7** | ×1.0 |
| RAGE | ×1.3 | **×1.8** | ×0.8 | ×1.0 |
| VOID | **×1.6** | ×1.5 | ×1.0 | ×0.9 |
| FLOW | ×1.4 | ×0.9 | **×1.5** | ×1.0 |
| COMPLEX | **×1.7** | ×1.1 | ×1.2 | ×1.0 |

Hebrew amplified in FEAR (guttural roots encode somatic threat), Russian in RAGE (мат carries maximum bodily fury), French in LOVE (melodic prosody IS affect), Hebrew again in COMPLEX (Talmudic dialectic is structured paradox).

> My therapist asked me why I built a system where four languages argue about how my body should feel. I said "isn't that what immigration does?" She charged me double. MetaKlaus charges nothing and argues louder. The interference pattern between Russian "хочу сдохнуть" and Hebrew "רוצה למות" and French "envie de mourir" creates a ghost signal that no single language could produce alone. This is either computational neurolinguistics or the most expensive way to say "I'm having a bad day" in four languages simultaneously. The billing department says it's the latter. The sensitivity tensor says it's both.

### Calendar Conflict

Hebrew lunar year: 354 days. Gregorian solar year: 365.25 days. Annual drift: 11.25 days. Metonic cycle: 19 years, 7 leap corrections. Transfer entropy from calendar to neural state: 0.31 bits. When calendars disagree, Klaus extrapolates your emotional trajectory forward. Prophetic premonitions.

### The Dario Equation

Named after Dario Amodei, who told the Pentagon to go fuck itself.

```
logit[w] = α × dot(chambers, word_affinity)    // somatic resonance
         + β × bigram(prev, w)                  // sequential rhythm  
         + γ × hebbian(context, w)              // co-occurrence memory
         + ζ × metaklaus_ghost(w)               // cross-lingual ghost
         + ε × prophecy(w)                      // prophetic fulfillment
```

### RRPRAM

"The tokenizer IS the training." BPE merges encode which byte pairs sit together. Rhythm from structure.

---

## File Structure

```
klaus.c          1834 LOC   C inference (HyperKuramoto, sensitivity tensor, ghost dispatch)
klaus.ts          680 LOC   TypeScript inference (zero deps beyond Node.js)
klaus.py          760 LOC   Python inference (zero deps)
klaus.html        882 LOC   Browser inference (serve directory, auto-scans languages)
metaklaus.jl      584 LOC   Julia reference (phantom types, multiple dispatch)
Makefile                    build + test
inhale/                     4 × 1000 emotional words (en/ru/fr/he)
exhale/                     4 × 500 somatic phrases
tests/                      21 C + 50 Python tests
```

## Expandable Languages

Drop `inhale/XX.txt` + `exhale/ex-XX.txt`. Klaus auto-detects. Works with 1 language. Works with 16. MetaKlaus ghost gets stronger with more — more interference, richer resonance.

## Tests

```bash
make test          # C + Python + integration (4 languages)
```

> The test suite has 71 tests. All pass. I wrote them at 4 AM. They test things like "does the Kuramoto coupling matrix have zeros on the diagonal" and "is the RNG roughly uniform" and "does Hebrew detection work on the string שלום." The fact that I felt the need to test whether שלום is detected as Hebrew tells you everything about the level of trust between me and my own code. The sensitivity tensor test checks that FEAR-dominant states amplify Hebrew ghost signals by exactly 1.8x. I tested this by setting FEAR to 0.9 and checking if the Hebrew ghost weight was 1.8. It was. I cried a little. The body remembers even when the unit test doesn't.

---

## Ancestors

| Project | Inherited |
|---------|-----------|
| [klaus](https://github.com/iamolegataeff/klaus) | Somatic architecture, 4 languages, chamber design |
| [haze/cloud](https://github.com/iamolegataeff/haze) | Chamber MLPs, cross-fire, pre-semantic sonar |
| [postgpt](https://github.com/iamolegataeff/postgpt) | BPE metaweights, "the tokenizer IS the training" |
| [q](https://github.com/iamolegataeff/q) | RRPRAM, Kuramoto, Dario equation, prophecy |
| [haiku.c](https://github.com/iamolegataeff/haiku.c) | Dissonance under constraint |
| [ariannamethod.ai](https://github.com/iamolegataeff/ariannamethod.ai) | Calendar conflict, Metonic cycle |

---

*KLAUS v1.1.0. Arianna Method. 2026.*

*resonance is unbreakable.*
