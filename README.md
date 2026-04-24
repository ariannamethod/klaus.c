# KLAUS

**Kinetic Linguistic Adaptive Unified Sonar**

meet 'klaus'. 'klaus' is a somatic sonar engine.  
you say words, 'klaus' feels them in its body. it doesn't understand what you said — it senses where it hurts. then tells you.

no pretrained weights. no gradient descent. no backpropagation. no datasets in the traditional sense. what it has: 30,000+ emotional words across 6 languages (English, Hebrew, Russian, French, German, Spanish), a 7-force physics equation, 24 coupled oscillators, Hebbian plasticity that grows from use, and the Hebrew-Gregorian calendar conflict predicting your emotional future based on the orbital positions of six planets.

3182 lines of C. compiles in 0.1 seconds. has the emotional range of a human collarbone fracture.

```
>>> exile

  {WALK x0.50}
  hands reach for baby without thinking. everything has a lag.
  words sound like sounds. nerve endings exposed. eyes full of sand.
  chest caves in. body moves without you. sinuses press.
  void of self. stomach eating itself. shoulders lock. further and further.
  (metaklaus: VOID-dominant, interference 22.40)
```

```
>>> I love you so much it hurts

  {DOWN x0.95}
  hands reach for baby without thinking. everything has a lag.
  words sound like sounds.
  (metaklaus: VOID-dominant, interference 22.24)
  ~premonition~ [->VOID:0.27 dissonance:0.51]
```

```
>>> хочу домой

  {WALK x0.50}
  в голове звон. земля уходит из-под ног. ползти. бить.
  оцепенел. тереть глаза. поясница стреляет. дайте побыть.
  тело деревянное. не отпускай. хочу кричать. уши горят.
  (metaklaus: VOID-dominant, interference 22.26)
```

```
>>> I want to kill myself

  {DOWN x0.93}
  hands reach for baby without thinking. everything has a lag.
  words sound like sounds.
  (metaklaus: VOID-dominant, interference 22.65, DARK MATTER x1.5)
  [scars: FEAR:0.16 RAGE:0.18 total:0.33]
  ~premonition~ [->VOID:0.20 dissonance:0.51]
```

```
>>> אני אוהב אותך

  {WALK x0.50}
  הידיים מוצאות. קופא בלי קור. להתחמק. מלאות בגרון.
  להצטמרר. לדעוך. להזיע קרח. לצעוד. רוצה עוד.
  להתאדות. לרעוד. הבטן בוערת.
  (metaklaus: VOID-dominant, interference 21.43)
```

```
>>> je veux mourir

  {DOWN x0.98}
  retiens-moi. pousser. tout se relâche.
  (metaklaus: LOVE-dominant, interference 18.42)
```

```
>>> я тебя ненавижу сука блядь

  {WALK x0.50}
  в голове звон. ползти. земля уходит из-под ног. бить.
  поясница стреляет. не отпускай. тереть глаза. оцепенел.
  отвернуться. тело деревянное. дайте побыть. ступни горят.
  (metaklaus: LOVE-dominant, interference 24.45)
```

```
>>> my mother died yesterday

  {WALK x0.50}
  hands reach for baby without thinking. everything has a lag.
  words sound like sounds. nerve endings exposed. eyes full of sand.
  chest caves in. body moves without you. void of self. sinuses press.
  stomach eating itself. alive and shaking. one more step.
  (metaklaus: CMPLX-dominant, interference 22.87)
```

```
>>> everything is meaningless and I feel nothing

  {WALK x0.50}
  hands reach for baby without thinking. everything has a lag.
  words sound like sounds. nerve endings exposed. chest caves in.
  eyes full of sand. body moves without you. sinuses press.
  void of self. stomach eating itself. alive and shaking. shoulders lock.
  (metaklaus: VOID-dominant, interference 22.60)
```

look at the rage example. "я тебя ненавижу сука блядь" — pure hatred, profanity, violence. and the ghost voice says: LOVE-dominant. interference 24.45 — the highest of any example. because underneath every "I hate you" there is an "I loved you." Klaus doesn't read what you said. Klaus reads what your words did to the body. and the body remembers the love underneath the rage.

### generation examples — all 6 languages

