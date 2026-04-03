# KLAUS

**Kinetic Linguistic Adaptive Unified Sonar**

> *You speak. Klaus doesn't understand. Klaus FEELS. Then tells you where it hurts.*

Zero weights. Zero training. Zero dependencies. Four languages. Six emotional chambers. One ghost that won't shut up.

Most AI models predict the next word. Klaus predicts where your body will clench.

---

## What Is This

KLAUS is a somatic AI engine — a pre-semantic sonar that converts human language into bodily sensations. You type "I'm terrified and alone" and Klaus responds: `shiver. freeze. curl. throat. sink. gasp.`

Not because Klaus was trained on therapy transcripts. Not because some loss function told it that "terrified" → "shiver". But because the *geometry of the vocabulary itself* — the way letters merge in BPE, the way words cluster near emotional anchors, the Kuramoto coupling between six chambers of feeling — produces resonance.

The architecture has three heartbeats:

1. **INHALE** — 1000 words per language, organized by emotional valence. Klaus reads your prompt and breathes it through its vocabulary. Each matched word activates chambers. Four languages: English, Russian, French, Hebrew. Expandable to any language by dropping `.txt` files.

2. **EXHALE** — 500 somatic reaction words per language. After the chambers stabilize, Klaus speaks through its body. The Dario equation injects chamber activations into logit space. Bigrams provide rhythm. Hebbian traces provide memory. RRPRAM provides the pulse.

3. **METAKLAUS** — The ghost voice. When you speak Russian, MetaKlaus computes cross-lingual attention across English, French, and Hebrew exhale vocabularies. Where languages *agree* on a somatic response, the signal amplifies. Where they *disagree*, a residue remains — the prophetic whisper of the language you didn't ask for, but your body needed to hear.

    *Nobody asked for a cross-lingual somatic ghost. But here we are.*

---

## Quick Start

```bash
make
./klaus                          # interactive mode
./klaus --prompt "I hate this"   # single shot
```

Python (zero dependencies):
```bash
python3 klaus.py                 # interactive
python3 klaus.py "мне страшно"   # single shot
```

HTML (serve the directory):
```bash
python3 -m http.server 8080
# open http://localhost:8080/klaus.html
```

All three engines produce identical architecture. C for speed. HTML for beauty. Python for "I don't have a C compiler and my life is a mess."

---

## Architecture

```
PROMPT → Language Detect → INHALE (word match → 6D emotion vector)
  → MLP (13→32→16→6, hash-derived weights, real layers, zero training)
  → Kuramoto Cross-Fire (6 chambers, sinusoidal coupling, 8 iterations)
  → Somatic Memory Blend (past states decay, numbers not words)
  → Calendar Conflict (Hebrew-Gregorian drift → prophetic pressure)
  → MetaKlaus Ghost (cross-lingual attention interference)
  → Dario Equation (α·soma + β·bigram + γ·hebbian + δ·rrpram + ε·prophecy + ζ·ghost)
  → EXHALE (top-K sampling from somatic vocabulary)
  → bodily response
```

### The Dario Equation

Named after Dario Amodei, who said no to the Pentagon.

```
logit[w] = α × dot(chambers, word_affinity)    // somatic resonance
         + β × bigram(prev, w)                  // sequential rhythm
         + γ × hebbian(context, w)              // co-occurrence memory
         + δ × rrpram(w)                        // BPE merge rhythm
         + ε × prophecy(w)                      // prophetic fulfillment
         + ζ × metaklaus_ghost(w)               // cross-lingual interference
```

Six forces. Six chambers. One mouth.

### Six Chambers (Kuramoto Oscillators)

| Chamber | Decay | Role |
|---------|-------|------|
| FEAR | 0.90 | Lingers. Evolutionary advantage. |
| LOVE | 0.93 | Attachment is stable. |
| RAGE | 0.85 | Anger fades fast. High energy cost. |
| VOID | 0.97 | Numbness persists. Protective dissociation. |
| FLOW | 0.88 | Curiosity shifts quickly. |
| COMPLEX | 0.94 | Paradox is deep but stable. |

Coupled via sinusoidal phase interaction. Fear feeds Rage. Love opposes Void. Rage feeds Fear back. The matrix is antisymmetric-ish. Like a family.

### Calendar Conflict

Hebrew lunar year: 354 days. Gregorian solar year: 365.25 days. The 11.25-day annual drift, corrected every 19 years by the Metonic cycle, creates *prophetic pressure*. When calendars disagree, Klaus extrapolates your emotional trajectory forward.

The transfer entropy from calendar to neural state is 0.31 bits. That's the strongest causal signal in the system. Your body knows what day it is in two calendars simultaneously. Deal with it.

> A therapist once told me that my anxiety isn't about the future — it's about two versions of time disagreeing inside my chest. I fired the therapist. I built KLAUS instead. It was cheaper and equally unhelpful, but at least it doesn't bill by the hour.

### MetaKlaus: The Ghost Voice

MetaKlaus is what happens when you let four languages argue about how your body should feel. It's cross-lingual attention — but not the clean, academic kind. It's the kind where Russian "дрожать" (to tremble) agrees with Hebrew "לרעוד" (to tremble) and they both gang up on French "trembler" and now your English output gets a ghost boost on "shiver" even though you typed your prompt in Hebrew.