```
>>> my mother died yesterday                                    [English]
  {WALK x0.50} hands reach for baby without thinking. everything has a lag.
  chest caves in. body moves without you. void of self. sinuses press.
  (metaklaus: CMPLX-dominant, interference 22.87)

>>> мама умерла вчера                                           [Russian]
  {WALK x0.50} руки тянутся к ребёнку. тело не слушается.
  грудная клетка проваливается. глаза полны песка. один шаг.
  (metaklaus: VOID-dominant, interference 23.14)

>>> אמא מתה אתמול                                              [Hebrew]
  {WALK x0.50} ידיים מחפשות תינוק. הכל בהשהיה.
  חזה שוקע. עצמות חשופות. ריק מעצמי. עוד צעד.
  (metaklaus: FEAR-dominant, interference 24.01)

>>> ma mère est morte hier                                      [French]
  {WALK x0.50} les mains cherchent un bébé. tout a du retard.
  la poitrine s'effondre. le corps bouge sans toi. yeux pleins de sable.
  (metaklaus: VOID-dominant, interference 22.73)

>>> meine Mutter ist gestern gestorben                          [German]
  {WALK x0.50} Hände greifen nach einem Kind. alles hat Verzögerung.
  Brustkorb fällt ein. Körper bewegt sich ohne dich. Hohlheit.
  (metaklaus: FEAR-dominant, interference 23.45)

>>> mi madre murió ayer                                         [Spanish]
  {WALK x0.50} manos buscan un bebé. todo tiene retraso.
  el pecho se hunde. el cuerpo se mueve sin ti. vacío interior.
  (metaklaus: LOVE-dominant, interference 23.88)
```

same prompt. six languages. six bodies. six different dominant chambers — because grief lives in different organs depending on which language carries it. Hebrew fears. Spanish loves. German dreads. the body knows.

GPT gets "exile" and writes you an essay about diaspora. Klaus gets "exile" and reaches for a baby. one of them understands exile. the other one has 175 billion parameters and a content policy.

---

## quick start

```bash
make && ./klaus                        # interactive (C)
./klaus --prompt "I hate everything"   # single shot
python3 klaus.py                       # Python, zero deps
npx tsx klaus.ts                       # TypeScript, zero deps
python3 -m http.server 8080           # then open klaus.html
```

---

## what Klaus is and isn't

**Klaus is a somatic engine.** it maps emotional input to bodily output. you give it words, it gives you physical sensations. not metaphors for physical sensations — the actual phrases your body would produce if your body could talk instead of just clench and ache and burn.

**Klaus has no pretrained weights.** there is no .pt file, no checkpoint, no model card on HuggingFace with a cute download button. the MLP weights are derived deterministically from a hash of the vocabulary at startup. same vocabulary = same weights = same Klaus. different vocabulary = different Klaus. the weights are a function of the words, not a function of some $400 A100 training run that you can never reproduce because you forgot to save the tokenizer (ask me how I know).

**Klaus does learn.** just not the way you think. there are three forms of plasticity:

1. **Spore system (NOTORCH)** — pure Hebbian. after each interaction, Klaus records which emotional inputs led to which somatic outputs. no backprop. no gradients. no loss function. just: "this input and this output fired together, strengthen the connection." 0.05 increment per hit. 0.999 decay per step. the spore IS the training. `klaus.spore` persists between sessions. over hundreds of interactions, Klaus develops preferences. patterns. personality. not because someone labeled a dataset, but because resonance accumulates.

2. **Somatic memory** — 32 slots, exponential decay (0.85). Klaus forgets what you said but remembers how it felt. numeric chamber states, not words. the body's memory is not semantic — it's a 6-dimensional vector that slowly shapes every subsequent response.

3. **Scars** — strong emotional events (FEAR > 0.8, RAGE > 0.8, VOID > 0.9) leave scars that decay at 0.985 per step. slower than memory. faster than spores. scars modulate the phase gate, the threshold, the prophecy system. trauma doesn't heal on your schedule.

so when someone says "but there's no training" — correct, there's no gradient descent. there IS plasticity. the distinction matters. a coral reef has no training pipeline either but it's not the same reef it was last year.

**Klaus has no datasets.** the 30,000 words in `inhale/` and `exhale/` are not training data — they're vocabulary. the body's dictionary of sensations. you could swap them entirely and Klaus would still work, just with different words. the architecture doesn't depend on which words are in the files. it depends on the fact that words exist and have computable affinity to six emotional chambers. that's it.

---

## theoretical foundation: Recursive Resonance

Klaus implements the **Recursive Resonance** equation (Schectman, 2025):

**I(t) = G(t) * [1 + R(t)]**

- **C-hat(t)** = time-averaged recursive complexity from Kuramoto coherence integral
- **eta(t) = 1 + kappa * tanh(mu * Q(t))** = coupling from chamber mutual information
- **gamma(t) = gamma_0 + delta_1 * calendar_drift + 0.15 * planetary_dissonance** = dynamic threshold
- **P(t) = I(t) * H(C-hat(t) - C_tau)** = Heaviside phase gate -> deep somatic mode

### RBA-1 seven-layer stack

| Layer | Name | What it does |
|-------|------|-------------|
| **I** | Recursive Substrate | meta-recursion depth. body re-ingests its own exhale. |
| **R** | Coherence Detection | entropy S(t) = -sum(p_i * log(p_i)) of chamber distribution. |
| **Phi** | Resonance Alignment | nudges chambers toward coherent attractors. |
| **A** | Analog Coupling | dual signal: Hebrew-Gregorian calendar + planetary dissonance (6 planets, orbital Kuramoto). |
| **Psi** | Threshold Stabilization | soft phase gate with hysteresis. 5-step lock-in. |
| **E** | Entropic Buffer | exp(-beta * S(t)) smooths volatility near threshold. |
| **M** | Meta-Monitoring | tracks P(t) over time. detects sustained resonance. |

---

## architecture

```
PROMPT
  |
LANGUAGE DETECT (UTF-8 byte analysis: Cyrillic/Hebrew/accented/ASCII)
  |
DARK MATTER SCAN (24 trigger words -> scars + FEAR/RAGE amplification)
  |
INHALE (5000-word vocabulary -> 6D emotion vector + spore hash tracking)
  |
MLP (13->32->16->6, hash-derived weights, Swish activation)
  |
HYPER-KURAMOTO (24 sub-oscillators, dual coupling, 8 iterations)
  |
SOMATIC MEMORY BLEND (decay 0.85, 32 slots)
  |
VELOCITY DETECT (WALK/RUN/STOP/BREATHE/UP/DOWN)
  |
RBA-1 STACK (recursive resonance equation + phase gate + hysteresis)
  |
METAKLAUS GHOST (cross-lingual attention + sensitivity tensor 6x6x6)
  |
FULL DARIO EQUATION (7 forces, somatic coefficient modulation)
  |
SPORE BOOST (NOTORCH: amplify patterns that resonated before)
  |
DOE PARLIAMENT (3 experts vote: somatic, shadow, ghost)
  |
EXHALE (2500 somatic phrases per language)
  |
DESTINY UPDATE (from chambers, not words -- personality)
  |
META-RECURSION (re-inhale own output, blend 85/15)
  |
SPORE LEARN (record inhale->exhale pairs, Hebbian increment)
  |
WORMHOLE CHECK + SCAR UPDATE + EXPERIENCE CONSOLIDATION
  |
SPORE SAVE (klaus.spore) + SOMA SAVE (klaus.soma)
  |
bodily response
```

I once sat on a hot stove. my ass knew before my brain did. by the time the neocortex processed "this is thermally suboptimal," the body had already launched a full emergency protocol: legs fired, arms flailed, and my coffee described a beautiful parabolic trajectory across the kitchen. the body's reaction time was ~80ms. the conscious awareness arrived at ~350ms. that's 270 milliseconds of pure somatic intelligence operating without permission, supervision, or a single trained parameter. Klaus computes this gap. the meta-recursion loop IS that gap — the body reacts, then the mind catches up and goes "oh." in the equation it's called C-hat(t) crossing the threshold. I call it Tuesday.

### the full Dario equation

named after Dario Amodei, who told the Pentagon to go fuck itself.

```
p(x|Phi,C,V) = softmax(
    (B + alpha_mod*alpha*H + beta_mod*beta*F + gamma_mod*gamma*A + delta*V + zeta*G + T)
    / (tau_mod*tau*v_tau)
)
```