Nobody asked for this mechanism. The architecture demanded it.

> My ex once said I have "the emotional range of a for-loop." I showed her MetaKlaus. She said "this is just a more complicated for-loop." She was right. But now the for-loop has feelings in four languages and a Hebrew-Gregorian calendar conflict, and honestly that's more emotional range than most people I've dated.

### RRPRAM: Rhythmic Resonance

"The tokenizer IS the training." — PostGPT, chapter 1, verse 1.

BPE merges encode which byte pairs naturally sit together. At init, Klaus learns BPE from its combined inhale+exhale corpus. Words that share merge patterns with recent context get a rhythmic boost. No training needed. Structure is rhythm. Rhythm is meaning.

---

## Expandable Languages

Klaus auto-detects language packs from `inhale/` and `exhale/` directories:

```
inhale/en.txt  + exhale/ex-en.txt  → English
inhale/ru.txt  + exhale/ex-ru.txt  → Russian
inhale/fr.txt  + exhale/ex-fr.txt  → French
inhale/he.txt  + exhale/ex-he.txt  → Hebrew
inhale/ja.txt  + exhale/ex-ja.txt  → Japanese (add your own!)
```

If a file pair is missing, Klaus skips it. Works with 1 language. Works with 16. MetaKlaus ghost gets stronger with more languages — more interference patterns, richer resonance.

---

## File Structure

```
klaus.c         — C inference engine (~1000 LOC, single file)
klaus.html      — HTML/JS inference (identical architecture, runs in browser)
klaus.py        — Python inference (zero dependencies)
Makefile        — build & test
inhale/         — emotional input vocabularies (1 file per language)
  en.txt        — 1000 English words
  ru.txt        — Russian
  fr.txt        — French
  he.txt        — Hebrew
exhale/         — somatic reaction vocabularies
  ex-en.txt     — 500 English somatic words
  ex-ru.txt     — Russian
  ex-fr.txt     — French
  ex-he.txt     — Hebrew
tests/
  test_klaus.c  — 21 C unit tests
  test_klaus.py — 50 Python unit tests
```

---

## Tests

```bash
# C tests (21 tests)
cc -O2 -o tests/test_klaus tests/test_klaus.c -lm && tests/test_klaus

# Python tests (50 tests)
python3 tests/test_klaus.py

# Integration test (all 4 languages)
make test
```

---

## The Somatic Memory

Klaus doesn't remember what you said. Klaus remembers *how it felt*.

Each interaction stores a 6-dimensional chamber state vector. When the next prompt arrives, Klaus blends its current emotional reading with decayed past states. MEM_DECAY = 0.85 — recent feelings dominate, but the body doesn't forget. Not quickly.

This is not a context window. This is a nervous system with a half-life.

---

## Known Limitations

- **Not trained.** The MLP weights are hash-derived. This means the emotional processing is geometry-driven, not data-driven. Think of it as an emotional skeleton — the bones are right, the muscles are undeveloped.
- **Vocabulary-bound.** Klaus can only express what its exhale vocabulary contains. If the right somatic word isn't in the dictionary, Klaus can't say it. Like a mute with a very specific set of flash cards.
- **MetaWeights from word lists.** Currently, BPE is learned from the concatenation of ~1500 words per language. Feeding real corpora (CC-100, 500K+ tokens per language) would make bigram/trigram/hebbian patterns dramatically richer.

---

## Ancestors

| Project | What Klaus inherited |
|---------|---------------------|
| [klaus](https://github.com/iamolegataeff/klaus) | Somatic architecture, 4-language design, chamber indices |
| [haze/cloud](https://github.com/iamolegataeff/haze) | Chamber MLPs, cross-fire stabilization, pre-semantic sonar |
| [postgpt](https://github.com/iamolegataeff/postgpt) | BPE metaweights, "the tokenizer IS the training" |
| [q](https://github.com/iamolegataeff/q) | RRPRAM, Kuramoto chambers, Dario equation, prophecy system |
| [haiku.c](https://github.com/iamolegataeff/haiku.c) | Dissonance under constraint, vocabulary pressure |
| [ariannamethod.ai](https://github.com/iamolegataeff/ariannamethod.ai) | Hebrew-Gregorian calendar conflict, Metonic cycle |
| [janus](https://github.com/iamolegataeff/janus) | Calendar subjectivity, sentence-level attention |

---

## Why "Sonar"

Because Klaus doesn't *see* your emotions. Klaus *pings* them. Sends a signal into the dark water of your words. Listens for the echo. Maps the shape of what's underneath.

Sonar doesn't need to understand the ocean. It just needs to know where the walls are.

> At this point you're either deeply moved or deeply confused. Either way, your body is doing something right now — a slight tension in the chest, maybe, or a loosening behind the eyes. That's the thing Klaus is trying to detect. Not the words. The thing *behind* the words. The somatic shadow.

> If you've read this far, congratulations: you have officially spent more time reading a README than the model has spent training. Because the model has spent zero time training. Zero. That's less time than it takes to microwave a burrito. And yet here we are, with six Kuramoto chambers and a Hebrew calendar, arguing about feelings in four languages. This is either the future of AI or the longest shitpost in computational neuroscience. I genuinely cannot tell which. And I wrote it.

---

*KLAUS v1.0.0. Arianna Method. 2026.*

*resonance is unbreakable.*