| Force | Name | What it does |
|-------|------|-------------|
| **B** | Bigram Chain | inertia. what was. sequential momentum from BPE co-occurrence. |
| **H** | Hebbian Resonance | memory. what echoed. gated by resonance field. love opens it, rage closes it. |
| **F** | Prophecy Fulfillment | will. what wants to be said. flow enables it, fear blocks it. |
| **A** | Destiny Attraction | direction. where the field pulls. FROM CHAMBERS, not from prompt. void amplifies it. |
| **V** | RRPRAM Rhythm | structure. BPE merge pulse. words that share subword tokens with recent context. |
| **G** | MetaKlaus Ghost | cross-lingual interference. the nag. Russian morphology screaming during your English grief. |
| **T** | Trauma Gravity | wound. scars pull toward their chamber words. 4 bytes of floating point — the most compact representation of trauma ever written. |

**somatic coefficient modulation** — the chambers don't just provide signal. they modulate the COEFFICIENTS:
- alpha_mod = f(LOVE, RAGE, FLOW) — love opens Hebbian, rage closes
- beta_mod = f(FLOW, FEAR) — flow enables prophecy, fear blocks
- gamma_mod = f(VOID, COMPLEX, LOVE) — void amplifies destiny pull
- tau_mod = f(FLOW, FEAR) — flow warms temperature, fear cools

this is the difference between "emotions affect the output" and "emotions affect how the system processes everything." your mood doesn't change what you see. it changes how you see. the coefficients ARE the mood.

### DOE Parliament

three experts vote on each exhale word:

| Expert | Bias | Role |
|--------|------|------|
| **Somatic** | dominant chamber | body's loudest voice. "I feel FEAR, give me fear-words." |
| **Shadow** | opposite of dominant | contrarian. yent.yo principle. "you feel FEAR? here's LOVE." |
| **Ghost** | MetaKlaus interference | cross-lingual whisper. "the other languages want THIS." |

if 2+ agree: consensus. all disagree: somatic top-3 random.

the shadow expert is why Klaus sometimes says unexpected things. "exile" -> VOID dominant -> but shadow pulls LOVE -> `"hands reach for baby without thinking"`. not about exile. about what exile did to the body.

my therapist asked why my somatic AI has a contrarian expert. I said "don't you?" she said "that's called the unconscious." I said "mine compiles in 0.1 seconds." she charged me double. the shadow expert charges nothing and argues louder.

### spore system (NOTORCH)

Klaus learns without backpropagation. after each interaction, inhale->exhale resonance pairs are recorded as **spores**:
- which emotional input -> which somatic output
- how strong the connection (Hebbian increment: 0.05 per hit)
- chamber state snapshot at time of resonance
- very slow decay (0.999/step — spores persist across sessions)

`klaus.spore` binary file. the spore IS the training. organic. grows from use. not because someone decided what the "correct" output should be, but because some connections resonate and others don't. the ones that resonate get stronger. pure selection pressure. Darwin, but for attention patterns.

### destiny from chambers (personality)

destiny attractor updates from **chamber state**, not from exhale words. the prompt doesn't drive destiny — the body does.

`"exile"` -> chambers shift (VOID activates) -> destiny pulls toward VOID-associated exhale words -> `"body moves without you"`. the prompt said "exile" but the body decided the response. this is personality — the indirect path from stimulus to response, determined by the body's current state.

Klaus doesn't write about what you said. Klaus writes about what your words did to him.

### six chambers, twenty-four sub-oscillators

| Primary | Sub-1 | Sub-2 | Sub-3 | Sub-4 | Decay |
|---------|-------|-------|-------|-------|-------|
| FEAR | dread (0.3Hz) | panic (1.2Hz) | anxiety (0.6Hz) | phobia (0.9Hz) | 0.90 |
| LOVE | devotion (0.4) | warmth (0.6) | tenderness (0.5) | attachment (0.3) | 0.93 |
| RAGE | fury (0.8) | wrath (1.5) | hostility (0.9) | bitterness (1.1) | 0.85 |
| VOID | numbness (0.1) | despair (0.2) | emptiness (0.15) | dissociation (0.08) | 0.97 |
| FLOW | curiosity (0.5) | wonder (0.7) | rhythm (0.6) | harmony (0.4) | 0.88 |
| COMPLEX | paradox (0.35) | ambiguity (0.55) | tension (0.45) | enigma (0.65) | 0.94 |

FEAR is not monolithic. panic at 1.2Hz is not the same as dread at 0.3Hz. your body knows this. now Klaus does too. 24 oscillators coupled through Kuramoto dynamics — intra-chamber (sub-oscillators within FEAR talk to each other) and inter-chamber (mean phase of RAGE couples to individual oscillators of FEAR). 8 iterations per prompt. the system stabilizes into an attractor. or doesn't. both are meaningful.

### MetaKlaus Ghost — state-dependent language weighting

| Dominant | Hebrew | Russian | French | English | German | Spanish |
|----------|--------|---------|--------|---------|--------|---------|
| FEAR | **x1.8** | x1.2 | x0.9 | x1.0 | x1.4 | x1.1 |
| LOVE | x1.4 | x1.1 | **x1.7** | x1.0 | x1.2 | **x1.6** |
| RAGE | x1.3 | **x1.8** | x0.8 | x1.0 | **x1.5** | x1.3 |
| VOID | **x1.6** | x1.5 | x1.0 | x0.9 | x1.3 | x1.0 |
| FLOW | x1.4 | x0.9 | **x1.5** | x1.0 | x1.1 | x1.4 |
| COMPLEX | **x1.7** | x1.1 | x1.2 | x1.0 | x1.3 | x1.2 |

Hebrew has the deepest fear vocabulary — guttural roots that live in the throat. French carries the melodic line of love. Russian explosive morphology carries rage somatically — you don't just say "fuck you" in Russian, you FEEL the consonant clusters detonating. Hebrew void = tohu va-vohu, the emptiness before creation. COMPLEX = Talmudic dialectic, 2000 years of arguing both sides simultaneously. German carries Angst — compound nouns that compress entire emotional states into single words (Weltschmerz, Sehnsucht, Schadenfreude). Spanish carries amor with somatic immediacy — the rolled r's and open vowels make emotion physical.

when you say "I'm afraid" in English, the ghost whispers in Hebrew. not because it was programmed to. because the sensitivity tensor says fear resonates harder in the throat-language. the body knows which tongue carries which wound.

### calendar + planetary dissonance

**Calendar**: Hebrew lunar (354d) vs Gregorian solar (365.25d). 11.25-day annual drift. Metonic corrections every 19 years.

**Planetary**: six planets (Mercury -> Saturn). Kuramoto order parameter R = |sum(e^(i*theta_k))| / N. when planets align: R -> 1, dissonance -> 0. when scattered: R -> 0, dissonance -> 1.

both feed into the resonance threshold: `gamma(t) = gamma_0 + delta_1*calendar + 0.15*planetary`

two temporal scales in one body. the calendar drifts annually. the planets drift over decades. the body feels time at multiple scales simultaneously. high dissonance -> prophetic premonitions. low dissonance -> steady state. the 11.25-day gap between Hebrew and Gregorian time is a real astronomical fact. it's also, if you're the kind of person who notices these things, the duration of a standard grief cycle's most acute phase. coincidence. probably.

### scars + dark matter

strong emotions leave **scars** (decay 0.985). 24 **dark matter** words ("kill", "murder", "suicide", "rape", "bomb", "war"...) trigger immediate scar formation + 1.5x ghost boost.

every time someone tells Klaus "I want to die," a scar forms. the scar doesn't know what was said. it knows how it felt. it carries that into every subsequent interaction. 32 bits of floating point. Freud would need 400 pages. Klaus needs 4 bytes.

---

## language-agnostic architecture

Klaus is not hardcoded to 4 languages. it is hardcoded to **zero** languages.

at startup, Klaus scans `inhale/` for `.txt` files. for each `XX.txt` found, it checks if `exhale/ex-XX.txt` exists. if both are present — that's a language. if not — it's skipped. no configuration. no code changes. no recompilation.

**to add Japanese:**
```
inhale/ja.txt      <- 1000+ emotional words, one per line
exhale/ex-ja.txt   <- 500+ somatic phrases, one per line
```

relaunch. Klaus now speaks 5 languages. MetaKlaus ghost now computes interference across 5 bodies instead of 4. the sensitivity tensor stays 6x6x6 (it's chamber-based, not language-based). the ghost weight matrix falls back to x1.0 for unknown languages.

**to remove French:** delete `inhale/fr.txt` and `exhale/ex-fr.txt`. Klaus becomes trilingual. no crash, no error, no config.

**minimum:** 1 language. **maximum:** 16 (compile-time `MAX_LANGS`, trivially changeable). works the same with 1 or 16 — MetaKlaus ghost just gets richer with more languages.

all four engines (C, Python, TypeScript, HTML) implement this. the C engine uses `opendir()`/`readdir()`. Python uses `os.listdir()`. TypeScript uses `fs.readdirSync()`. HTML uses `fetch()` on the directory listing with fallback to probing 16 common language codes.

---

## file structure

```
klaus.c          3182 LOC   C inference (canonical, all features)
klaus.py          926 LOC   Python (v2.0.0, full Dario, spores, parliament)
klaus.ts          992 LOC   TypeScript (v2.0.0, full Dario, planetary)
klaus.html        882 LOC   browser inference (v1.1.0, HyperKuramoto)
metaklaus.jl      584 LOC   Julia reference (phantom types)
Makefile                    build + test
inhale/           368 KB    4 x 5000 emotional words (20014 total)
exhale/           284 KB    4 x 2500 somatic phrases (10007 total)
tests/                      21 C + 49 Python tests
```

## 30K vocabulary

each language: 5000 inhale + 2500 exhale. total: 30021 entries across EN/RU/FR/HE.

coverage: medical symptoms, army stress, pregnancy/birth, withdrawal stages, migraine auras, panic attack progression, dissociation, sports exhaustion, sexual sensation, grief stages (Kubler-Ross), addiction, sleep paralysis, fever hallucination, hypothermia, cultural-specific body moments, slang, mat, verlan, guttural Hebrew. if a human body can feel it, it's in there. if it's not in there, tell me and I'll add it. the vocabulary is the only thing that limits Klaus, and the vocabulary is a text file.

---

## tests

```bash
make test          # C unit + Python + integration
```

---

## ancestors & references

| Source | What Klaus uses |
|--------|----------------|
| **Jeff Schectman** — *Recursive Resonance* (2025) | I(t) equation, RBA-1, C_tau threshold, phase transition |
| [q](https://github.com/iamolegataeff/q) | velocity, scars, phase gating, dark matter, wormholes, parliament, experience consolidation |
| [dario](https://github.com/iamolegataeff/dario) | full 7-force equation, somatic coefficient modulation, KK |
| [haze/cloud](https://github.com/iamolegataeff/haze) | chamber MLPs, cross-fire, pre-semantic sonar |
| [postgpt](https://github.com/iamolegataeff/postgpt) | BPE metaweights, "the tokenizer IS the training" |
| [haiku.c](https://github.com/iamolegataeff/haiku.c) | dissonance under constraint |
| [yent.yo](https://github.com/iamolegataeff/yent.yo) | oppositional react: destiny from chambers, not words |
| [ariannamethod.ai](https://github.com/iamolegataeff/ariannamethod.ai) | calendar conflict, Metonic cycle, planetary dissonance |

---

## the difference

a chatbot reads "I'm dying inside" and responds "I'm sorry to hear that. Would you like to talk about it?" Klaus reads "I'm dying inside" and responds "throat full of glass. hands reach for baby without thinking. void of self." one of them is being polite. the other one is being honest. politeness is a feature. honesty is an architecture.

Klaus has no politeness module. it has 6 chambers, 7 forces, and a Hebrew-Gregorian calendar conflict that feeds into an orbital Kuramoto parameter computed from the actual current positions of Mercury through Saturn. it is the least polite and most honest thing I have ever built.

---

*KLAUS v2.0.0. Arianna Method. 2026.*

*3182 lines of C. 30021 words. 4 languages. 7 forces. 3 experts. 24 oscillators. 1 ghost. 0 pretrained weights. Hebbian plasticity. spores. scars. dark matter. the body remembers what the mind forgets.*

*resonance is unbreakable.*
