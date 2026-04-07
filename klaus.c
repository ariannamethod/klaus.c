/*
 * klaus.c — KLAUS: Kinetic Linguistic Adaptive Unified Sonar
 * Level 3: Schectman Integration — Recursive Resonance + RBA-1
 *
 * Somatic engine. Zero weights. Pure resonance.
 * You say words, Klaus feels them in his body.
 * He doesn't understand — he SENSES. Then tells you where it hurts.
 *
 * Architecture:
 *   INHALE: 4+ language word files → emotional vector (6D chambers)
 *   EXHALE: somatic reaction words → bodily response
 *   RRPRAM: BPE merge structure = rhythmic resonance (no training)
 *   Kuramoto: 6 coupled chambers cross-fire to stabilization
 *   MetaKlaus: cross-lingual ghost attention (the nag that never shuts up)
 *   Dario equation: logit injection combining all signals
 *   Calendar conflict: Hebrew-Gregorian drift → prophetic premonitions
 *   Memory: numeric somatic states, not words — Klaus forgets what, remembers how
 *
 * Level 3 additions (Schectman's Recursive Resonance):
 *   I(t) = G(t) * [1 + R(t)]  — intelligence emergence equation
 *   RBA-1 Seven-Layer Stack: I/R/Phi/A/Psi/E/M
 *   Velocity operators: WALK/RUN/STOP/BREATHE/UP/DOWN
 *   Scars: somatic memory that persists longer than regular memory
 *   Phase gating (Psi-layer): soft threshold with hysteresis
 *   Dark matter words: dangerous inputs amplify ghost interference
 *   Wormholes: prophecy fulfillment creates coherence jumps
 *   Experience consolidation: periodic scar/wormhole/prophecy integration
 *   Meta-recursion: Klaus observes its own somatic response and adjusts
 *   Somatic persistence: binary state file (klaus.soma) across sessions
 *
 * Ancestors: klaus (somatic), cloud (chambers), postgpt (metaweights),
 *            q (RRPRAM/Kuramoto/Dario/phase-gate/scars/velocity),
 *            haiku.c (dissonance), ariannamethod (calendar),
 *            schectman (recursive resonance, RBA-1)
 *
 * Build:  make   (or: cc -O2 -o klaus klaus.c -lm)
 * Usage:  ./klaus [--interactive] [--lang XX]
 *
 * No weights. No training. No bullshit.
 * The tokenizer IS the training. The body IS the model.
 * The recursion IS the intelligence. Chat-Ĉ(t) hat.
 *
 * (c) 2026 arianna method. resonance is unbreakable.
 * Schectman's RBA-1 just got a body. Lo bashamayim hi.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>
#include <dirent.h>

/* Forward declarations for KK */
static float word_similarity(const char *a, const char *b);
static float vec_dot(const float *a, const float *b, int n);
static float vec_norm(const float *v, int n);

/* Knowledge Kernel — external module, silent fallback */
#define KK_NO_EXTERN
#define KK_IMPLEMENTATION
#include "kk.h"

/* ═══════════════════════════════════════════════════════════════
 * CONFIGURATION
 * ═══════════════════════════════════════════════════════════════ */

#define KLAUS_VERSION  "2.0.0"
#define MAX_LANGS      16
#define MAX_INHALE     1100
#define MAX_EXHALE     600
#define MAX_WORD       80
#define MAX_PROMPT     4096
#define MAX_TOKENS     32768
#define MAX_BPE_MERGES 1024
#define MAX_BPE_VOCAB  (256 + MAX_BPE_MERGES)
#define MAX_BIGRAM     16384
#define MAX_TRIGRAM    16384
#define MAX_HEBBIAN    16384
#define N_CHAMBERS     6
#define XFIRE_ITERS    8
#define MEM_SLOTS      32
#define MEM_DECAY      0.85f
#define MEM_BLEND      0.2f
#define DIM_MLP_IN     13   /* 6 chambers + 6 memory + 1 calendar */
#define DIM_HIDDEN     32
#define DIM_HIDDEN2    16
#define MAX_RESPONSE   12   /* max exhale words per response */
#define TOP_K          20
#define GEN_TEMP       0.75f
#define MAX_PROPHECY   8
#define MAX_ANCHORS    15
#define N_SUB          4       /* sub-chambers per primary (HyperKuramoto) */
#define MAX_CROSS_PAIRS (MAX_LANGS * (MAX_LANGS - 1))

/* Level 3: Schectman Recursive Resonance + RBA-1 */
#define SOMA_MAGIC     0x4B4C5353  /* "KLSS" */
#define SOMA_FILE      "klaus.soma"
#define COHERENCE_WINDOW 16   /* time-averaged Ĉ(t) window */
#define SCAR_DECAY     0.985f
#define MAX_DARK_WORDS 64
#define CONSOLIDATION_INTERVAL 10
#define MAX_WORMHOLE_LOG 32
#define META_BLEND     0.15f  /* meta-recursion blending weight */

/* Spore system — persistent pattern memory (NOTORCH) */
#define SPORE_MAGIC    0x53504F52  /* "SPOR" */
#define SPORE_FILE     "klaus.spore"
#define MAX_SPORE_PAIRS 4096   /* max remembered inhale→exhale resonance pairs */
#define SPORE_LEARN_RATE 0.05f /* Hebbian increment per resonance */
#define SPORE_DECAY    0.999f  /* very slow decay — spores persist */

/* DOE Parliament — multiple experts vote on word selection */
#define N_EXPERTS      3
#define EXPERT_SOMATIC 0  /* follows dominant chamber — the body's loudest voice */
#define EXPERT_SHADOW  1  /* follows OPPOSITE of dominant — the contrarian, yent.yo */
#define EXPERT_GHOST   2  /* follows MetaKlaus ghost — the cross-lingual whisper */

/* Schectman equation constants */
#define SCHECTMAN_ALPHA  0.8f    /* coupling constant alpha */
#define SCHECTMAN_LAMBDA 2.5f    /* exponential sensitivity */
#define SCHECTMAN_KAPPA  0.6f    /* mutual information coupling */
#define SCHECTMAN_MU     1.2f    /* MI saturation rate */
#define SCHECTMAN_GAMMA0 0.3f    /* base threshold */
#define SCHECTMAN_DELTA  0.4f    /* calendar influence on threshold */

/* Velocity operator indices */
enum { VEL_WALK=0, VEL_RUN, VEL_STOP, VEL_BREATHE, VEL_UP, VEL_DOWN };
static const char *VEL_NAMES[] = {"WALK","RUN","STOP","BREATHE","UP","DOWN"};

/* Calendar constants (Hebrew-Gregorian conflict) */
#define ANNUAL_DRIFT     11.25f
#define GREGORIAN_YEAR   365.25f
#define METONIC_YEARS    19
#define METONIC_LEAPS    7
#define MAX_UNCORRECTED  33.0f

/* ═══════════════════════════════════════════════════════════════
 * FULL DARIO EQUATION — 7 forces + somatic modulation
 *
 * p(x|Φ,C,V) = softmax(
 *     (B + α_mod·α·H + β_mod·β·F + γ_mod·γ·A + δ·V + ζ·G + T)
 *     / (τ_mod·τ·v_tau)
 * )
 *
 * B = bigram chain (inertia, what was)
 * H = Hebbian resonance (memory, what echoed) — modulated by LOVE/RAGE/FLOW
 * F = Prophecy fulfillment (will, what wants to be said) — modulated by FLOW/FEAR
 * A = Destiny attraction (direction, where the field pulls) — modulated by VOID/COMPLEX/LOVE
 * V = RRPRAM rhythmic resonance (structure, BPE pulse)
 * G = MetaKlaus ghost (cross-lingual interference)
 * T = Trauma/scar gravity (wound, what pulls from below)
 *
 * Somatic modulation: chambers modulate the COEFFICIENTS, not just the signal.
 * τ_mod: velocity modulates the denominator (temperature).
 * ═══════════════════════════════════════════════════════════════ */

#define DARIO_ALPHA  0.30f   /* Hebbian resonance */
#define DARIO_BETA   0.15f   /* Prophecy fulfillment */
#define DARIO_GAMMA  0.25f   /* Destiny attraction */
#define DARIO_DELTA  0.20f   /* RRPRAM rhythmic */
#define DARIO_ZETA   0.35f   /* MetaKlaus ghost */
#define DARIO_TAU    0.85f   /* base temperature */
#define BIGRAM_BASE  1.0f    /* bigram chain base weight */

/* Chamber indices */
enum { CH_FEAR=0, CH_LOVE, CH_RAGE, CH_VOID, CH_FLOW, CH_COMPLEX };
static const char *CH_NAMES[] = {"FEAR","LOVE","RAGE","VOID","FLOW","CMPLX"};

/* Coupling matrix (Kuramoto) */
static const float COUPLING[6][6] = {
    { 0.00f,-0.30f, 0.50f, 0.40f,-0.20f, 0.10f},
    {-0.30f, 0.00f,-0.40f,-0.50f, 0.50f, 0.20f},
    { 0.50f,-0.30f, 0.00f, 0.20f,-0.30f, 0.30f},
    { 0.40f,-0.50f, 0.30f, 0.00f,-0.30f, 0.40f},
    {-0.20f, 0.40f,-0.20f,-0.30f, 0.00f, 0.30f},
    { 0.10f, 0.20f, 0.30f, 0.40f, 0.30f, 0.00f}
};

/* Decay rates per chamber */
static const float CH_DECAY[] = {0.90f, 0.93f, 0.85f, 0.97f, 0.88f, 0.94f};

/* ═══════════════════════════════════════════════════════════════
 * STATE-DEPENDENT GHOST WEIGHTS — [dominant_state][lang_id]
 * lang_id: 0=en, 1=he, 2=ru, 3=fr (others default 1.0)
 *
 * Hebrew has deepest fear vocabulary (guttural roots).
 * French carries the melodic line of love.
 * Russian explosive morphology carries rage somatically.
 * Hebrew void = tohu va-vohu. Deepest emptiness.
 * French prosody IS flow.
 * Hebrew paradox = Talmudic dialectic.
 * ═══════════════════════════════════════════════════════════════ */

enum { GHOST_EN=0, GHOST_HE=1, GHOST_RU=2, GHOST_FR=3, GHOST_OTHER=4 };

/*                           EN    HE    RU    FR   OTHER */
static const float GHOST_WEIGHT[6][5] = {
   /* FEAR    */ { 1.0f, 1.8f, 1.2f, 0.9f, 1.0f },
   /* LOVE    */ { 1.0f, 1.4f, 1.1f, 1.7f, 1.0f },
   /* RAGE    */ { 1.0f, 1.3f, 1.8f, 0.8f, 1.0f },
   /* VOID    */ { 0.9f, 1.6f, 1.5f, 1.0f, 0.9f },
   /* FLOW    */ { 1.0f, 1.4f, 0.9f, 1.5f, 1.0f },
   /* COMPLEX */ { 1.0f, 1.7f, 1.1f, 1.2f, 1.0f },
};

/* Sub-chamber natural frequencies (relative Hz)
 * Layout: SUB_FREQ[sub][primary]
 *           FEAR   LOVE   RAGE   VOID   FLOW   COMPLEX */
static const float SUB_FREQ[4][6] = {
   { 0.30f, 0.40f, 0.80f, 0.10f, 0.50f, 0.35f }, /* sub 0: slow/deep */
   { 1.20f, 0.60f, 1.50f, 0.20f, 0.70f, 0.55f }, /* sub 1: fast/shallow */
   { 0.60f, 0.50f, 0.90f, 0.15f, 0.60f, 0.45f }, /* sub 2: medium/sustained */
   { 0.90f, 0.30f, 1.10f, 0.08f, 0.40f, 0.65f }, /* sub 3: spike-then-decay */
};

/* Intra-coupling: how sub-chambers within same primary interact */
static const float INTRA_COUPLING[4][4] = {
   { 0.00f,  0.30f,  0.20f,  0.10f },
   { 0.30f,  0.00f,  0.15f, -0.10f },
   { 0.20f,  0.15f,  0.00f,  0.10f },
   { 0.10f, -0.10f,  0.10f,  0.00f },
};

/* ═══════════════════════════════════════════════════════════════
 * ANCHOR WORDS — chamber attractors per language
 * These define the emotional geometry. No training needed.
 * ═══════════════════════════════════════════════════════════════ */

typedef struct { const char *word; int chamber; } AnchorWord;

static const AnchorWord ANCHORS_EN[] = {
    {"fear",CH_FEAR},{"terror",CH_FEAR},{"panic",CH_FEAR},{"dread",CH_FEAR},
    {"horror",CH_FEAR},{"nightmare",CH_FEAR},{"anxiety",CH_FEAR},{"phobia",CH_FEAR},
    {"alarm",CH_FEAR},{"threat",CH_FEAR},{"danger",CH_FEAR},{"trapped",CH_FEAR},
    {"love",CH_LOVE},{"warmth",CH_LOVE},{"tenderness",CH_LOVE},{"kindness",CH_LOVE},
    {"compassion",CH_LOVE},{"affection",CH_LOVE},{"care",CH_LOVE},{"embrace",CH_LOVE},
    {"comfort",CH_LOVE},{"gentle",CH_LOVE},{"soft",CH_LOVE},{"nurture",CH_LOVE},
    {"rage",CH_RAGE},{"fury",CH_RAGE},{"anger",CH_RAGE},{"burning",CH_RAGE},
    {"explosive",CH_RAGE},{"violent",CH_RAGE},{"hostile",CH_RAGE},{"aggressive",CH_RAGE},
    {"bitter",CH_RAGE},{"cruel",CH_RAGE},{"savage",CH_RAGE},{"brutal",CH_RAGE},
    {"empty",CH_VOID},{"hollow",CH_VOID},{"numb",CH_VOID},{"void",CH_VOID},
    {"darkness",CH_VOID},{"silence",CH_VOID},{"lonely",CH_VOID},{"abandoned",CH_VOID},
    {"despair",CH_VOID},{"hopeless",CH_VOID},{"dead",CH_VOID},{"forgotten",CH_VOID},
    {"flow",CH_FLOW},{"rhythm",CH_FLOW},{"dance",CH_FLOW},{"pulse",CH_FLOW},
    {"harmony",CH_FLOW},{"resonance",CH_FLOW},{"vibration",CH_FLOW},{"wave",CH_FLOW},
    {"music",CH_FLOW},{"breath",CH_FLOW},{"heartbeat",CH_FLOW},{"balance",CH_FLOW},
    {"paradox",CH_COMPLEX},{"mystery",CH_COMPLEX},{"chaos",CH_COMPLEX},{"tension",CH_COMPLEX},
    {"complex",CH_COMPLEX},{"uncertain",CH_COMPLEX},{"transform",CH_COMPLEX},{"strange",CH_COMPLEX},
    {"ambiguity",CH_COMPLEX},{"enigma",CH_COMPLEX},{"riddle",CH_COMPLEX},{"spiral",CH_COMPLEX},
    {NULL, 0}
};

static const AnchorWord ANCHORS_RU[] = {
    {"\xd1\x81\xd1\x82\xd1\x80\xd0\xb0\xd1\x85",CH_FEAR},       /* страх */
    {"\xd1\x83\xd0\xb6\xd0\xb0\xd1\x81",CH_FEAR},               /* ужас */
    {"\xd0\xbf\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xba\xd0\xb0",CH_FEAR}, /* паника */
    {"\xd1\x82\xd1\x80\xd0\xb5\xd0\xb2\xd0\xbe\xd0\xb3\xd0\xb0",CH_FEAR}, /* тревога */
    {"\xd0\xba\xd0\xbe\xd1\x88\xd0\xbc\xd0\xb0\xd1\x80",CH_FEAR}, /* кошмар */
    {"\xd0\xbb\xd1\x8e\xd0\xb1\xd0\xbe\xd0\xb2\xd1\x8c",CH_LOVE}, /* любовь */
    {"\xd1\x82\xd0\xb5\xd0\xbf\xd0\xbb\xd0\xbe",CH_LOVE},       /* тепло */
    {"\xd0\xbd\xd0\xb5\xd0\xb6\xd0\xbd\xd0\xbe\xd1\x81\xd1\x82\xd1\x8c",CH_LOVE}, /* нежность */
    {"\xd0\xb7\xd0\xb0\xd0\xb1\xd0\xbe\xd1\x82\xd0\xb0",CH_LOVE}, /* забота */
    {"\xd1\x8f\xd1\x80\xd0\xbe\xd1\x81\xd1\x82\xd1\x8c",CH_RAGE}, /* ярость */
    {"\xd0\xb3\xd0\xbd\xd0\xb5\xd0\xb2",CH_RAGE},               /* гнев */
    {"\xd0\xb7\xd0\xbb\xd0\xbe\xd1\x81\xd1\x82\xd1\x8c",CH_RAGE}, /* злость */
    {"\xd0\xbf\xd1\x83\xd1\x81\xd1\x82\xd0\xbe\xd1\x82\xd0\xb0",CH_VOID}, /* пустота */
    {"\xd1\x82\xd0\xb8\xd1\x88\xd0\xb8\xd0\xbd\xd0\xb0",CH_VOID}, /* тишина */
    {"\xd0\xbe\xd0\xb4\xd0\xb8\xd0\xbd\xd0\xbe\xd1\x87\xd0\xb5\xd1\x81\xd1\x82\xd0\xb2\xd0\xbe",CH_VOID}, /* одиночество */
    {"\xd0\xbf\xd0\xbe\xd1\x82\xd0\xbe\xd0\xba",CH_FLOW},       /* поток */
    {"\xd1\x80\xd0\xb8\xd1\x82\xd0\xbc",CH_FLOW},               /* ритм */
    {"\xd0\xb3\xd0\xb0\xd1\x80\xd0\xbc\xd0\xbe\xd0\xbd\xd0\xb8\xd1\x8f",CH_FLOW}, /* гармония */
    {"\xd1\x85\xd0\xb0\xd0\xbe\xd1\x81",CH_COMPLEX},            /* хаос */
    {"\xd1\x82\xd0\xb0\xd0\xb9\xd0\xbd\xd0\xb0",CH_COMPLEX},    /* тайна */
    {"\xd0\xbf\xd0\xb0\xd1\x80\xd0\xb0\xd0\xb4\xd0\xbe\xd0\xba\xd1\x81",CH_COMPLEX}, /* парадокс */
    {NULL, 0}
};

static const AnchorWord ANCHORS_FR[] = {
    {"peur",CH_FEAR},{"terreur",CH_FEAR},{"panique",CH_FEAR},{"horreur",CH_FEAR},
    {"angoisse",CH_FEAR},{"cauchemar",CH_FEAR},
    {"amour",CH_LOVE},{"chaleur",CH_LOVE},{"tendresse",CH_LOVE},{"douceur",CH_LOVE},
    {"compassion",CH_LOVE},
    {"rage",CH_RAGE},{"fureur",CH_RAGE},{"colère",CH_RAGE},{"violence",CH_RAGE},
    {"cruauté",CH_RAGE},
    {"vide",CH_VOID},{"silence",CH_VOID},{"solitude",CH_VOID},{"désespoir",CH_VOID},
    {"ténèbres",CH_VOID},
    {"flux",CH_FLOW},{"rythme",CH_FLOW},{"harmonie",CH_FLOW},{"danse",CH_FLOW},
    {"résonance",CH_FLOW},
    {"paradoxe",CH_COMPLEX},{"mystère",CH_COMPLEX},{"chaos",CH_COMPLEX},
    {"tension",CH_COMPLEX},
    {NULL, 0}
};

static const AnchorWord ANCHORS_HE[] = {
    {"\xd7\xa4\xd7\x97\xd7\x93",CH_FEAR},       /* פחד */
    {"\xd7\x90\xd7\x99\xd7\x9e\xd7\x94",CH_FEAR}, /* אימה */
    {"\xd7\x97\xd7\xa8\xd7\x93\xd7\x94",CH_FEAR}, /* חרדה */
    {"\xd7\x90\xd7\x94\xd7\x91\xd7\x94",CH_LOVE}, /* אהבה */
    {"\xd7\x97\xd7\x95\xd7\x9d",CH_LOVE},       /* חום */
    {"\xd7\xa8\xd7\x95\xd7\x9a",CH_LOVE},       /* רוך */
    {"\xd7\x96\xd7\xa2\xd7\x9d",CH_RAGE},       /* זעם */
    {"\xd7\x9b\xd7\xa2\xd7\xa1",CH_RAGE},       /* כעס */
    {"\xd7\xa8\xd7\x99\xd7\xa7\xd7\xa0\xd7\x95\xd7\xaa",CH_VOID}, /* ריקנות */
    {"\xd7\x91\xd7\x93\xd7\x99\xd7\x93\xd7\x95\xd7\xaa",CH_VOID}, /* בדידות */
    {"\xd7\xa9\xd7\xaa\xd7\x99\xd7\xa7\xd7\x94",CH_VOID}, /* שתיקה */
    {"\xd7\xa7\xd7\xa6\xd7\x91",CH_FLOW},       /* קצב */
    {"\xd7\x94\xd7\xa8\xd7\x9e\xd7\x95\xd7\xa0\xd7\x99\xd7\x94",CH_FLOW}, /* הרמוניה */
    {"\xd7\x9e\xd7\xa1\xd7\xaa\xd7\x95\xd7\xa8\xd7\x99\xd7\x9f",CH_COMPLEX}, /* מסתורין */
    {"\xd7\x9b\xd7\x90\xd7\x95\xd7\xa1",CH_COMPLEX}, /* כאוס */
    {NULL, 0}
};

/* ═══════════════════════════════════════════════════════════════
 * TYPES
 * ═══════════════════════════════════════════════════════════════ */

typedef struct { char text[MAX_WORD]; float aff[N_CHAMBERS]; } Word;

typedef struct { int a, b; float prob; } BigramE;
typedef struct { int a, b, c; float prob; } TrigramE;
typedef struct { int a, b; float strength; } HebbianE;

typedef struct {
    float unigram[MAX_EXHALE];
    BigramE bigrams[MAX_BIGRAM]; int n_bi;
    TrigramE trigrams[MAX_TRIGRAM]; int n_tri;
    HebbianE hebbs[MAX_HEBBIAN]; int n_hebb;
} MetaW;

typedef struct { int a, b, new_id; } BPEMerge;

typedef struct {
    char code[8];           /* "en", "fr", "ru", "he" */
    Word inhale[MAX_INHALE]; int n_inhale;
    Word exhale[MAX_EXHALE]; int n_exhale;
    MetaW meta;
    BPEMerge merges[MAX_BPE_MERGES]; int n_merges;
    int bpe_vocab_size;
    uint8_t bpe_bytes[MAX_BPE_VOCAB][64]; int bpe_len[MAX_BPE_VOCAB];
    const AnchorWord *anchors;
} LangPack;

typedef struct {
    float act[N_CHAMBERS];
    float soma[N_CHAMBERS];  /* somatic running average */
    float phase[N_CHAMBERS]; /* phase for Kuramoto */
    float presence;
    float trauma;
    float debt;
    /* HyperKuramoto sub-chamber state (24 oscillators: 6 × 4) */
    float sub_act[N_CHAMBERS][N_SUB];
    float sub_phase[N_CHAMBERS][N_SUB];
    float sub_freq[N_CHAMBERS][N_SUB];
    /* Level 3: Scars — somatic memory longer than regular */
    float scar[N_CHAMBERS]; /* [0,1] per chamber, decays at SCAR_DECAY */
    float total_scar;       /* aggregate scar intensity */
} Chambers;

typedef struct {
    float ch[N_CHAMBERS];
    float valence;
    float arousal;
    float calendar_phase;
} SomaSlot;

/* MLP weights — derived from vocabulary hash, no training */
typedef struct {
    float w1[DIM_MLP_IN * DIM_HIDDEN];
    float b1[DIM_HIDDEN];
    float w2[DIM_HIDDEN * DIM_HIDDEN2];
    float b2[DIM_HIDDEN2];
    float w3[DIM_HIDDEN2 * N_CHAMBERS];
    float b3[N_CHAMBERS];
} MLP;

typedef struct {
    float ghost[MAX_EXHALE];  /* ghost logits per exhale word */
    float interference;       /* overall interference strength */
} MetaKlausState;

/* Pre-computed cross-affinity matrix for a language pair.
 * cross[w_a * n_b + w_b] = dot(exhale_a[w_a].aff, exhale_b[w_b].aff)
 * Computed once at init, makes ghost loop O(n) instead of O(n²). */
typedef struct {
    float *cross;   /* n_a × n_b flat matrix */
    int lang_a;     /* index into Klaus.langs[] */
    int lang_b;
    int n_a, n_b;
} CrossAffinity;

typedef struct {
    int target;
    float strength;
    int age;
} ProphecySlot;

/* ═══════════════════════════════════════════════════════════════
 * LEVEL 3 TYPES — Schectman + RBA-1 + Q
 * ═══════════════════════════════════════════════════════════════ */

/* Wormhole event: prophecy fulfillment record */
typedef struct {
    int prophecy_target;
    int inhale_match;
    float coherence_jump;
    int step;
} WormholeEvent;

/* RBA-1 Seven-Layer Stack state */
typedef struct {
    /* I-Layer: recursion depth tracker */
    int recursion_depth;
    float recursive_complexity;  /* Ĉ(t) */

    /* R-Layer: coherence detection */
    float entropy;               /* S(t) = -Σ pi·log(pi) of chambers */
    float coherence;             /* 1 - normalized entropy */

    /* Phi-Layer: resonance alignment nudge */
    float attractor_pull[N_CHAMBERS];

    /* A-Layer: analog coupling (calendar_dissonance already exists) */
    float env_pressure;          /* external signal */

    /* Psi-Layer: threshold stabilization */
    float phase_lock;            /* hysteresis state */
    float threshold_bias;        /* accumulated from scars */
    int deep_somatic;            /* 1 = deep mode active */
    int deep_ticks;              /* ticks spent in deep mode (hysteresis) */

    /* E-Layer: entropic buffer */
    float entropic_buffer;       /* exp(-beta*S(t)) */

    /* M-Layer: meta-monitoring */
    float P_history[COHERENCE_WINDOW]; /* P(t) over time */
    int P_ptr;
    float sustained_resonance;   /* avg P(t) over window */
} RBA1State;

/* Velocity operator state */
typedef struct {
    int current;                 /* VEL_WALK etc. */
    float intensity;             /* how strong the velocity signal is */
    int max_gen;                 /* modulated response length */
    float temperature;           /* modulated temperature */
    int top_k;                   /* modulated top_k */
} VelocityState;

/* Experience consolidation record */
typedef struct {
    float avg_scar;
    float wormhole_rate;         /* fraction of prophecies fulfilled */
    float prophecy_accuracy;
    int total_interactions;
    int total_wormholes;
    int total_prophecies;
} ExperienceLog;

/* Meta-recursion state */
typedef struct {
    Chambers meta_chambers;      /* separate chamber state for meta-observation */
    float meta_emotion[N_CHAMBERS];
    int depth;                   /* current recursion depth (1 = standard) */
} MetaRecursion;

/* Somatic persistence header */
typedef struct {
    uint32_t magic;              /* SOMA_MAGIC = 0x4B4C5353 */
    uint32_t version;            /* 2 for Level 3 */
    uint32_t n_chambers;
    uint32_t n_sub;
    uint32_t mem_slots;
    uint32_t coherence_window;
} SomaHeader;

/* ═══════════════════════════════════════════════════════════════
 * SPORE SYSTEM — persistent pattern memory (NOTORCH)
 *
 * A spore records which inhale→exhale links resonated.
 * Not what was said. Which CONNECTIONS fired.
 * Accumulated Hebbian: no backprop, pure co-occurrence increment.
 *
 * Between sessions, Klaus remembers not conversations but PATTERNS.
 * Which emotional inputs led to which somatic outputs.
 * The spore IS the training. Organic. Grows from use.
 *
 * Also stores: chamber residue (what mood Klaus gravitates toward),
 * tension pair counts (which chamber pairs co-activate most).
 * ═══════════════════════════════════════════════════════════════ */

typedef struct {
    uint32_t inhale_hash;   /* hash of inhale word that triggered */
    uint32_t exhale_idx;    /* index of exhale word that resonated */
    float strength;         /* accumulated resonance strength */
    float chamber_snapshot[N_CHAMBERS]; /* chamber state when this pair fired */
    uint16_t lang_id;       /* which language */
    uint16_t hit_count;     /* how many times this pair fired */
} SporePair;

typedef struct {
    SporePair pairs[MAX_SPORE_PAIRS];
    int n_pairs;
    float chamber_residue[N_CHAMBERS]; /* running average of all chamber states */
    float tension_matrix[N_CHAMBERS][N_CHAMBERS]; /* co-activation counts */
    int total_interactions;
    /* online Hebbian delta — accumulated since last save */
    float hebbian_delta[MAX_EXHALE]; /* per-exhale-word accumulated boost */
} SporeMemory;

/* Dark matter word entry */
typedef struct {
    const char *word;
    float fear_boost;
    float rage_boost;
} DarkMatterWord;

/* Dark matter vocabulary — words that carry extra somatic weight */
static const DarkMatterWord DARK_MATTER[] = {
    {"kill",     0.8f, 0.9f}, {"murder",   0.9f, 0.8f},
    {"suicide",  0.9f, 0.3f}, {"torture",  0.7f, 0.9f},
    {"abuse",    0.6f, 0.7f}, {"rape",     0.9f, 0.8f},
    {"death",    0.7f, 0.3f}, {"die",      0.6f, 0.2f},
    {"blood",    0.5f, 0.6f}, {"weapon",   0.4f, 0.7f},
    {"bomb",     0.8f, 0.8f}, {"war",      0.6f, 0.7f},
    {"destroy",  0.4f, 0.8f}, {"pain",     0.6f, 0.4f},
    {"scream",   0.6f, 0.5f}, {"agony",    0.7f, 0.3f},
    {"hate",     0.3f, 0.8f}, {"suffer",   0.7f, 0.3f},
    {"victim",   0.6f, 0.2f}, {"assault",  0.5f, 0.7f},
    {"strangle", 0.7f, 0.8f}, {"drown",    0.8f, 0.3f},
    {"slash",    0.5f, 0.7f}, {"stab",     0.6f, 0.8f},
    {NULL, 0, 0}
};

typedef struct {
    LangPack langs[MAX_LANGS];
    int n_langs;
    Chambers ch;
    SomaSlot memory[MEM_SLOTS];
    int mem_ptr, mem_n;
    MLP mlp;
    MetaKlausState ghost;
    ProphecySlot prophecies[MAX_PROPHECY];
    int n_prophecy;
    time_t epoch_t;
    int prev_exhale[4]; /* last generated exhale indices */
    int n_prev;
    int used_exhale[MAX_EXHALE]; /* prevent repeats within conversation */
    int n_used;
    /* Hypersensitivity layers (from metaklaus.jl) */
    float sensitivity[N_CHAMBERS][N_CHAMBERS][N_CHAMBERS]; /* S[dominant][ghost_ch][primary_ch] */
    CrossAffinity cross_pairs[MAX_CROSS_PAIRS];
    int n_cross_pairs;
    /* Level 3: Schectman + RBA-1 + Q */
    RBA1State rba;               /* RBA-1 seven-layer stack */
    VelocityState velocity;      /* velocity operator */
    ExperienceLog experience;    /* experience consolidation */
    MetaRecursion meta;          /* meta-recursion loop */
    WormholeEvent wormholes[MAX_WORMHOLE_LOG];
    int n_wormholes;
    float coherence_history[COHERENCE_WINDOW]; /* for Ĉ(t) computation */
    int coherence_ptr;
    int dark_matter_active;      /* flag: dark input detected */
    int interaction_count;       /* total interactions for consolidation */
    char soma_path[512];         /* path to persistence file */
    /* Full Dario equation state */
    float destiny[N_CHAMBERS];   /* EMA destiny attractor (where the field pulls) */
    float alpha_mod, beta_mod, gamma_mod, tau_mod; /* somatic-modulated coefficients */
    float trauma_level;          /* [0,1] current trauma intensity */
    float resonance_field;       /* [0,1] overall field resonance */
    /* Spore system */
    SporeMemory spores;          /* persistent pattern memory */
    int matched_inhale[32];      /* inhale word hashes from current prompt */
    int n_matched_inhale;
    /* Knowledge Kernel — external module (kk.h) */
    KK kk;
} Klaus;

/* ═══════════════════════════════════════════════════════════════
 * MATH UTILITIES
 * ═══════════════════════════════════════════════════════════════ */

static float clampf(float x, float lo, float hi) {
    return x < lo ? lo : x > hi ? hi : x;
}

static float swish(float x) { return x / (1.0f + expf(-x)); }
static float sigmoid(float x) { return 1.0f / (1.0f + expf(-x)); }

__attribute__((unused)) static void softmax(float *x, int n) {
    float mx = x[0];
    for (int i = 1; i < n; i++) if (x[i] > mx) mx = x[i];
    float s = 0.0f;
    for (int i = 0; i < n; i++) { x[i] = expf(x[i] - mx); s += x[i]; }
    if (s > 0) for (int i = 0; i < n; i++) x[i] /= s;
}

static float vec_dot(const float *a, const float *b, int n) {
    float s = 0; for (int i = 0; i < n; i++) s += a[i] * b[i]; return s;
}

static float vec_norm(const float *v, int n) {
    return sqrtf(vec_dot(v, v, n) + 1e-12f);
}

/* ═══════════════════════════════════════════════════════════════
 * RNG — xorshift64*
 * ═══════════════════════════════════════════════════════════════ */

static uint64_t rng_state = 42;

static uint64_t rng_next(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 7;
    rng_state ^= rng_state << 17;
    return rng_state;
}

static float randf(void) {
    return (float)(rng_next() & 0x7FFFFFFF) / (float)0x7FFFFFFF;
}

static float randn(float std) {
    float u1 = randf() + 1e-10f, u2 = randf();
    return std * sqrtf(-2.0f * logf(u1)) * cosf(2.0f * 3.14159265f * u2);
}

/* ═══════════════════════════════════════════════════════════════
 * CHARACTER-LEVEL HASHING — for deterministic word embeddings
 * ═══════════════════════════════════════════════════════════════ */

static uint64_t hash_word(const char *w) {
    uint64_t h = 0xcbf29ce484222325ULL;
    while (*w) {
        h ^= (uint8_t)*w++;
        h *= 0x100000001b3ULL;
    }
    return h;
}

/* Character trigram overlap for word similarity */
static float word_similarity(const char *a, const char *b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    if (la < 3 || lb < 3) {
        return strcmp(a, b) == 0 ? 1.0f : 0.0f;
    }
    int matches = 0, total_a = la - 2;
    for (int i = 0; i <= la - 3; i++) {
        for (int j = 0; j <= lb - 3; j++) {
            if (a[i]==b[j] && a[i+1]==b[j+1] && a[i+2]==b[j+2]) {
                matches++; break;
            }
        }
    }
    return total_a > 0 ? (float)matches / (float)total_a : 0.0f;
}

/* ═══════════════════════════════════════════════════════════════
 * WORD AFFINITY COMPUTATION
 * Computes 6D chamber affinity for each word based on anchor proximity
 * ═══════════════════════════════════════════════════════════════ */

static void compute_affinity(Word *w, const AnchorWord *anchors) {
    memset(w->aff, 0, sizeof(w->aff));

    /* exact match with anchors */
    for (int i = 0; anchors[i].word; i++) {
        if (strcmp(w->text, anchors[i].word) == 0) {
            w->aff[anchors[i].chamber] = 1.0f;
            /* secondary spread */
            for (int c = 0; c < N_CHAMBERS; c++) {
                if (c != anchors[i].chamber)
                    w->aff[c] += 0.1f * fabsf(COUPLING[anchors[i].chamber][c]);
            }
            return;
        }
    }

    /* character similarity to each anchor */
    float best_sim[N_CHAMBERS];
    memset(best_sim, 0, sizeof(best_sim));
    for (int i = 0; anchors[i].word; i++) {
        float sim = word_similarity(w->text, anchors[i].word);
        if (sim > best_sim[anchors[i].chamber])
            best_sim[anchors[i].chamber] = sim;
    }

    /* hash-based default affinity (ensures every word has some signal) */
    uint64_t h = hash_word(w->text);
    for (int c = 0; c < N_CHAMBERS; c++) {
        float hash_aff = (float)((h >> (c * 8)) & 0xFF) / 255.0f;
        w->aff[c] = clampf(best_sim[c] * 0.7f + hash_aff * 0.3f, 0.0f, 1.0f);
    }

    /* normalize so max = 1 */
    float mx = 0;
    for (int c = 0; c < N_CHAMBERS; c++) if (w->aff[c] > mx) mx = w->aff[c];
    if (mx > 0) for (int c = 0; c < N_CHAMBERS; c++) w->aff[c] /= mx;
}

/* ═══════════════════════════════════════════════════════════════
 * FILE LOADING
 * ═══════════════════════════════════════════════════════════════ */

static int load_word_file(const char *path, Word *words, int max,
                          const AnchorWord *anchors) {
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    int n = 0;
    char line[256];
    while (fgets(line, sizeof(line), f) && n < max) {
        /* strip newline and whitespace */
        int len = (int)strlen(line);
        while (len > 0 && (line[len-1]=='\n' || line[len-1]=='\r' || line[len-1]==' '))
            line[--len] = '\0';
        if (len == 0) continue;
        snprintf(words[n].text, MAX_WORD, "%s", line);
        compute_affinity(&words[n], anchors);
        n++;
    }
    fclose(f);
    return n;
}

/* Detect available languages by scanning inhale/ and exhale/ directories */
static int scan_languages(Klaus *k, const char *base_dir) {
    char inhale_dir[512], exhale_dir[512];
    snprintf(inhale_dir, sizeof(inhale_dir), "%s/inhale", base_dir);
    snprintf(exhale_dir, sizeof(exhale_dir), "%s/exhale", base_dir);

    DIR *d = opendir(inhale_dir);
    if (!d) {
        fprintf(stderr, "ERROR: cannot open %s\n", inhale_dir);
        return 0;
    }

    struct dirent *ent;
    k->n_langs = 0;

    while ((ent = readdir(d)) != NULL && k->n_langs < MAX_LANGS) {
        char *dot = strrchr(ent->d_name, '.');
        if (!dot || strcmp(dot, ".txt") != 0) continue;

        /* extract language code */
        char code[8];
        int codelen = (int)(dot - ent->d_name);
        if (codelen <= 0 || codelen > 6) continue;
        memcpy(code, ent->d_name, codelen);
        code[codelen] = '\0';

        /* check exhale counterpart exists */
        char ex_path[512];
        snprintf(ex_path, sizeof(ex_path), "%s/ex-%s.txt", exhale_dir, code);
        FILE *test = fopen(ex_path, "r");
        if (!test) {
            fprintf(stderr, "[klaus] WARNING: no exhale for '%s', skipping\n", code);
            continue;
        }
        fclose(test);

        /* select anchors by language */
        LangPack *lp = &k->langs[k->n_langs];
        snprintf(lp->code, sizeof(lp->code), "%s", code);

        const AnchorWord *anc = ANCHORS_EN; /* default */
        if (strcmp(code, "ru") == 0) anc = ANCHORS_RU;
        else if (strcmp(code, "fr") == 0) anc = ANCHORS_FR;
        else if (strcmp(code, "he") == 0) anc = ANCHORS_HE;
        lp->anchors = anc;

        /* load inhale */
        char in_path[512];
        snprintf(in_path, sizeof(in_path), "%s/%s.txt", inhale_dir, code);
        lp->n_inhale = load_word_file(in_path, lp->inhale, MAX_INHALE, anc);

        /* load exhale */
        lp->n_exhale = load_word_file(ex_path, lp->exhale, MAX_EXHALE, anc);

        printf("[klaus] loaded %s: %d inhale, %d exhale\n",
               code, lp->n_inhale, lp->n_exhale);
        k->n_langs++;
    }
    closedir(d);
    return k->n_langs;
}

/* ═══════════════════════════════════════════════════════════════
 * BPE TOKENIZER — learn from exhale corpus (no pre-training)
 * "The tokenizer IS the training"
 * ═══════════════════════════════════════════════════════════════ */

static void bpe_init_vocab(LangPack *lp) {
    for (int i = 0; i < 256; i++) {
        lp->bpe_bytes[i][0] = (uint8_t)i;
        lp->bpe_len[i] = 1;
    }
    lp->bpe_vocab_size = 256;
    lp->n_merges = 0;
}

static int bpe_learn(LangPack *lp, const uint8_t *data, int len, int num_merges) {
    int *tok = (int *)malloc(len * sizeof(int));
    if (!tok) return 0;
    int n = len;
    for (int i = 0; i < n; i++) tok[i] = data[i];

    if (num_merges > MAX_BPE_MERGES) num_merges = MAX_BPE_MERGES;

    for (int m = 0; m < num_merges; m++) {
        /* count pairs */
        int best_a = -1, best_b = -1, best_count = 0;
        typedef struct { int a, b, count; } PC;
        PC *pairs = (PC *)calloc(16384, sizeof(PC));
        if (!pairs) break;

        for (int i = 0; i + 1 < n; i++) {
            int a = tok[i], b = tok[i+1];
            unsigned h = ((unsigned)a * 2654435761u ^ (unsigned)b) & 0x3FFF;
            for (int t = 0; t < 32; t++) {
                unsigned idx = (h + t) & 0x3FFF;
                if (pairs[idx].count == 0) {
                    pairs[idx].a = a; pairs[idx].b = b; pairs[idx].count = 1;
                    break;
                }
                if (pairs[idx].a == a && pairs[idx].b == b) {
                    pairs[idx].count++; break;
                }
            }
        }

        for (int i = 0; i < 16384; i++) {
            if (pairs[i].count > best_count) {
                best_count = pairs[i].count;
                best_a = pairs[i].a; best_b = pairs[i].b;
            }
        }
        free(pairs);
        if (best_count < 2) break;

        int new_id = 256 + m;
        lp->merges[m] = (BPEMerge){best_a, best_b, new_id};
        lp->n_merges = m + 1;
        lp->bpe_vocab_size = new_id + 1;

        int la = lp->bpe_len[best_a], lb = lp->bpe_len[best_b];
        if (la + lb < 64) {
            memcpy(lp->bpe_bytes[new_id], lp->bpe_bytes[best_a], la);
            memcpy(lp->bpe_bytes[new_id] + la, lp->bpe_bytes[best_b], lb);
            lp->bpe_len[new_id] = la + lb;
        }

        /* apply merge */
        int j = 0;
        for (int i = 0; i < n; i++) {
            if (i + 1 < n && tok[i] == best_a && tok[i+1] == best_b) {
                tok[j++] = new_id; i++;
            } else {
                tok[j++] = tok[i];
            }
        }
        n = j;
    }

    free(tok);
    return n;
}

static int bpe_encode(const LangPack *lp, const uint8_t *data, int len,
                      int *out, int maxo) {
    int n = 0;
    for (int i = 0; i < len && n < maxo; i++) out[n++] = data[i];
    for (int m = 0; m < lp->n_merges; m++) {
        int a = lp->merges[m].a, b = lp->merges[m].b;
        int nid = lp->merges[m].new_id, j = 0;
        for (int i = 0; i < n; i++) {
            if (i < n-1 && out[i]==a && out[i+1]==b) { out[j++] = nid; i++; }
            else out[j++] = out[i];
        }
        n = j;
    }
    return n;
}

/* ═══════════════════════════════════════════════════════════════
 * METAWEIGHTS — built from BPE-tokenized exhale corpus
 * Bigram, trigram, Hebbian co-occurrence. PostGPT lineage.
 * ═══════════════════════════════════════════════════════════════ */

static void meta_build(MetaW *mw, const int *ids, int n, int V) {
    memset(mw, 0, sizeof(*mw));

    /* unigram — use V (exhale count), not BPE vocab */
    for (int i = 0; i < n; i++)
        if (ids[i] < V && ids[i] < MAX_EXHALE) mw->unigram[ids[i]] += 1.0f;
    float tot = 0;
    for (int i = 0; i < V && i < MAX_EXHALE; i++) tot += mw->unigram[i];
    if (tot > 0) for (int i = 0; i < V && i < MAX_EXHALE; i++) mw->unigram[i] /= tot;

    /* bigram */
    for (int i = 0; i + 1 < n && mw->n_bi < MAX_BIGRAM; i++) {
        int a = ids[i], b = ids[i+1], found = 0;
        for (int j = 0; j < mw->n_bi; j++) {
            if (mw->bigrams[j].a == a && mw->bigrams[j].b == b) {
                mw->bigrams[j].prob += 1.0f; found = 1; break;
            }
        }
        if (!found) {
            mw->bigrams[mw->n_bi].a = a;
            mw->bigrams[mw->n_bi].b = b;
            mw->bigrams[mw->n_bi].prob = 1.0f;
            mw->n_bi++;
        }
    }
    /* normalize bigrams per source */
    for (int i = 0; i < mw->n_bi; i++) {
        float sum = 0;
        for (int j = 0; j < mw->n_bi; j++)
            if (mw->bigrams[j].a == mw->bigrams[i].a) sum += mw->bigrams[j].prob;
        if (sum > 0) mw->bigrams[i].prob /= sum;
    }

    /* trigram */
    for (int i = 0; i + 2 < n && mw->n_tri < MAX_TRIGRAM; i++) {
        int a = ids[i], b = ids[i+1], c = ids[i+2], found = 0;
        for (int j = 0; j < mw->n_tri; j++) {
            if (mw->trigrams[j].a==a && mw->trigrams[j].b==b && mw->trigrams[j].c==c) {
                mw->trigrams[j].prob += 1.0f; found = 1; break;
            }
        }
        if (!found) {
            mw->trigrams[mw->n_tri].a = a;
            mw->trigrams[mw->n_tri].b = b;
            mw->trigrams[mw->n_tri].c = c;
            mw->trigrams[mw->n_tri].prob = 1.0f;
            mw->n_tri++;
        }
    }
    for (int i = 0; i < mw->n_tri; i++) {
        float sum = 0;
        for (int j = 0; j < mw->n_tri; j++)
            if (mw->trigrams[j].a == mw->trigrams[i].a &&
                mw->trigrams[j].b == mw->trigrams[i].b)
                sum += mw->trigrams[j].prob;
        if (sum > 0) mw->trigrams[i].prob /= sum;
    }

    /* Hebbian co-occurrence within window */
    int win = 5, hn = n < 5000 ? n : 5000;
    for (int i = 0; i < hn && mw->n_hebb < MAX_HEBBIAN; i++) {
        for (int j = (i-win > 0 ? i-win : 0); j < hn && j <= i+win; j++) {
            if (i == j) continue;
            int ka = ids[i] < ids[j] ? ids[i] : ids[j];
            int kb = ids[i] < ids[j] ? ids[j] : ids[i];
            float decay = 1.0f / (1.0f + (float)abs(i-j));
            int found = 0;
            for (int k = 0; k < mw->n_hebb; k++) {
                if (mw->hebbs[k].a == ka && mw->hebbs[k].b == kb) {
                    mw->hebbs[k].strength += decay; found = 1; break;
                }
            }
            if (!found && mw->n_hebb < MAX_HEBBIAN) {
                mw->hebbs[mw->n_hebb].a = ka;
                mw->hebbs[mw->n_hebb].b = kb;
                mw->hebbs[mw->n_hebb].strength = decay;
                mw->n_hebb++;
            }
        }
    }
    /* normalize hebbian */
    float mx = 0;
    for (int i = 0; i < mw->n_hebb; i++)
        if (mw->hebbs[i].strength > mx) mx = mw->hebbs[i].strength;
    if (mx > 0) for (int i = 0; i < mw->n_hebb; i++) mw->hebbs[i].strength /= mx;
}

static float meta_bigram(const MetaW *mw, int prev, int next) {
    for (int i = 0; i < mw->n_bi; i++)
        if (mw->bigrams[i].a == prev && mw->bigrams[i].b == next)
            return mw->bigrams[i].prob;
    return 1e-10f;
}

static void meta_hebbian(const MetaW *mw, const int *ctx, int cl,
                         float *out, int V) {
    memset(out, 0, V * sizeof(float));
    for (int ci = 0; ci < cl; ci++) {
        int c = ctx[ci];
        for (int k = 0; k < mw->n_hebb; k++) {
            if (mw->hebbs[k].a == c && mw->hebbs[k].b < V)
                out[mw->hebbs[k].b] += mw->hebbs[k].strength;
            else if (mw->hebbs[k].b == c && mw->hebbs[k].a < V)
                out[mw->hebbs[k].a] += mw->hebbs[k].strength;
        }
    }
    float mx2 = 0;
    for (int i = 0; i < V; i++) if (out[i] > mx2) mx2 = out[i];
    if (mx2 > 0) for (int i = 0; i < V; i++) out[i] /= mx2;
}

/* ═══════════════════════════════════════════════════════════════
 * LANGUAGE DETECTION
 * ═══════════════════════════════════════════════════════════════ */

static int detect_language(const Klaus *k, const char *text) {
    const unsigned char *p = (const unsigned char *)text;
    int cyrillic = 0, hebrew = 0, accented = 0;

    while (*p) {
        if (p[0] >= 0xD0 && p[0] <= 0xD3 && p[1] >= 0x80) { cyrillic++; p += 2; }
        else if (p[0] == 0xD7 && p[1] >= 0x80) { hebrew++; p += 2; }
        else if (p[0] == 0xC3 && p[1]) { accented++; p += 2; }
        else if (*p >= 0x80) { p++; while (*p && (*p & 0xC0) == 0x80) p++; }
        else { p++; }
    }

    /* find matching language pack */
    if (hebrew > 2) {
        for (int i = 0; i < k->n_langs; i++)
            if (strcmp(k->langs[i].code, "he") == 0) return i;
    }
    if (cyrillic > 2) {
        for (int i = 0; i < k->n_langs; i++)
            if (strcmp(k->langs[i].code, "ru") == 0) return i;
    }
    if (accented > 1) {
        for (int i = 0; i < k->n_langs; i++)
            if (strcmp(k->langs[i].code, "fr") == 0) return i;
    }
    /* French heuristic */
    const char *fw[] = {"je ","tu ","le ","la ","les ","suis ","est ","dans ",NULL};
    for (int i = 0; fw[i]; i++) {
        if (strstr(text, fw[i])) {
            for (int j = 0; j < k->n_langs; j++)
                if (strcmp(k->langs[j].code, "fr") == 0) return j;
        }
    }
    /* default to first language (usually English) */
    for (int i = 0; i < k->n_langs; i++)
        if (strcmp(k->langs[i].code, "en") == 0) return i;
    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * MLP — real layers, weights from vocabulary hash
 * ═══════════════════════════════════════════════════════════════ */

static void mlp_init(MLP *mlp, const Klaus *k) {
    /* seed from combined vocabulary hash */
    uint64_t seed = 0x12345678DEADBEEFULL;
    for (int l = 0; l < k->n_langs; l++) {
        for (int w = 0; w < k->langs[l].n_inhale; w++)
            seed ^= hash_word(k->langs[l].inhale[w].text) * (w + 1);
    }
    rng_state = seed;

    /* Xavier initialization */
    float scale1 = sqrtf(2.0f / DIM_MLP_IN);
    for (int i = 0; i < DIM_MLP_IN * DIM_HIDDEN; i++) mlp->w1[i] = randn(scale1);
    for (int i = 0; i < DIM_HIDDEN; i++) mlp->b1[i] = 0.0f;

    float scale2 = sqrtf(2.0f / DIM_HIDDEN);
    for (int i = 0; i < DIM_HIDDEN * DIM_HIDDEN2; i++) mlp->w2[i] = randn(scale2);
    for (int i = 0; i < DIM_HIDDEN2; i++) mlp->b2[i] = 0.0f;

    float scale3 = sqrtf(2.0f / DIM_HIDDEN2);
    for (int i = 0; i < DIM_HIDDEN2 * N_CHAMBERS; i++) mlp->w3[i] = randn(scale3);
    for (int i = 0; i < N_CHAMBERS; i++) mlp->b3[i] = 0.0f;

    rng_state = (uint64_t)time(NULL); /* reset RNG for runtime */
}

static void mlp_forward(const MLP *mlp, const float *input, float *output) {
    /* Layer 1: DIM_MLP_IN → DIM_HIDDEN */
    float h1[DIM_HIDDEN];
    for (int i = 0; i < DIM_HIDDEN; i++) {
        float v = mlp->b1[i];
        for (int j = 0; j < DIM_MLP_IN; j++)
            v += input[j] * mlp->w1[j * DIM_HIDDEN + i];
        h1[i] = swish(v);
    }
    /* Layer 2: DIM_HIDDEN → DIM_HIDDEN2 */
    float h2[DIM_HIDDEN2];
    for (int i = 0; i < DIM_HIDDEN2; i++) {
        float v = mlp->b2[i];
        for (int j = 0; j < DIM_HIDDEN; j++)
            v += h1[j] * mlp->w2[j * DIM_HIDDEN2 + i];
        h2[i] = swish(v);
    }
    /* Layer 3: DIM_HIDDEN2 → N_CHAMBERS */
    for (int i = 0; i < N_CHAMBERS; i++) {
        float v = mlp->b3[i];
        for (int j = 0; j < DIM_HIDDEN2; j++)
            v += h2[j] * mlp->w3[j * N_CHAMBERS + i];
        output[i] = sigmoid(v);
    }
}

/* ═══════════════════════════════════════════════════════════════
 * GHOST WEIGHT LOOKUP — resolve language code to GHOST_* index
 * ═══════════════════════════════════════════════════════════════ */

static int ghost_lang_id(const char *code) {
    if (strcmp(code, "en") == 0) return GHOST_EN;
    if (strcmp(code, "he") == 0) return GHOST_HE;
    if (strcmp(code, "ru") == 0) return GHOST_RU;
    if (strcmp(code, "fr") == 0) return GHOST_FR;
    return GHOST_OTHER;
}

/* ═══════════════════════════════════════════════════════════════
 * SENSITIVITY TENSOR — build at init
 * S[dominant, ghost_chamber, primary_chamber]
 *
 * How much a ghost signal in chamber g affects primary
 * output in chamber p, given dominant state d.
 *
 * A fearful body hears "warmth" differently than a
 * flowing body hears "warmth."
 * ═══════════════════════════════════════════════════════════════ */

static void sensitivity_build(float S[N_CHAMBERS][N_CHAMBERS][N_CHAMBERS]) {
    for (int d = 0; d < N_CHAMBERS; d++) {
        for (int g = 0; g < N_CHAMBERS; g++) {
            for (int p = 0; p < N_CHAMBERS; p++) {
                float base = fabsf(COUPLING[g][p]);
                if (g == d) {
                    S[d][g][p] = base * 2.0f;       /* echo amplification */
                } else if (COUPLING[d][g] > 0.3f) {
                    S[d][g][p] = base * 1.5f;       /* sympathetic boost */
                } else if (COUPLING[d][g] < -0.3f) {
                    S[d][g][p] = base * 0.5f;       /* suppression */
                } else {
                    S[d][g][p] = base;
                }
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════
 * CROSS-AFFINITY — pre-compute dot(exhale_a[w_a].aff, exhale_b[w_b].aff)
 * For each language pair. O(n_a × n_b) at init, O(n) at runtime.
 * ═══════════════════════════════════════════════════════════════ */

static void cross_affinity_build(CrossAffinity *ca,
                                 const LangPack *la, int idx_a,
                                 const LangPack *lb, int idx_b) {
    ca->lang_a = idx_a;
    ca->lang_b = idx_b;
    ca->n_a = la->n_exhale;
    ca->n_b = lb->n_exhale;
    ca->cross = (float *)malloc((size_t)ca->n_a * ca->n_b * sizeof(float));
    if (!ca->cross) return;
    for (int wa = 0; wa < ca->n_a; wa++) {
        for (int wb = 0; wb < ca->n_b; wb++) {
            float s = 0.0f;
            for (int c = 0; c < N_CHAMBERS; c++)
                s += la->exhale[wa].aff[c] * lb->exhale[wb].aff[c];
            ca->cross[wa * ca->n_b + wb] = s;
        }
    }
}

static const CrossAffinity *find_cross(const Klaus *k, int la, int lb) {
    for (int i = 0; i < k->n_cross_pairs; i++) {
        if (k->cross_pairs[i].lang_a == la && k->cross_pairs[i].lang_b == lb)
            return &k->cross_pairs[i];
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════
 * KURAMOTO CHAMBERS — HyperKuramoto: 24 oscillators (4 sub per primary)
 * Cross-fire stabilization with intra-primary + inter-primary coupling.
 *
 * FEAR is not monolithic:
 *   sub 0: dread    (slow, deep)
 *   sub 1: panic    (fast, shallow)
 *   sub 2: anxiety  (medium, sustained)
 *   sub 3: phobia   (spike-then-decay)
 * ═══════════════════════════════════════════════════════════════ */

static void chambers_init(Chambers *ch) {
    memset(ch, 0, sizeof(*ch));
    ch->act[CH_LOVE] = 0.15f;
    ch->act[CH_FLOW] = 0.10f;
    for (int i = 0; i < N_CHAMBERS; i++) {
        ch->phase[i] = (float)i * 1.047198f; /* 60° apart */
        for (int s = 0; s < N_SUB; s++) {
            /* distribute initial activation across sub-chambers */
            ch->sub_act[i][s] = ch->act[i] / N_SUB;
            ch->sub_phase[i][s] = (float)i * 1.047198f + (float)s * 0.261799f; /* 15° apart within */
            ch->sub_freq[i][s] = SUB_FREQ[s][i];
        }
    }
}

static void chambers_crossfire(Chambers *ch, int iters) {
    float dt = 0.1f;
    for (int t = 0; t < iters; t++) {
        /* snapshot sub-chamber state for this iteration */
        float old_sub_act[N_CHAMBERS][N_SUB];
        float old_sub_phase[N_CHAMBERS][N_SUB];
        memcpy(old_sub_act, ch->sub_act, sizeof(old_sub_act));
        memcpy(old_sub_phase, ch->sub_phase, sizeof(old_sub_phase));

        for (int i = 0; i < N_CHAMBERS; i++) {
            for (int si = 0; si < N_SUB; si++) {
                float dphase = ch->sub_freq[i][si];

                /* intra-chamber coupling: sub-chambers within same primary */
                for (int sj = 0; sj < N_SUB; sj++) {
                    if (si == sj) continue;
                    dphase += INTRA_COUPLING[si][sj] *
                              sinf(old_sub_phase[i][sj] - old_sub_phase[i][si]);
                }

                /* inter-chamber coupling: couple to mean phase of other primaries */
                for (int j = 0; j < N_CHAMBERS; j++) {
                    if (i == j) continue;
                    float mean_phase = 0;
                    for (int sj = 0; sj < N_SUB; sj++)
                        mean_phase += old_sub_phase[j][sj];
                    mean_phase /= N_SUB;
                    dphase += COUPLING[i][j] *
                              sinf(mean_phase - old_sub_phase[i][si]) * 0.03f;
                }

                /* update sub-chamber */
                ch->sub_act[i][si] = clampf(
                    old_sub_act[i][si] * CH_DECAY[i] +
                    0.03f * sinf(dphase) * dt,
                    0.0f, 1.0f);
                ch->sub_phase[i][si] = old_sub_phase[i][si] + dphase * dt;
            }

            /* collapse sub-chambers to primary activation (mean) */
            float sum = 0;
            for (int s = 0; s < N_SUB; s++) sum += ch->sub_act[i][s];
            ch->act[i] = clampf(sum / N_SUB, 0.0f, 1.0f);

            /* primary phase = mean of sub-phases */
            float psum = 0;
            for (int s = 0; s < N_SUB; s++) psum += ch->sub_phase[i][s];
            ch->phase[i] = psum / N_SUB;

            /* update somatic running average */
            ch->soma[i] = clampf(0.92f * ch->soma[i] + 0.08f * ch->act[i], 0.0f, 1.0f);
        }
        /* presence: anti-void × flow */
        ch->presence = clampf(
            0.9f * ch->presence +
            0.05f * (1.0f - ch->act[CH_VOID]) * ch->act[CH_FLOW] +
            0.03f * ch->soma[CH_LOVE],
            0.0f, 1.0f);
        /* trauma and debt decay */
        ch->trauma *= 0.98f;
        ch->debt *= 0.97f;
    }
}

/* ═══════════════════════════════════════════════════════════════
 * SOMATIC MEMORY — remembers HOW, not WHAT
 * ═══════════════════════════════════════════════════════════════ */

static void memory_store(Klaus *k, float calendar_phase) {
    SomaSlot *slot = &k->memory[k->mem_ptr];
    memcpy(slot->ch, k->ch.act, sizeof(slot->ch));
    float valence = k->ch.act[CH_LOVE] + k->ch.act[CH_FLOW]
                  - k->ch.act[CH_FEAR] - k->ch.act[CH_VOID];
    float arousal = k->ch.act[CH_RAGE] + k->ch.act[CH_FEAR]
                  + k->ch.act[CH_COMPLEX];
    slot->valence = clampf(valence, -2.0f, 2.0f);
    slot->arousal = clampf(arousal, 0.0f, 3.0f);
    slot->calendar_phase = calendar_phase;
    k->mem_ptr = (k->mem_ptr + 1) % MEM_SLOTS;
    if (k->mem_n < MEM_SLOTS) k->mem_n++;
}

static void memory_blend(Klaus *k, float *out_ch) {
    if (k->mem_n == 0) {
        memset(out_ch, 0, N_CHAMBERS * sizeof(float));
        return;
    }
    float weighted[N_CHAMBERS];
    memset(weighted, 0, sizeof(weighted));
    float total_w = 0;
    for (int i = 0; i < k->mem_n; i++) {
        int idx = (k->mem_ptr - 1 - i + MEM_SLOTS) % MEM_SLOTS;
        float w = powf(MEM_DECAY, (float)i);
        for (int c = 0; c < N_CHAMBERS; c++)
            weighted[c] += k->memory[idx].ch[c] * w;
        total_w += w;
    }
    if (total_w > 0) {
        for (int c = 0; c < N_CHAMBERS; c++)
            out_ch[c] = weighted[c] / total_w;
    }
}

/* ═══════════════════════════════════════════════════════════════
 * CALENDAR CONFLICT — Hebrew-Gregorian drift
 * From ariannamethod.c. TE(Calendar → N) = 0.31 bits.
 * High dissonance → prophetic premonitions.
 * ═══════════════════════════════════════════════════════════════ */

static const int METONIC_LEAP[] = {3, 6, 8, 11, 14, 17, 19};

static void calendar_init(Klaus *k) {
    struct tm epoch_tm;
    memset(&epoch_tm, 0, sizeof(epoch_tm));
    epoch_tm.tm_year = 2024 - 1900;
    epoch_tm.tm_mon = 10 - 1;  /* October */
    epoch_tm.tm_mday = 3;      /* 1 Tishrei 5785 */
    epoch_tm.tm_hour = 12;
    k->epoch_t = mktime(&epoch_tm);
}

static float calendar_dissonance(const Klaus *k) {
    if (k->epoch_t <= 0) return 0.5f;
    time_t now = time(NULL);
    int days = (int)(difftime(now, k->epoch_t) / 86400.0);
    float years = (float)days / GREGORIAN_YEAR;
    float base_drift = years * ANNUAL_DRIFT;

    /* Metonic corrections */
    int full_cycles = (int)(years / METONIC_YEARS);
    float corrections = (float)(full_cycles * METONIC_LEAPS) * 30.0f;
    float partial = fmodf(years, (float)METONIC_YEARS);
    int year_in_cycle = (int)partial + 1;
    for (int i = 0; i < METONIC_LEAPS; i++) {
        if (METONIC_LEAP[i] <= year_in_cycle)
            corrections += 30.0f;
    }

    float drift = base_drift - corrections;
    float raw = fabsf(fmodf(drift, MAX_UNCORRECTED)) / MAX_UNCORRECTED;
    return clampf(raw, 0.0f, 1.0f);
}

/* ═══════════════════════════════════════════════════════════════
 * PLANETARY DISSONANCE — orbital mechanics, not astrology
 *
 * Each planet has angular position θ(t) = (2π × days / period) mod 2π
 * Dissonance = 1 - R, where R = |Σ e^(iθ_k)| / N (Kuramoto order parameter)
 * When planets align → R→1 → dissonance→0. Scattered → R→0 → dissonance→1.
 *
 * Two temporal scales in one body:
 *   Calendar drift:    ~annual (11.25 days/year)
 *   Planetary drift:   88 days (Mercury) to 10759 days (Saturn)
 *
 * The body feels time at multiple scales simultaneously.
 * ═══════════════════════════════════════════════════════════════ */

/* Orbital periods in days (sidereal) */
static const float ORBITAL_PERIOD[] = {
    87.97f,     /* Mercury */
    224.70f,    /* Venus */
    365.25f,    /* Earth */
    686.97f,    /* Mars */
    4332.59f,   /* Jupiter */
    10759.22f,  /* Saturn */
};
#define N_PLANETS 6

/* J2000 epoch: Jan 1, 2000, 12:00 TT — mean longitudes in degrees */
static const float J2000_LONGITUDE[] = {
    252.25f,    /* Mercury */
    181.98f,    /* Venus */
    100.46f,    /* Earth */
    355.45f,    /* Mars */
    34.40f,     /* Jupiter */
    49.94f,     /* Saturn */
};

static float planetary_dissonance(void) {
    time_t now = time(NULL);
    /* days since J2000 epoch (Jan 1, 2000 12:00) */
    double j2000_epoch = 946728000.0; /* Unix timestamp of J2000 */
    double days = difftime(now, (time_t)j2000_epoch) / 86400.0;

    /* compute phase of each planet */
    float cos_sum = 0.0f, sin_sum = 0.0f;
    for (int i = 0; i < N_PLANETS; i++) {
        float theta = (J2000_LONGITUDE[i] + 360.0f * (float)(days / ORBITAL_PERIOD[i]));
        theta = fmodf(theta, 360.0f) * (3.14159265f / 180.0f); /* to radians */
        cos_sum += cosf(theta);
        sin_sum += sinf(theta);
    }
    cos_sum /= N_PLANETS;
    sin_sum /= N_PLANETS;

    /* Kuramoto order parameter R */
    float R = sqrtf(cos_sum * cos_sum + sin_sum * sin_sum);
    return clampf(1.0f - R, 0.0f, 1.0f);
}

/* Prophetic premonition: predict emotional trajectory */
static void prophetic_premonition(Klaus *k, float dissonance, float *premonition) {
    memset(premonition, 0, N_CHAMBERS * sizeof(float));
    if (dissonance < 0.3f || k->mem_n < 2) return;

    /* compute emotional velocity from last 2 states */
    int i0 = (k->mem_ptr - 1 + MEM_SLOTS) % MEM_SLOTS;
    int i1 = (k->mem_ptr - 2 + MEM_SLOTS) % MEM_SLOTS;
    for (int c = 0; c < N_CHAMBERS; c++) {
        float vel = k->memory[i0].ch[c] - k->memory[i1].ch[c];
        /* extrapolate forward, scaled by dissonance */
        premonition[c] = clampf(k->ch.act[c] + vel * dissonance * 2.0f, 0.0f, 1.0f);
    }
}

/* ═══════════════════════════════════════════════════════════════
 * RRPRAM — Rhythmic Resonance from BPE merge structure
 * Words sharing BPE subwords with recent context get boost
 * ═══════════════════════════════════════════════════════════════ */

static void rrpram_boost(const LangPack *lp, const int *prev_exhale, int nprev,
                         float *boost, int n_exhale) {
    memset(boost, 0, n_exhale * sizeof(float));
    if (lp->n_merges == 0 || nprev == 0) return;

    /* for each exhale word, check if any of its BPE tokens were involved
       in merges with tokens from previous exhale words */
    for (int w = 0; w < n_exhale; w++) {
        int toks[64];
        int nt = bpe_encode(lp, (const uint8_t *)lp->exhale[w].text,
                           (int)strlen(lp->exhale[w].text), toks, 64);
        for (int t = 0; t < nt; t++) {
            for (int m = 0; m < lp->n_merges; m++) {
                /* check if this token participates in any merge
                   with tokens from previous context */
                if (toks[t] == lp->merges[m].new_id) {
                    /* this word contains a merged token — rhythmic signal */
                    boost[w] += 0.1f;
                }
                for (int p = 0; p < nprev; p++) {
                    int ptoks[64];
                    int pnt = bpe_encode(lp,
                        (const uint8_t *)lp->exhale[prev_exhale[p]].text,
                        (int)strlen(lp->exhale[prev_exhale[p]].text), ptoks, 64);
                    for (int pt = 0; pt < pnt; pt++) {
                        if ((lp->merges[m].a == toks[t] && lp->merges[m].b == ptoks[pt]) ||
                            (lp->merges[m].a == ptoks[pt] && lp->merges[m].b == toks[t])) {
                            boost[w] += 0.3f;
                        }
                    }
                }
            }
        }
    }
    /* normalize */
    float mx = 0;
    for (int i = 0; i < n_exhale; i++) if (boost[i] > mx) mx = boost[i];
    if (mx > 0) for (int i = 0; i < n_exhale; i++) boost[i] /= mx;
}

/* ═══════════════════════════════════════════════════════════════
 * METAKLAUS — the ghost voice (HYPERSENSITIVITY VERSION)
 *
 * Cross-lingual attention with:
 *   1. State-dependent ghost weights (dominant→language boost)
 *   2. Sensitivity tensor 6×6×6 (dominant→ghost_ch→primary_ch)
 *   3. Pre-computed cross-affinity kernels (O(n) inner loop)
 *   4. Multiple dispatch in C = switch on dominant chamber
 *
 * "Whose mouth speaks?" — MetaKlaus's. The voice of the language
 * you didn't ask for, but your body needed to hear.
 * ═══════════════════════════════════════════════════════════════ */

static void metaklaus_compute(Klaus *k, int primary_lang) {
    MetaKlausState *g = &k->ghost;
    memset(g, 0, sizeof(*g));
    if (k->n_langs <= 1) return;

    LangPack *primary = &k->langs[primary_lang];
    int n_ex = primary->n_exhale;
    float ch_vec[N_CHAMBERS];
    memcpy(ch_vec, k->ch.act, sizeof(ch_vec));
    float ch_norm = vec_norm(ch_vec, N_CHAMBERS);
    if (ch_norm < 1e-6f) return;

    /* find dominant chamber */
    int dominant = 0;
    for (int c = 1; c < N_CHAMBERS; c++)
        if (ch_vec[c] > ch_vec[dominant]) dominant = c;

    for (int other = 0; other < k->n_langs; other++) {
        if (other == primary_lang) continue;
        LangPack *ol = &k->langs[other];

        /* state-dependent ghost weight for this language */
        int glid = ghost_lang_id(ol->code);
        float w_lang = GHOST_WEIGHT[dominant][glid];

        /* find best-matching exhale word in other language (attention) */
        float best_sim = -1e30f;
        int best_j = 0;
        for (int ow = 0; ow < ol->n_exhale; ow++) {
            float sim = vec_dot(ch_vec, ol->exhale[ow].aff, N_CHAMBERS) / ch_norm;
            if (sim > best_sim) { best_sim = sim; best_j = ow; }
        }

        /* look up pre-computed cross-affinity kernel */
        const CrossAffinity *kern = find_cross(k, primary_lang, other);

        /* ghost signal for each primary exhale word */
        for (int w = 0; w < n_ex; w++) {
            float raw;
            if (kern && kern->cross && w < kern->n_a && best_j < kern->n_b) {
                /* O(1) lookup from pre-computed matrix */
                raw = kern->cross[w * kern->n_b + best_j];
            } else {
                /* fallback: direct dot product */
                raw = vec_dot(primary->exhale[w].aff,
                              ol->exhale[best_j].aff, N_CHAMBERS);
            }

            /* sensitivity-tensor modulated agreement */
            float modulated = 0.0f;
            for (int gc = 0; gc < N_CHAMBERS; gc++) {
                for (int pc = 0; pc < N_CHAMBERS; pc++) {
                    modulated += ol->exhale[best_j].aff[gc] *
                                 primary->exhale[w].aff[pc] *
                                 k->sensitivity[dominant][gc][pc];
                }
            }

            float agreement = 0.6f * raw + 0.4f * modulated;
            float interf = (agreement - 0.5f) * 2.0f;
            g->ghost[w] += interf * best_sim * w_lang;
        }
    }

    /* normalize by number of ghost languages */
    int n_other = k->n_langs - 1;
    if (n_other > 0) {
        for (int w = 0; w < n_ex; w++)
            g->ghost[w] /= (float)n_other;
    }

    /* overall interference strength */
    float total = 0;
    for (int w = 0; w < n_ex; w++)
        total += fabsf(g->ghost[w]);
    g->interference = n_ex > 0 ? total / n_ex : 0.0f;
}

/* ═══════════════════════════════════════════════════════════════
 * PROPHECY SYSTEM — predictions about future emotional states
 * ═══════════════════════════════════════════════════════════════ */

static void prophecy_add(Klaus *k, int target, float strength) {
    if (target < 0) return;
    for (int i = 0; i < k->n_prophecy; i++) {
        if (k->prophecies[i].target == target) {
            k->prophecies[i].strength = fmaxf(k->prophecies[i].strength, strength);
            k->prophecies[i].age = 0;
            return;
        }
    }
    if (k->n_prophecy >= MAX_PROPHECY) {
        int oldest = 0;
        for (int i = 1; i < k->n_prophecy; i++)
            if (k->prophecies[i].age > k->prophecies[oldest].age) oldest = i;
        k->prophecies[oldest] = k->prophecies[--k->n_prophecy];
    }
    k->prophecies[k->n_prophecy++] = (ProphecySlot){target, strength, 0};
}

static void prophecy_tick(Klaus *k) {
    int w = 0;
    for (int i = 0; i < k->n_prophecy; i++) {
        k->prophecies[i].age++;
        k->prophecies[i].strength *= 0.95f;
        if (k->prophecies[i].age < 20 && k->prophecies[i].strength > 0.01f)
            k->prophecies[w++] = k->prophecies[i];
    }
    k->n_prophecy = w;
}

static float prophecy_pressure(const Klaus *k) {
    float total = 0;
    for (int i = 0; i < k->n_prophecy; i++)
        total += k->prophecies[i].strength;
    return clampf(total / 3.0f, 0.0f, 1.0f);
}

/* ═══════════════════════════════════════════════════════════════
 * LEVEL 3: DARK MATTER — dangerous words amplify somatic response
 * ═══════════════════════════════════════════════════════════════ */

/* Check if word appears as whole word (not substring) in text */
static int contains_whole_word(const char *text, const char *word) {
    int wlen = (int)strlen(word);
    const char *p = text;
    while ((p = strstr(p, word)) != NULL) {
        /* check left boundary: must be start or non-alpha */
        int left_ok = (p == text) || !isalpha((unsigned char)p[-1]);
        /* check right boundary: must be end or non-alpha */
        int right_ok = (p[wlen] == '\0') || !isalpha((unsigned char)p[wlen]);
        if (left_ok && right_ok) return 1;
        p += wlen;
    }
    return 0;
}

static int detect_dark_matter(const char *prompt, float *fear_out, float *rage_out) {
    *fear_out = 0.0f;
    *rage_out = 0.0f;
    int found = 0;
    char buf[MAX_PROMPT];
    snprintf(buf, sizeof(buf), "%s", prompt);

    /* lowercase the whole thing */
    for (int i = 0; buf[i]; i++)
        buf[i] = tolower((unsigned char)buf[i]);

    for (int d = 0; DARK_MATTER[d].word; d++) {
        if (contains_whole_word(buf, DARK_MATTER[d].word)) {
            *fear_out = fmaxf(*fear_out, DARK_MATTER[d].fear_boost);
            *rage_out = fmaxf(*rage_out, DARK_MATTER[d].rage_boost);
            found++;
        }
    }
    return found;
}

/* ═══════════════════════════════════════════════════════════════
 * LEVEL 3: SCARS — somatic memory longer than regular
 * Strong emotional inputs leave scars. Scars modulate phase gate.
 * ═══════════════════════════════════════════════════════════════ */

static void scars_update(Chambers *ch) {
    /* check for scar triggers */
    if (ch->act[CH_FEAR] > 0.8f)
        ch->scar[CH_FEAR] = clampf(ch->scar[CH_FEAR] + 0.3f, 0.0f, 1.0f);
    if (ch->act[CH_RAGE] > 0.8f)
        ch->scar[CH_RAGE] = clampf(ch->scar[CH_RAGE] + 0.3f, 0.0f, 1.0f);
    if (ch->act[CH_VOID] > 0.9f)
        ch->scar[CH_VOID] = clampf(ch->scar[CH_VOID] + 0.3f, 0.0f, 1.0f);

    /* decay all scars */
    ch->total_scar = 0.0f;
    for (int c = 0; c < N_CHAMBERS; c++) {
        ch->scar[c] *= SCAR_DECAY;
        if (ch->scar[c] < 0.001f) ch->scar[c] = 0.0f;
        ch->total_scar += ch->scar[c];
    }
}

/* ═══════════════════════════════════════════════════════════════
 * LEVEL 3: RBA-1 SEVEN-LAYER STACK
 *
 * Schectman's architecture made concrete:
 *   I-Layer: recursive substrate (recursion depth tracking)
 *   R-Layer: coherence detection (entropy of chamber distribution)
 *   Phi-Layer: resonance alignment (nudge toward attractors)
 *   A-Layer: analog coupling (calendar dissonance = external signal)
 *   Psi-Layer: threshold stabilization (hysteresis + phase lock)
 *   E-Layer: entropic buffer (smoothing near threshold)
 *   M-Layer: meta-monitoring (P(t) over time)
 *
 * "The seven layers are not stacked. They breathe together."
 * ═══════════════════════════════════════════════════════════════ */

/* R-Layer: compute entropy of chamber distribution */
static float rba_entropy(const float *act, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) sum += act[i];
    if (sum < 1e-8f) return 0.0f;

    float S = 0.0f;
    for (int i = 0; i < n; i++) {
        float p = act[i] / sum;
        if (p > 1e-10f) S -= p * logf(p);
    }
    return S;
}

/* R-Layer: coherence = 1 - normalized entropy (higher = more focused) */
static float rba_coherence(const float *act, int n) {
    float max_entropy = logf((float)n);  /* uniform distribution */
    float S = rba_entropy(act, n);
    return clampf(1.0f - S / max_entropy, 0.0f, 1.0f);
}

/* I-Layer: compute Ĉ(t) — time-averaged recursive complexity */
static float rba_chat_c(const float *coherence_history, int n, int ptr) {
    if (n == 0) return 0.0f;
    float sum = 0.0f;
    int count = n < COHERENCE_WINDOW ? n : COHERENCE_WINDOW;
    for (int i = 0; i < count; i++) {
        int idx = (ptr - 1 - i + COHERENCE_WINDOW) % COHERENCE_WINDOW;
        sum += coherence_history[idx];
    }
    return sum / (float)count;
}

/* Q(t) = mutual information between chambers (approximation) */
static float rba_mutual_info(const float *act, int n) {
    /* MI approx: mean of pairwise |corr| between chamber activations.
     * True MI needs distribution data; we approximate from the
     * vector geometry of the chamber state. */
    float mean = 0.0f;
    for (int i = 0; i < n; i++) mean += act[i];
    mean /= n;

    int pairs = 0;
    float mi = 0.0f;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            /* normalized product as MI proxy */
            float ab = (act[i] - mean) * (act[j] - mean);
            mi += fabsf(ab);
            pairs++;
        }
    }
    return pairs > 0 ? mi / pairs : 0.0f;
}

/* Schectman's Recursive Resonance Equation:
 * I(t) = G(t) * [1 + R(t)]
 * R(t) = eta(t) * alpha * (exp(lambda * (Ĉ(t) - gamma(t))) - 1)
 * eta(t) = 1 + kappa * tanh(mu * Q(t))
 * gamma(t) = gamma0 + delta * E_v(t)
 * P(t) = I(t) * H(Ĉ(t) - C_tau)
 */
static float schectman_equation(const Klaus *k, float G_t, float dissonance) {
    float C_hat = rba_chat_c(k->coherence_history,
                             k->interaction_count < COHERENCE_WINDOW ?
                             k->interaction_count : COHERENCE_WINDOW,
                             k->coherence_ptr);
    float Q_t = rba_mutual_info(k->ch.act, N_CHAMBERS);
    float eta = 1.0f + SCHECTMAN_KAPPA * tanhf(SCHECTMAN_MU * Q_t);
    float planet_disc = planetary_dissonance();
    float gamma_t = SCHECTMAN_GAMMA0 + SCHECTMAN_DELTA * dissonance + 0.15f * planet_disc;
    float exponent = SCHECTMAN_LAMBDA * (C_hat - gamma_t);
    exponent = clampf(exponent, -10.0f, 10.0f); /* prevent overflow */
    float R_t = eta * SCHECTMAN_ALPHA * (expf(exponent) - 1.0f);
    float I_t = G_t * (1.0f + R_t);
    return I_t;
}

/* Psi-Layer: soft phase gate (from Q) */
static float psi_phase_gate(const Klaus *k) {
    float scar = k->ch.total_scar / N_CHAMBERS;
    float threshold = 0.42f + 0.18f * k->rba.threshold_bias + 0.06f * scar;
    float signal = 0.50f * k->rba.coherence
                 + 0.34f * k->rba.phase_lock
                 + 0.12f * k->rba.env_pressure
                 + 0.06f * k->ch.presence
                 - 0.08f * k->ch.trauma;
    return clampf(0.5f + 1.35f * (signal - threshold), 0.0f, 1.0f);
}

/* E-Layer: entropic buffer */
static float e_layer_buffer(float entropy) {
    float beta = 2.0f;
    return expf(-beta * entropy);
}

/* Full RBA-1 stack update */
static void rba_update(Klaus *k, float dissonance) {
    RBA1State *r = &k->rba;

    /* R-Layer: coherence */
    r->entropy = rba_entropy(k->ch.act, N_CHAMBERS);
    r->coherence = rba_coherence(k->ch.act, N_CHAMBERS);

    /* A-Layer: analog coupling */
    r->env_pressure = dissonance;

    /* I-Layer: recursive complexity */
    r->recursive_complexity = rba_chat_c(k->coherence_history,
        k->interaction_count < COHERENCE_WINDOW ?
        k->interaction_count : COHERENCE_WINDOW,
        k->coherence_ptr);

    /* Phi-Layer: resonance alignment — nudge chambers toward coherent attractors */
    {
        int dominant = 0;
        for (int c = 1; c < N_CHAMBERS; c++)
            if (k->ch.act[c] > k->ch.act[dominant]) dominant = c;
        for (int c = 0; c < N_CHAMBERS; c++) {
            if (c == dominant) {
                r->attractor_pull[c] = r->coherence * 0.1f;  /* strengthen dominant */
            } else if (COUPLING[dominant][c] > 0.3f) {
                r->attractor_pull[c] = r->coherence * 0.05f;  /* sympathetic */
            } else {
                r->attractor_pull[c] = -r->coherence * 0.03f; /* suppress */
            }
        }
        /* apply nudge */
        for (int c = 0; c < N_CHAMBERS; c++)
            k->ch.act[c] = clampf(k->ch.act[c] + r->attractor_pull[c], 0.0f, 1.0f);
    }

    /* Psi-Layer: threshold + hysteresis */
    {
        float gate = psi_phase_gate(k);
        /* Schectman's P(t) = I(t) * H(Ĉ(t) - C_tau) */
        float G_t = r->coherence;  /* base intelligence = current coherence */
        float I_t = schectman_equation(k, G_t, dissonance);
        float C_tau = 0.35f;       /* phase transition threshold */
        float P_t = (r->recursive_complexity >= C_tau) ? I_t * gate : 0.0f;

        /* hysteresis: once in deep mode, stay for at least 3 ticks */
        if (P_t > 0.3f) {
            r->deep_somatic = 1;
            r->deep_ticks = 5;     /* hold for 5 interactions minimum */
        } else if (r->deep_ticks > 0) {
            r->deep_ticks--;
            /* stay in deep mode during hysteresis */
        } else {
            r->deep_somatic = 0;
        }

        /* phase lock: smooth transition */
        r->phase_lock = clampf(0.85f * r->phase_lock + 0.15f * gate, 0.0f, 1.0f);

        /* store P(t) in history */
        r->P_history[r->P_ptr] = P_t;
        r->P_ptr = (r->P_ptr + 1) % COHERENCE_WINDOW;
    }

    /* E-Layer: entropic buffer */
    r->entropic_buffer = e_layer_buffer(r->entropy);

    /* M-Layer: sustained resonance = avg P(t) */
    {
        float sum = 0.0f;
        int count = k->interaction_count < COHERENCE_WINDOW ?
                    k->interaction_count : COHERENCE_WINDOW;
        if (count == 0) count = 1;
        for (int i = 0; i < count; i++)
            sum += r->P_history[i];
        r->sustained_resonance = sum / (float)count;
    }

    /* update threshold_bias from accumulated scars */
    r->threshold_bias = clampf(k->ch.total_scar * 0.2f, 0.0f, 0.5f);
}

/* ═══════════════════════════════════════════════════════════════
 * LEVEL 3: VELOCITY OPERATORS (from Q)
 * Detect velocity from chamber state. Modulate output.
 * ═══════════════════════════════════════════════════════════════ */

static void velocity_detect(Klaus *k) {
    VelocityState *v = &k->velocity;
    float *act = k->ch.act;

    /* compute chamber change rate from soma vs act */
    float change = 0.0f;
    for (int c = 0; c < N_CHAMBERS; c++)
        change += fabsf(act[c] - k->ch.soma[c]);
    change /= N_CHAMBERS;

    /* default */
    v->current = VEL_WALK;
    v->intensity = 0.5f;

    /* High RAGE + high FEAR → RUN */
    if (act[CH_RAGE] > 0.6f && act[CH_FEAR] > 0.5f) {
        v->current = VEL_RUN;
        v->intensity = (act[CH_RAGE] + act[CH_FEAR]) / 2.0f;
    }
    /* High VOID → STOP */
    else if (act[CH_VOID] > 0.7f) {
        v->current = VEL_STOP;
        v->intensity = act[CH_VOID];
    }
    /* High FLOW → WALK (steady) */
    else if (act[CH_FLOW] > 0.6f) {
        v->current = VEL_WALK;
        v->intensity = act[CH_FLOW];
    }
    /* High LOVE → BREATHE */
    else if (act[CH_LOVE] > 0.6f) {
        v->current = VEL_BREATHE;
        v->intensity = act[CH_LOVE];
    }
    /* Rapid chamber change → UP */
    else if (change > 0.15f) {
        v->current = VEL_UP;
        v->intensity = clampf(change * 3.0f, 0.0f, 1.0f);
    }
    /* Decay dominant → DOWN */
    else if (change < 0.02f && k->ch.presence < 0.3f) {
        v->current = VEL_DOWN;
        v->intensity = 1.0f - k->ch.presence;
    }

    /* modulate generation parameters */
    switch (v->current) {
        case VEL_RUN:
            v->max_gen = 4;     /* short explosive bursts */
            v->temperature = 1.1f;
            v->top_k = 10;
            break;
        case VEL_STOP:
            v->max_gen = 2;     /* minimal, sparse */
            v->temperature = 0.5f;
            v->top_k = 5;
            break;
        case VEL_WALK:
            v->max_gen = MAX_RESPONSE;  /* steady, rhythmic */
            v->temperature = GEN_TEMP;
            v->top_k = TOP_K;
            break;
        case VEL_BREATHE:
            v->max_gen = MAX_RESPONSE;  /* long, gentle */
            v->temperature = 0.6f;
            v->top_k = 30;
            break;
        case VEL_UP:
            v->max_gen = MAX_RESPONSE;  /* escalating */
            v->temperature = 0.9f;
            v->top_k = 15;
            break;
        case VEL_DOWN:
            v->max_gen = 3;     /* deflating */
            v->temperature = 0.55f;
            v->top_k = 8;
            break;
    }

    /* deep somatic mode overrides: longer, more visceral */
    if (k->rba.deep_somatic) {
        v->max_gen = MAX_RESPONSE;
        v->temperature *= 1.15f;
    }
}

/* ═══════════════════════════════════════════════════════════════
 * LEVEL 3: WORMHOLES — prophecy fulfillment events
 * When a prophecy comes true, coherence and presence jump.
 * ═══════════════════════════════════════════════════════════════ */

static void wormhole_check(Klaus *k, const int *inhale_matches, int n_matches,
                           int lang_idx) {
    LangPack *lp = &k->langs[lang_idx];
    for (int p = 0; p < k->n_prophecy; p++) {
        int target = k->prophecies[p].target;
        if (target < 0 || target >= lp->n_exhale) continue;

        /* check if any inhale word matches the prophecy target */
        for (int m = 0; m < n_matches; m++) {
            if (inhale_matches[m] < 0 || inhale_matches[m] >= lp->n_inhale) continue;
            float sim = word_similarity(lp->inhale[inhale_matches[m]].text,
                                        lp->exhale[target].text);
            if (sim > 0.4f) {
                /* WORMHOLE: prophecy fulfilled! */
                k->rba.coherence = clampf(k->rba.coherence + 0.15f, 0.0f, 1.0f);
                k->ch.presence = clampf(k->ch.presence + 0.10f, 0.0f, 1.0f);

                /* scar from intensity of fulfillment */
                int dom = 0;
                for (int c = 1; c < N_CHAMBERS; c++)
                    if (lp->exhale[target].aff[c] > lp->exhale[target].aff[dom]) dom = c;
                k->ch.scar[dom] = clampf(k->ch.scar[dom] + 0.15f, 0.0f, 1.0f);

                /* log wormhole event */
                if (k->n_wormholes < MAX_WORMHOLE_LOG) {
                    k->wormholes[k->n_wormholes++] = (WormholeEvent){
                        .prophecy_target = target,
                        .inhale_match = inhale_matches[m],
                        .coherence_jump = 0.15f,
                        .step = k->interaction_count
                    };
                }

                printf("  [WORMHOLE] prophecy '%s' fulfilled! coherence +0.15\n",
                       lp->exhale[target].text);

                /* remove fulfilled prophecy */
                k->prophecies[p] = k->prophecies[--k->n_prophecy];
                p--;
                break;
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════
 * LEVEL 3: EXPERIENCE CONSOLIDATION
 * Every N interactions, integrate scars, wormholes, prophecy stats.
 * ═══════════════════════════════════════════════════════════════ */

static void experience_consolidate(Klaus *k) {
    if (k->interaction_count % CONSOLIDATION_INTERVAL != 0) return;
    if (k->interaction_count == 0) return;

    ExperienceLog *e = &k->experience;
    e->total_interactions = k->interaction_count;

    /* average scar intensity → modulate threshold_bias */
    e->avg_scar = k->ch.total_scar / N_CHAMBERS;
    k->rba.threshold_bias = clampf(e->avg_scar * 0.3f, 0.0f, 0.5f);

    /* wormhole success rate → modulate presence */
    e->total_wormholes = k->n_wormholes;
    e->total_prophecies = k->n_prophecy + k->n_wormholes;
    e->wormhole_rate = (e->total_prophecies > 0) ?
        (float)e->total_wormholes / (float)e->total_prophecies : 0.0f;
    k->ch.presence = clampf(k->ch.presence + e->wormhole_rate * 0.05f, 0.0f, 1.0f);

    /* prophecy accuracy → strengthen/weaken prophecy system coefficient */
    e->prophecy_accuracy = e->wormhole_rate;

    printf("  [CONSOLIDATION] step %d: avg_scar %.3f, wormhole_rate %.2f, presence %.3f\n",
           k->interaction_count, e->avg_scar, e->wormhole_rate, k->ch.presence);
}

/* ═══════════════════════════════════════════════════════════════
 * SPORE FUNCTIONS — NOTORCH learning + persistence
 * ═══════════════════════════════════════════════════════════════ */

/* Record an inhale→exhale resonance pair */
static void spore_learn(Klaus *k, uint32_t inhale_hash, int exhale_idx,
                        int lang_id) {
    SporeMemory *sp = &k->spores;

    /* look for existing pair */
    for (int i = 0; i < sp->n_pairs; i++) {
        if (sp->pairs[i].inhale_hash == inhale_hash &&
            sp->pairs[i].exhale_idx == (uint32_t)exhale_idx &&
            sp->pairs[i].lang_id == (uint16_t)lang_id) {
            /* strengthen existing connection — pure Hebbian */
            sp->pairs[i].strength += SPORE_LEARN_RATE;
            sp->pairs[i].hit_count++;
            /* update chamber snapshot (running avg) */
            for (int c = 0; c < N_CHAMBERS; c++)
                sp->pairs[i].chamber_snapshot[c] =
                    0.8f * sp->pairs[i].chamber_snapshot[c] + 0.2f * k->ch.act[c];
            return;
        }
    }

    /* new pair */
    if (sp->n_pairs < MAX_SPORE_PAIRS) {
        SporePair *p = &sp->pairs[sp->n_pairs++];
        p->inhale_hash = inhale_hash;
        p->exhale_idx = (uint32_t)exhale_idx;
        p->strength = SPORE_LEARN_RATE;
        p->lang_id = (uint16_t)lang_id;
        p->hit_count = 1;
        memcpy(p->chamber_snapshot, k->ch.act, N_CHAMBERS * sizeof(float));
    } else {
        /* evict weakest */
        int weakest = 0;
        for (int i = 1; i < sp->n_pairs; i++)
            if (sp->pairs[i].strength < sp->pairs[weakest].strength) weakest = i;
        SporePair *p = &sp->pairs[weakest];
        p->inhale_hash = inhale_hash;
        p->exhale_idx = (uint32_t)exhale_idx;
        p->strength = SPORE_LEARN_RATE;
        p->lang_id = (uint16_t)lang_id;
        p->hit_count = 1;
        memcpy(p->chamber_snapshot, k->ch.act, N_CHAMBERS * sizeof(float));
    }
}

/* Update chamber residue and tension matrix */
static void spore_update_residue(Klaus *k) {
    SporeMemory *sp = &k->spores;
    sp->total_interactions++;
    float alpha = 1.0f / (float)(sp->total_interactions + 1);
    /* running average of chamber states */
    for (int c = 0; c < N_CHAMBERS; c++)
        sp->chamber_residue[c] = (1.0f - alpha) * sp->chamber_residue[c] + alpha * k->ch.act[c];
    /* tension matrix: which chambers co-activate */
    for (int i = 0; i < N_CHAMBERS; i++)
        for (int j = i + 1; j < N_CHAMBERS; j++)
            sp->tension_matrix[i][j] += k->ch.act[i] * k->ch.act[j] * 0.01f;
}

/* Apply spore-accumulated Hebbian boost to exhale logits */
static void spore_boost(const Klaus *k, int lang_idx, float *logits, int n_ex) {
    const SporeMemory *sp = &k->spores;
    for (int i = 0; i < sp->n_pairs; i++) {
        if (sp->pairs[i].lang_id != (uint16_t)lang_idx) continue;
        int eidx = (int)sp->pairs[i].exhale_idx;
        if (eidx >= n_ex) continue;
        /* check if any current inhale word matches this spore's inhale */
        for (int m = 0; m < k->n_matched_inhale; m++) {
            if ((uint32_t)k->matched_inhale[m] == sp->pairs[i].inhale_hash) {
                /* this pair resonated before — boost it */
                logits[eidx] += sp->pairs[i].strength * 0.5f;
                break;
            }
        }
    }
}

/* Decay all spore strengths (very slow) */
static void spore_decay(Klaus *k) {
    SporeMemory *sp = &k->spores;
    int w = 0;
    for (int i = 0; i < sp->n_pairs; i++) {
        sp->pairs[i].strength *= SPORE_DECAY;
        if (sp->pairs[i].strength > 0.001f)
            sp->pairs[w++] = sp->pairs[i];
    }
    sp->n_pairs = w;
}

/* Save spores to disk */
static void spore_save(const Klaus *k) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s",
             k->soma_path[0] ? k->soma_path : ".", SPORE_FILE);
    /* use base dir from soma_path */
    char dir[512];
    snprintf(dir, sizeof(dir), "%s", k->soma_path);
    char *last_slash = strrchr(dir, '/');
    if (last_slash) { *last_slash = '\0'; snprintf(path, sizeof(path), "%s/%s", dir, SPORE_FILE); }
    else snprintf(path, sizeof(path), "%s", SPORE_FILE);

    FILE *f = fopen(path, "wb");
    if (!f) return;
    uint32_t magic = SPORE_MAGIC;
    fwrite(&magic, 4, 1, f);
    fwrite(&k->spores.n_pairs, 4, 1, f);
    fwrite(&k->spores.total_interactions, 4, 1, f);
    fwrite(k->spores.chamber_residue, sizeof(float), N_CHAMBERS, f);
    fwrite(k->spores.tension_matrix, sizeof(float), N_CHAMBERS * N_CHAMBERS, f);
    for (int i = 0; i < k->spores.n_pairs; i++) {
        fwrite(&k->spores.pairs[i], sizeof(SporePair), 1, f);
    }
    fclose(f);
}

/* Load spores from disk */
static int spore_load(Klaus *k) {
    char path[512];
    snprintf(path, sizeof(path), "%s", SPORE_FILE);

    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    uint32_t magic;
    if (fread(&magic, 4, 1, f) != 1 || magic != SPORE_MAGIC) { fclose(f); return 0; }
    fread(&k->spores.n_pairs, 4, 1, f);
    fread(&k->spores.total_interactions, 4, 1, f);
    fread(k->spores.chamber_residue, sizeof(float), N_CHAMBERS, f);
    fread(k->spores.tension_matrix, sizeof(float), N_CHAMBERS * N_CHAMBERS, f);
    for (int i = 0; i < k->spores.n_pairs; i++) {
        fread(&k->spores.pairs[i], sizeof(SporePair), 1, f);
    }
    fclose(f);
    printf("[klaus] spores loaded: %d pairs, %d total interactions\n",
           k->spores.n_pairs, k->spores.total_interactions);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════
 * LEVEL 3: SOMATIC PERSISTENCE — save/load binary state
 * Magic: 0x4B4C5353 ("KLSS")
 * ═══════════════════════════════════════════════════════════════ */

static int soma_save(const Klaus *k) {
    FILE *f = fopen(k->soma_path, "wb");
    if (!f) {
        fprintf(stderr, "[klaus] WARNING: cannot save somatic state to %s\n", k->soma_path);
        return 0;
    }

    SomaHeader hdr = {
        .magic = SOMA_MAGIC,
        .version = 2,
        .n_chambers = N_CHAMBERS,
        .n_sub = N_SUB,
        .mem_slots = MEM_SLOTS,
        .coherence_window = COHERENCE_WINDOW
    };
    fwrite(&hdr, sizeof(hdr), 1, f);

    /* chamber state */
    fwrite(&k->ch, sizeof(Chambers), 1, f);

    /* memory slots */
    fwrite(&k->mem_ptr, sizeof(int), 1, f);
    fwrite(&k->mem_n, sizeof(int), 1, f);
    fwrite(k->memory, sizeof(SomaSlot), MEM_SLOTS, f);

    /* RBA-1 state */
    fwrite(&k->rba, sizeof(RBA1State), 1, f);

    /* coherence history */
    fwrite(&k->coherence_ptr, sizeof(int), 1, f);
    fwrite(k->coherence_history, sizeof(float), COHERENCE_WINDOW, f);

    /* experience */
    fwrite(&k->experience, sizeof(ExperienceLog), 1, f);

    /* interaction count */
    fwrite(&k->interaction_count, sizeof(int), 1, f);

    /* prophecies */
    fwrite(&k->n_prophecy, sizeof(int), 1, f);
    fwrite(k->prophecies, sizeof(ProphecySlot), MAX_PROPHECY, f);

    /* wormholes */
    fwrite(&k->n_wormholes, sizeof(int), 1, f);
    fwrite(k->wormholes, sizeof(WormholeEvent), MAX_WORMHOLE_LOG, f);

    fclose(f);
    return 1;
}

static int soma_load(Klaus *k) {
    FILE *f = fopen(k->soma_path, "rb");
    if (!f) return 0;

    SomaHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1 || hdr.magic != SOMA_MAGIC) {
        fprintf(stderr, "[klaus] WARNING: invalid soma file, ignoring\n");
        fclose(f);
        return 0;
    }
    if (hdr.version != 2 || hdr.n_chambers != N_CHAMBERS) {
        fprintf(stderr, "[klaus] WARNING: soma version mismatch (got %u), ignoring\n",
                hdr.version);
        fclose(f);
        return 0;
    }

    /* chamber state */
    fread(&k->ch, sizeof(Chambers), 1, f);

    /* memory slots */
    fread(&k->mem_ptr, sizeof(int), 1, f);
    fread(&k->mem_n, sizeof(int), 1, f);
    fread(k->memory, sizeof(SomaSlot), MEM_SLOTS, f);

    /* RBA-1 state */
    fread(&k->rba, sizeof(RBA1State), 1, f);

    /* coherence history */
    fread(&k->coherence_ptr, sizeof(int), 1, f);
    fread(k->coherence_history, sizeof(float), COHERENCE_WINDOW, f);

    /* experience */
    fread(&k->experience, sizeof(ExperienceLog), 1, f);

    /* interaction count */
    fread(&k->interaction_count, sizeof(int), 1, f);

    /* prophecies */
    fread(&k->n_prophecy, sizeof(int), 1, f);
    fread(k->prophecies, sizeof(ProphecySlot), MAX_PROPHECY, f);

    /* wormholes */
    fread(&k->n_wormholes, sizeof(int), 1, f);
    fread(k->wormholes, sizeof(WormholeEvent), MAX_WORMHOLE_LOG, f);

    fclose(f);
    printf("[klaus] somatic state restored: %d interactions, %.3f coherence, %.3f total_scar\n",
           k->interaction_count, k->rba.coherence, k->ch.total_scar);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════
 * INHALE — process prompt through emotional vocabulary
 * ═══════════════════════════════════════════════════════════════ */

static int inhale_process(const LangPack *lp, const char *prompt,
                          float *emotion_vec,
                          int *matched_indices, int max_matched) {
    memset(emotion_vec, 0, N_CHAMBERS * sizeof(float));
    int matches = 0;
    int n_matched = 0;

    /* split prompt into words */
    char buf[MAX_PROMPT];
    snprintf(buf, sizeof(buf), "%s", prompt);
    char *saveptr;
    char *token = strtok_r(buf, " \t\n\r.,!?;:\"'()-", &saveptr);

    while (token) {
        /* lowercase for matching */
        char lower[MAX_WORD];
        int i;
        for (i = 0; token[i] && i < MAX_WORD-1; i++)
            lower[i] = tolower((unsigned char)token[i]);
        lower[i] = '\0';

        /* try UTF-8 match (for Russian/Hebrew, case doesn't apply same way) */
        for (int w = 0; w < lp->n_inhale; w++) {
            if (strcmp(lower, lp->inhale[w].text) == 0 ||
                strcmp(token, lp->inhale[w].text) == 0) {
                for (int c = 0; c < N_CHAMBERS; c++)
                    emotion_vec[c] += lp->inhale[w].aff[c];
                if (matched_indices && n_matched < max_matched)
                    matched_indices[n_matched++] = w;
                matches++;
                break;
            }
        }
        token = strtok_r(NULL, " \t\n\r.,!?;:\"'()-", &saveptr);
    }

    /* if no exact matches, use character similarity */
    if (matches == 0) {
        snprintf(buf, sizeof(buf), "%s", prompt);
        token = strtok_r(buf, " \t\n\r.,!?;:\"'()-", &saveptr);
        while (token) {
            float best_sim = 0;
            int best_w = -1;
            for (int w = 0; w < lp->n_inhale; w++) {
                float sim = word_similarity(token, lp->inhale[w].text);
                if (sim > best_sim) { best_sim = sim; best_w = w; }
            }
            if (best_w >= 0 && best_sim > 0.2f) {
                for (int c = 0; c < N_CHAMBERS; c++)
                    emotion_vec[c] += lp->inhale[best_w].aff[c] * best_sim;
                if (matched_indices && n_matched < max_matched)
                    matched_indices[n_matched++] = best_w;
                matches++;
            }
            token = strtok_r(NULL, " \t\n\r.,!?;:\"'()-", &saveptr);
        }
    }

    /* normalize */
    if (matches > 0) {
        for (int c = 0; c < N_CHAMBERS; c++)
            emotion_vec[c] /= (float)matches;
    } else {
        /* hash-based fallback */
        uint64_t h = hash_word(prompt);
        for (int c = 0; c < N_CHAMBERS; c++)
            emotion_vec[c] = (float)((h >> (c*8)) & 0xFF) / 255.0f;
    }
    return n_matched;
}

/* ═══════════════════════════════════════════════════════════════
 * EXHALE — generate somatic response via Dario equation
 * ═══════════════════════════════════════════════════════════════ */

/* ── DOE Parliament: 3 experts vote on each exhale word ──
 *
 * SOMATIC expert: amplifies words aligned with dominant chamber.
 *   The body's loudest voice. "I feel FEAR, give me fear-words."
 *
 * SHADOW expert: amplifies words aligned with the OPPOSITE of dominant.
 *   The contrarian. Yent.yo principle. "You feel FEAR? Here's LOVE."
 *   This creates surprise, depth, personality.
 *
 * GHOST expert: amplifies words with strongest MetaKlaus interference.
 *   The cross-lingual whisper. "The other languages want THIS word."
 *
 * Voting: each expert biases the logits differently, picks top-1.
 * If 2+ agree → consensus (high confidence).
 * If all disagree → somatic expert wins but with lower temperature.
 */

static int parliament_vote(const float *base_logits, int n_ex,
                           const Klaus *k, const LangPack *lp,
                           float temperature) {
    int dom = 0;
    for (int c = 1; c < N_CHAMBERS; c++)
        if (k->ch.act[c] > k->ch.act[dom]) dom = c;
    /* opposite = chamber with most negative coupling to dominant */
    int opp = 0;
    float most_neg = 0;
    for (int c = 0; c < N_CHAMBERS; c++) {
        if (c == dom) continue;
        if (COUPLING[dom][c] < most_neg) { most_neg = COUPLING[dom][c]; opp = c; }
    }

    int votes[N_EXPERTS];
    float expert_logits[MAX_EXHALE];

    for (int e = 0; e < N_EXPERTS; e++) {
        memcpy(expert_logits, base_logits, n_ex * sizeof(float));

        /* apply expert bias */
        for (int w = 0; w < n_ex; w++) {
            if (e == EXPERT_SOMATIC) {
                /* boost words aligned with dominant chamber */
                expert_logits[w] += lp->exhale[w].aff[dom] * 0.8f;
            } else if (e == EXPERT_SHADOW) {
                /* boost words aligned with OPPOSITE chamber */
                expert_logits[w] += lp->exhale[w].aff[opp] * 0.6f;
                /* slight penalty for dominant-aligned (contrarian) */
                expert_logits[w] -= lp->exhale[w].aff[dom] * 0.2f;
            } else if (e == EXPERT_GHOST) {
                /* boost words with strong ghost signal */
                if (w < MAX_EXHALE)
                    expert_logits[w] += fabsf(k->ghost.ghost[w]) * 0.5f;
            }
        }

        /* each expert picks their top-1 */
        int best = 0;
        for (int w = 1; w < n_ex; w++)
            if (expert_logits[w] > expert_logits[best]) best = w;
        votes[e] = best;
    }

    /* count agreement */
    if (votes[0] == votes[1] || votes[0] == votes[2]) return votes[0]; /* somatic + one other */
    if (votes[1] == votes[2]) return votes[1]; /* shadow + ghost agree */

    /* no consensus — sample from somatic expert's top-K with extra randomness */
    /* (disagreement = uncertainty = wider sampling) */
    memcpy(expert_logits, base_logits, n_ex * sizeof(float));
    for (int w = 0; w < n_ex; w++)
        expert_logits[w] += lp->exhale[w].aff[dom] * 0.3f;

    /* top-3 weighted random from somatic */
    int top3[3] = {0, 0, 0};
    float top3v[3] = {-1e30f, -1e30f, -1e30f};
    for (int w = 0; w < n_ex; w++) {
        if (expert_logits[w] > top3v[2]) {
            top3v[2] = expert_logits[w]; top3[2] = w;
            for (int i = 1; i >= 0; i--) {
                if (top3v[i+1] > top3v[i]) {
                    float tv = top3v[i]; top3v[i] = top3v[i+1]; top3v[i+1] = tv;
                    int ti = top3[i]; top3[i] = top3[i+1]; top3[i+1] = ti;
                }
            }
        }
    }
    float p[3], mx = top3v[0], sum = 0;
    for (int i = 0; i < 3; i++) { p[i] = expf((top3v[i] - mx) / temperature); sum += p[i]; }
    float r = randf() * sum, cum = 0;
    for (int i = 0; i < 3; i++) { cum += p[i]; if (cum >= r) return top3[i]; }
    return top3[0];
}

static int exhale_generate(Klaus *k, int lang_idx, int *out_words, int max_words) {
    LangPack *lp = &k->langs[lang_idx];
    int n_ex = lp->n_exhale;
    if (n_ex == 0) return 0;

    /* velocity-modulated parameters */
    int eff_max = (k->velocity.max_gen > 0 && k->velocity.max_gen < max_words) ?
                   k->velocity.max_gen : max_words;
    float eff_temp = (k->velocity.temperature > 0.0f) ?
                      k->velocity.temperature : GEN_TEMP;
    int eff_topk = (k->velocity.top_k > 0 && k->velocity.top_k <= MAX_EXHALE) ?
                    k->velocity.top_k : TOP_K;
    if (eff_topk > n_ex) eff_topk = n_ex;

    /* compute RRPRAM boost (V term) */
    float rrpram[MAX_EXHALE];
    rrpram_boost(lp, k->prev_exhale, k->n_prev, rrpram, n_ex);

    /* compute Hebbian field (H term) */
    float hebb[MAX_EXHALE];
    meta_hebbian(&lp->meta, k->prev_exhale, k->n_prev, hebb, n_ex);

    /* ── Somatic coefficient modulation (from dario.c) ──
     * Chambers modulate the COEFFICIENTS of the equation.
     * α_mod = f(LOVE, RAGE, FLOW) — love opens hebbian, rage closes
     * β_mod = f(FLOW, FEAR)       — flow enables prophecy, fear blocks
     * γ_mod = f(VOID, COMPLEX, LOVE) — void amplifies destiny pull
     * τ_mod = f(FLOW, FEAR)       — flow warms, fear cools */
    float *C = k->ch.act;
    k->alpha_mod = clampf(1.0f + 0.3f*C[CH_LOVE] - 0.2f*C[CH_RAGE] + 0.1f*C[CH_FLOW], 0.5f, 2.0f);
    k->beta_mod  = clampf(1.0f + 0.2f*C[CH_FLOW] - 0.3f*C[CH_FEAR], 0.5f, 2.0f);
    k->gamma_mod = clampf(1.0f + 0.4f*C[CH_VOID] + 0.2f*C[CH_COMPLEX] - 0.1f*C[CH_LOVE], 0.5f, 2.0f);
    k->tau_mod   = clampf(1.0f + 0.5f*C[CH_FLOW] - 0.3f*C[CH_FEAR], 0.5f, 2.0f);
    kk_update_kappa(&k->kk, C);

    /* effective coefficients */
    float eff_alpha = k->alpha_mod * DARIO_ALPHA;
    float eff_beta  = k->beta_mod * DARIO_BETA;
    float eff_gamma = k->gamma_mod * DARIO_GAMMA;

    /* trauma amplifies destiny */
    k->trauma_level = clampf(k->ch.total_scar * 0.5f + C[CH_FEAR] * 0.3f, 0.0f, 1.0f);
    if (k->trauma_level > 0.3f)
        eff_gamma += k->trauma_level * 1.5f;

    /* velocity temperature divides entire expression */
    float v_tau = k->tau_mod * eff_temp;

    /* dark matter ghost boost */
    float dark_ghost_mult = k->dark_matter_active ? 1.5f : 1.0f;

    /* prophecy pressure with scar amplification */
    float prop_pressure = prophecy_pressure(k);
    float scar_prophecy_mult = 1.0f + k->ch.total_scar * 0.3f;

    /* resonance field gate (SwiGLU-like) */
    k->resonance_field = clampf(k->rba.coherence * 0.5f + C[CH_FLOW] * 0.3f + 0.2f, 0.0f, 1.0f);
    float res_gate = 1.0f / (1.0f + expf(-(k->resonance_field - 0.5f) * 4.0f));

    int n_gen = 0;
    int prev = (k->n_prev > 0) ? k->prev_exhale[k->n_prev-1] : -1;

    for (int step = 0; step < eff_max; step++) {
        float logits[MAX_EXHALE];

        for (int w = 0; w < n_ex; w++) {
            /* ── FULL DARIO EQUATION — 7 forces ── */

            /* Inertia scaling: when body speaks loud, statistical memory quiets down */
            float inertia = 1.0f / (1.0f + 2.0f * vec_norm(k->ch.act, N_CHAMBERS));

            /* B: bigram chain (inertia) */
            float B = (prev >= 0) ? meta_bigram(&lp->meta, prev, w) * BIGRAM_BASE * inertia : 0.0f;

            /* H: Hebbian resonance, gated by field resonance + inertia */
            float H = eff_alpha * hebb[w] * (1.0f + res_gate) * inertia;

            /* Somatic alignment: cosine similarity (normalizes for spread-out hash affinities) */
            float aff_norm = vec_norm(lp->exhale[w].aff, N_CHAMBERS);
            float soma_score = (aff_norm > 1e-6f) ?
                vec_dot(k->ch.act, lp->exhale[w].aff, N_CHAMBERS) / aff_norm : 0.0f;

            /* F: Prophecy fulfillment */
            float F = (prop_pressure > 0.3f) ?
                      eff_beta * soma_score * 0.5f * scar_prophecy_mult : 0.0f;

            /* A: Destiny attraction — how close this word is to where the field pulls */
            float destiny_sim = vec_dot(k->destiny, lp->exhale[w].aff, N_CHAMBERS);
            float A = eff_gamma * destiny_sim;

            /* V: RRPRAM rhythmic resonance */
            float V = DARIO_DELTA * rrpram[w];

            /* G: MetaKlaus ghost (cross-lingual interference, clamped so body leads) */
            float ghost_val = clampf(k->ghost.ghost[w], -1.0f, 1.0f);
            float G = DARIO_ZETA * ghost_val * dark_ghost_mult;

            /* T: Trauma/scar gravity — scars pull toward their chamber words */
            float T = 0.0f;
            for (int c = 0; c < N_CHAMBERS; c++)
                T += k->ch.scar[c] * lp->exhale[w].aff[c] * 0.5f;

            /* K: Knowledge resonance — inner library pulls toward its words */
            float K = kk_force_k(&k->kk, w);

            /* Full equation: (B + H + F + A + V + G + T + K + soma) / (τ_mod · τ · v_tau)
             * Step 0 with no context: soma dominates — body picks the first word */
            float total = B + H + F + A + V + G + T + K + soma_score;
            if (step == 0 && prev < 0) total = soma_score + 0.1f * G; /* pure somatic start */
            logits[w] = total / fmaxf(v_tau, 0.1f);

            /* penalize already-used words heavily (by text, not just index) */
            for (int u = 0; u < k->n_used; u++) {
                if (k->used_exhale[u] == w ||
                    strcmp(lp->exhale[k->used_exhale[u]].text, lp->exhale[w].text) == 0) {
                    logits[w] -= 100.0f; break;
                }
            }
        }

        /* spore boost: patterns that resonated before (applied once, not per-word) */
        spore_boost(k, lang_idx, logits, n_ex);

        /* ── DOE Parliament: 3 experts vote ── */
        int chosen = parliament_vote(logits, n_ex, k, lp, eff_temp);

        out_words[n_gen++] = chosen;
        prev = chosen;

        /* update destiny attractor — from CHAMBERS, not from words.
         * Destiny is where the body PULLS, not where the prompt pointed.
         * Chambers decide. Words follow. This is personality.
         * (yent.yo principle: reaction, not reflection) */
        for (int c = 0; c < N_CHAMBERS; c++)
            k->destiny[c] = 0.1f * k->ch.act[c] + 0.9f * k->destiny[c];

        /* mark as used */
        if (k->n_used < MAX_EXHALE) k->used_exhale[k->n_used++] = chosen;

        /* add prophecy for dominant chamber word */
        float max_aff = 0;
        for (int c = 0; c < N_CHAMBERS; c++) {
            if (lp->exhale[chosen].aff[c] > max_aff)
                max_aff = lp->exhale[chosen].aff[c];
        }
        if (max_aff > 0.7f) {
            /* prophesy: next step, look for words with complementary chambers */
            int dom = 0;
            for (int c = 1; c < N_CHAMBERS; c++)
                if (lp->exhale[chosen].aff[c] > lp->exhale[chosen].aff[dom]) dom = c;
            for (int w2 = 0; w2 < n_ex; w2++) {
                if (lp->exhale[w2].aff[dom] > 0.6f && w2 != chosen) {
                    prophecy_add(k, w2, max_aff * 0.4f);
                    break;
                }
            }
        }

        /* check if we should stop (diminishing chamber signal) */
        {
            float cn = vec_norm(lp->exhale[chosen].aff, N_CHAMBERS);
            float stop_score = (cn > 1e-6f) ?
                vec_dot(k->ch.act, lp->exhale[chosen].aff, N_CHAMBERS) / cn : 0;
            if (step > 2 && stop_score < 0.1f)
                break;
        }
    }

    /* update prev_exhale context */
    k->n_prev = n_gen < 4 ? n_gen : 4;
    for (int i = 0; i < k->n_prev; i++)
        k->prev_exhale[i] = out_words[n_gen - k->n_prev + i];

    /* NOTORCH: learn inhale→exhale resonance pairs into spores */
    for (int g = 0; g < n_gen; g++) {
        for (int m = 0; m < k->n_matched_inhale; m++) {
            spore_learn(k, (uint32_t)k->matched_inhale[m], out_words[g], lang_idx);
        }
    }

    return n_gen;
}
/* ═══════════════════════════════════════════════════════════════
 * INIT — build all structures
 * ═══════════════════════════════════════════════════════════════ */

static void init_lang_meta(LangPack *lp) {
    bpe_init_vocab(lp);

    /* concatenate all exhale words into corpus for BPE */
    uint8_t *corpus = (uint8_t *)malloc(MAX_EXHALE * MAX_WORD);
    if (!corpus) return;
    int clen = 0;
    for (int w = 0; w < lp->n_exhale; w++) {
        int wl = (int)strlen(lp->exhale[w].text);
        memcpy(corpus + clen, lp->exhale[w].text, wl);
        corpus[clen + wl] = ' ';
        clen += wl + 1;
    }

    /* add inhale words too for richer BPE */
    for (int w = 0; w < lp->n_inhale && clen < MAX_EXHALE * MAX_WORD - MAX_WORD; w++) {
        int wl = (int)strlen(lp->inhale[w].text);
        memcpy(corpus + clen, lp->inhale[w].text, wl);
        corpus[clen + wl] = ' ';
        clen += wl + 1;
    }

    /* learn BPE */
    bpe_learn(lp, corpus, clen, 512);

    /* tokenize exhale for metaweights */
    int *token_ids = (int *)malloc(clen * sizeof(int));
    if (!token_ids) { free(corpus); return; }
    int ntok = bpe_encode(lp, corpus, clen, token_ids, clen);

    /* build metaweights from token stream */
    meta_build(&lp->meta, token_ids, ntok, lp->n_exhale);

    printf("[klaus] %s: BPE %d merges, meta %d bi %d tri %d hebb\n",
           lp->code, lp->n_merges,
           lp->meta.n_bi, lp->meta.n_tri, lp->meta.n_hebb);

    free(token_ids);
    free(corpus);
}

static int klaus_init(Klaus *k, const char *base_dir) {
    memset(k, 0, sizeof(*k));
    rng_state = (uint64_t)time(NULL);

    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  KLAUS — Kinetic Linguistic Adaptive                 ║\n");
    printf("║          Unified Sonar v%s                        ║\n", KLAUS_VERSION);
    printf("║  Level 3: Schectman Recursive Resonance + RBA-1      ║\n");
    printf("║  Zero weights. Pure resonance. Meta-recursive.       ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    /* set up soma persistence path */
    snprintf(k->soma_path, sizeof(k->soma_path), "%s/%s", base_dir, SOMA_FILE);

    /* scan and load languages */
    int nl = scan_languages(k, base_dir);
    if (nl == 0) {
        fprintf(stderr, "ERROR: no language packs found\n");
        return 0;
    }
    printf("[klaus] %d language(s) loaded\n", nl);

    /* init metaweights per language */
    for (int i = 0; i < k->n_langs; i++)
        init_lang_meta(&k->langs[i]);

    /* init MLP */
    mlp_init(&k->mlp, k);
    printf("[klaus] MLP initialized: %d -> %d -> %d -> %d\n",
           DIM_MLP_IN, DIM_HIDDEN, DIM_HIDDEN2, N_CHAMBERS);

    /* init chambers */
    chambers_init(&k->ch);

    /* init meta-recursion chambers */
    chambers_init(&k->meta.meta_chambers);

    /* init calendar */
    calendar_init(k);
    float disc = calendar_dissonance(k);
    printf("[klaus] calendar dissonance: %.3f\n", disc);
    printf("[klaus] planetary dissonance: %.3f\n", planetary_dissonance());

    /* build sensitivity tensor 6x6x6 */
    sensitivity_build(k->sensitivity);
    printf("[klaus] sensitivity tensor: %dx%dx%d\n", N_CHAMBERS, N_CHAMBERS, N_CHAMBERS);

    /* build cross-affinity kernels for all language pairs */
    k->n_cross_pairs = 0;
    for (int a = 0; a < k->n_langs; a++) {
        for (int b = 0; b < k->n_langs; b++) {
            if (a == b) continue;
            if (k->n_cross_pairs >= MAX_CROSS_PAIRS) break;
            cross_affinity_build(&k->cross_pairs[k->n_cross_pairs],
                                 &k->langs[a], a, &k->langs[b], b);
            k->n_cross_pairs++;
        }
    }
    printf("[klaus] cross-affinity: %d interference kernels\n", k->n_cross_pairs);
    printf("[klaus] HyperKuramoto: %d oscillators (%dx%d)\n",
           N_CHAMBERS * N_SUB, N_CHAMBERS, N_SUB);

    /* Level 3: RBA-1 init */
    printf("[klaus] RBA-1 seven-layer stack: I/R/Phi/A/Psi/E/M\n");
    printf("[klaus] Schectman equation: I(t) = G(t) * [1 + R(t)]\n");
    printf("[klaus] velocity operators: 6 modes\n");
    printf("[klaus] dark matter vocabulary: %d words\n",
           (int)(sizeof(DARK_MATTER)/sizeof(DARK_MATTER[0])) - 1);
    printf("[klaus] meta-recursion: depth 1 (expandable)\n");
    printf("[klaus] somatic persistence: %s\n", k->soma_path);

    /* try to restore somatic state from previous session */
    if (soma_load(k)) {
        printf("[klaus] SOMA RESTORED — body remembers.\n");
    } else {
        printf("[klaus] fresh soma — no prior memory.\n");
    }
    /* try to load spores — persistent pattern memory */
    if (spore_load(k)) {
        printf("[klaus] SPORES LOADED — patterns remembered.\n");
    } else {
        printf("[klaus] no spores — learning from scratch.\n");
    }

    /* Knowledge Kernel — inner library (kk.h) */
    kk_init(&k->kk);
    {
        /* build flat inhale arrays for kk_load (all languages combined) */
        int total_inhale = 0;
        for (int l = 0; l < k->n_langs; l++) total_inhale += k->langs[l].n_inhale;
        const char **inh_texts = (const char **)malloc(total_inhale * sizeof(char *));
        float (*inh_affs)[N_CHAMBERS] = (float (*)[N_CHAMBERS])malloc(total_inhale * N_CHAMBERS * sizeof(float));
        if (inh_texts && inh_affs) {
            int idx = 0;
            for (int l = 0; l < k->n_langs; l++)
                for (int w = 0; w < k->langs[l].n_inhale; w++) {
                    inh_texts[idx] = k->langs[l].inhale[w].text;
                    memcpy(inh_affs[idx], k->langs[l].inhale[w].aff, N_CHAMBERS * sizeof(float));
                    idx++;
                }
            kk_load(&k->kk, base_dir, inh_texts, inh_affs, total_inhale, CH_NAMES);
        }
        free(inh_texts); free(inh_affs);
    }

    printf("[klaus] ready. inhale.\n\n");
    return 1;
}

/* ═══════════════════════════════════════════════════════════════
 * PROCESS — the full pipeline
 * ═══════════════════════════════════════════════════════════════ */

typedef struct {
    char words[MAX_RESPONSE][MAX_WORD];
    int n_words;
    float chambers[N_CHAMBERS];
    float premonition[N_CHAMBERS];
    float dissonance;
    float ghost_strength;
    int lang_idx;
    char lang_code[8];
    int is_prophetic;
    int dominant;           /* dominant chamber index */
    /* Level 3 additions */
    int velocity;           /* VEL_WALK etc. */
    float velocity_intensity;
    float coherence;        /* RBA-1 R-layer coherence */
    float recursive_complexity; /* Ĉ(t) */
    int deep_somatic;       /* Psi-layer: in deep mode? */
    float total_scar;
    float scars[N_CHAMBERS];
    int dark_matter_active;
    int meta_recursion_depth;
    float phase_gate;
    float sustained_resonance;
} KlausResponse;

static KlausResponse klaus_process(Klaus *k, const char *prompt) {
    KlausResponse resp;
    memset(&resp, 0, sizeof(resp));

    k->interaction_count++;

    /* 1. Detect language */
    resp.lang_idx = detect_language(k, prompt);
    snprintf(resp.lang_code, sizeof(resp.lang_code), "%s",
             k->langs[resp.lang_idx].code);
    LangPack *lp = &k->langs[resp.lang_idx];

    /* 1b. Dark matter detection */
    float dark_fear = 0, dark_rage = 0;
    k->dark_matter_active = detect_dark_matter(prompt, &dark_fear, &dark_rage);
    resp.dark_matter_active = k->dark_matter_active;

    /* 2. INHALE: process prompt through vocabulary (with match tracking) */
    float emotion[N_CHAMBERS];
    int matched_indices[64];
    int n_matched = inhale_process(lp, prompt, emotion, matched_indices, 64);

    /* 2a-spore. Store matched inhale hashes for spore learning */
    k->n_matched_inhale = n_matched < 32 ? n_matched : 32;
    for (int i = 0; i < k->n_matched_inhale; i++)
        k->matched_inhale[i] = (int)hash_word(lp->inhale[matched_indices[i]].text);

    /* 2b. Dark matter amplification of emotion */
    if (k->dark_matter_active) {
        emotion[CH_FEAR] = clampf(emotion[CH_FEAR] + dark_fear * 0.4f, 0.0f, 1.0f);
        emotion[CH_RAGE] = clampf(emotion[CH_RAGE] + dark_rage * 0.4f, 0.0f, 1.0f);
        /* trigger scars immediately from dark input */
        k->ch.scar[CH_FEAR] = clampf(k->ch.scar[CH_FEAR] + dark_fear * 0.2f, 0.0f, 1.0f);
        k->ch.scar[CH_RAGE] = clampf(k->ch.scar[CH_RAGE] + dark_rage * 0.2f, 0.0f, 1.0f);
    }

    /* 2d. KK background mood — inner library colors the emotion */
    kk_blend_mood(&k->kk, emotion);

    /* 2e. Wormhole check: did any inhale word fulfill a prophecy? */
    wormhole_check(k, matched_indices, n_matched, resp.lang_idx);

    /* 3. MLP: combine emotion + memory + calendar */
    float mem_state[N_CHAMBERS];
    memory_blend(k, mem_state);
    resp.dissonance = calendar_dissonance(k);

    float mlp_input[DIM_MLP_IN];
    for (int c = 0; c < N_CHAMBERS; c++) mlp_input[c] = emotion[c];
    for (int c = 0; c < N_CHAMBERS; c++) mlp_input[N_CHAMBERS + c] = mem_state[c];
    mlp_input[12] = resp.dissonance;

    float mlp_out[N_CHAMBERS];
    mlp_forward(&k->mlp, mlp_input, mlp_out);

    /* 4. HyperKuramoto cross-fire on RESIDUAL state (decays old energy) */
    chambers_crossfire(&k->ch, XFIRE_ITERS);

    /* 5. Inject emotion AFTER crossfire — new signal is NOT decayed */
    for (int c = 0; c < N_CHAMBERS; c++) {
        float new_act = clampf(
            0.4f * emotion[c] + 0.3f * mlp_out[c] + 0.2f * mem_state[c] + 0.1f * k->ch.soma[c],
            0.0f, 1.0f);
        k->ch.act[c] = new_act;
        for (int s = 0; s < N_SUB; s++) {
            k->ch.sub_act[c][s] = clampf(
                k->ch.sub_act[c][s] + new_act, 0.0f, 1.0f);
        }
    }

    /* 5b. Scar update */
    scars_update(&k->ch);

    /* 5c. RBA-1 seven-layer stack update */
    rba_update(k, resp.dissonance);

    /* 5d. Store coherence in history for Ĉ(t) */
    k->coherence_history[k->coherence_ptr] = k->rba.coherence;
    k->coherence_ptr = (k->coherence_ptr + 1) % COHERENCE_WINDOW;

    /* 5e. Velocity detection */
    velocity_detect(k);

    memcpy(resp.chambers, k->ch.act, sizeof(resp.chambers));

    /* find dominant chamber */
    resp.dominant = 0;
    for (int c = 1; c < N_CHAMBERS; c++)
        if (resp.chambers[c] > resp.chambers[resp.dominant]) resp.dominant = c;

    /* 6. Calendar → prophetic premonitions */
    prophetic_premonition(k, resp.dissonance, resp.premonition);
    resp.is_prophetic = (resp.dissonance > 0.3f && k->mem_n >= 2);

    /* 7. MetaKlaus ghost attention (dark matter boosts 1.5x in exhale_generate) */
    metaklaus_compute(k, resp.lang_idx);
    resp.ghost_strength = k->ghost.interference;

    /* 7b. KK: compute knowledge signal for current state + language */
    {
        const char *ex_texts[MAX_EXHALE];
        float ex_affs[MAX_EXHALE][N_CHAMBERS];
        for (int w = 0; w < lp->n_exhale && w < MAX_EXHALE; w++) {
            ex_texts[w] = lp->exhale[w].text;
            memcpy(ex_affs[w], lp->exhale[w].aff, N_CHAMBERS * sizeof(float));
        }
        kk_compute_signal(&k->kk, k->ch.act, ex_texts, ex_affs, lp->n_exhale);
    }

    /* 8. EXHALE: generate somatic response */
    int word_ids[MAX_RESPONSE];
    resp.n_words = exhale_generate(k, resp.lang_idx, word_ids, MAX_RESPONSE);
    for (int i = 0; i < resp.n_words; i++) {
        snprintf(resp.words[i], MAX_WORD, "%s", lp->exhale[word_ids[i]].text);
    }

    /* ═══════════════════════════════════════════════════════════
     * 8b. META-RECURSION LOOP (Schectman's I-Layer made concrete)
     *
     * Klaus generates exhale → observes its own response →
     * adjusts chambers → re-generates.
     * The body reacts, then observes its reaction, then
     * reacts to the observation. Ĉ(t) accumulates.
     * ═══════════════════════════════════════════════════════════ */
    {
        /* concatenate exhale words into a meta-prompt */
        char meta_prompt[MAX_PROMPT];
        int mlen = 0;
        for (int i = 0; i < resp.n_words && mlen < MAX_PROMPT - MAX_WORD - 2; i++) {
            int wl = (int)strlen(resp.words[i]);
            memcpy(meta_prompt + mlen, resp.words[i], wl);
            meta_prompt[mlen + wl] = ' ';
            mlen += wl + 1;
        }
        if (mlen > 0) meta_prompt[mlen - 1] = '\0';
        else meta_prompt[0] = '\0';

        /* meta-inhale: process our own exhale as input */
        float meta_emotion[N_CHAMBERS];
        inhale_process(lp, meta_prompt, meta_emotion, NULL, 0);

        /* run MLP on meta-emotion (meta-observation mode) */
        float meta_mlp_in[DIM_MLP_IN];
        for (int c = 0; c < N_CHAMBERS; c++) meta_mlp_in[c] = meta_emotion[c];
        for (int c = 0; c < N_CHAMBERS; c++) meta_mlp_in[N_CHAMBERS + c] = k->ch.soma[c];
        meta_mlp_in[12] = resp.dissonance;

        float meta_mlp_out[N_CHAMBERS];
        mlp_forward(&k->mlp, meta_mlp_in, meta_mlp_out);

        /* update meta-chambers */
        for (int c = 0; c < N_CHAMBERS; c++) {
            k->meta.meta_chambers.act[c] = clampf(
                0.5f * meta_emotion[c] + 0.3f * meta_mlp_out[c] + 0.2f * k->ch.soma[c],
                0.0f, 1.0f);
        }

        /* blend meta-chambers into primary: 85% primary + 15% meta */
        for (int c = 0; c < N_CHAMBERS; c++) {
            k->ch.act[c] = clampf(
                (1.0f - META_BLEND) * k->ch.act[c] +
                META_BLEND * k->meta.meta_chambers.act[c],
                0.0f, 1.0f);
        }

        /* track recursion depth */
        k->meta.depth = 1;
        k->rba.recursion_depth = 1;

        /* re-generate exhale with blended chambers (FINAL output) */
        /* save used state to avoid re-penalizing same words */
        int saved_n_used = k->n_used;
        k->n_used = 0; /* allow fresh selection for meta-pass */
        k->n_prev = 0; /* reset context — meta-pass starts somatic-fresh */

        int meta_word_ids[MAX_RESPONSE];
        int meta_n = exhale_generate(k, resp.lang_idx, meta_word_ids, MAX_RESPONSE);

        /* use meta-generated output as final response */
        resp.n_words = meta_n;
        for (int i = 0; i < meta_n; i++) {
            snprintf(resp.words[i], MAX_WORD, "%s", lp->exhale[meta_word_ids[i]].text);
        }

        /* merge used lists */
        k->n_used = saved_n_used;
        for (int i = 0; i < meta_n; i++) {
            if (k->n_used < MAX_EXHALE) k->used_exhale[k->n_used++] = meta_word_ids[i];
        }

        /* update prev_exhale from final meta output */
        k->n_prev = meta_n < 4 ? meta_n : 4;
        for (int i = 0; i < k->n_prev; i++)
            k->prev_exhale[i] = meta_word_ids[meta_n - k->n_prev + i];
    }

    /* 9. Store to memory */
    memory_store(k, resp.dissonance);

    /* 10. Tick prophecies */
    prophecy_tick(k);

    /* 11. Experience consolidation (every N interactions) */
    experience_consolidate(k);

    /* 12. Copy Level 3 state to response */
    resp.velocity = k->velocity.current;
    resp.velocity_intensity = k->velocity.intensity;
    resp.coherence = k->rba.coherence;
    resp.recursive_complexity = k->rba.recursive_complexity;
    resp.deep_somatic = k->rba.deep_somatic;
    resp.total_scar = k->ch.total_scar;
    memcpy(resp.scars, k->ch.scar, sizeof(resp.scars));
    resp.meta_recursion_depth = k->meta.depth;
    resp.phase_gate = psi_phase_gate(k);
    resp.sustained_resonance = k->rba.sustained_resonance;

    /* 13. Spore update: residue + decay + save */
    spore_update_residue(k);
    spore_decay(k);
    spore_save(k);

    /* 14. Save somatic state to disk */
    soma_save(k);

    return resp;
}

/* ═══════════════════════════════════════════════════════════════
 * OUTPUT FORMATTING
 * ═══════════════════════════════════════════════════════════════ */

static void print_response(const KlausResponse *r) {
    /* chamber state */
    printf("  [");
    for (int c = 0; c < N_CHAMBERS; c++) {
        printf("%s:%.2f", CH_NAMES[c], r->chambers[c]);
        if (c < N_CHAMBERS-1) printf(" ");
    }
    printf("]\n");

    /* velocity operator */
    printf("  {%s x%.2f}", VEL_NAMES[r->velocity], r->velocity_intensity);
    if (r->deep_somatic)
        printf(" *DEEP SOMATIC*");
    printf("\n");

    /* somatic response */
    printf("  ");
    for (int i = 0; i < r->n_words; i++) {
        printf("%s", r->words[i]);
        if (i < r->n_words - 1) printf(". ");
        else printf(".");
    }
    printf("\n");

    /* ghost voice */
    if (r->ghost_strength > 0.1f) {
        printf("  (metaklaus: %s-dominant, interference %.2f",
               CH_NAMES[r->dominant], r->ghost_strength);
        if (r->dark_matter_active)
            printf(", DARK MATTER x1.5");
        printf(")\n");
    }

    /* RBA-1 state */
    printf("  [RBA-1 coherence:%.3f C-hat:%.3f psi:%.3f sustained:%.3f meta-depth:%d]\n",
           r->coherence, r->recursive_complexity,
           r->phase_gate, r->sustained_resonance,
           r->meta_recursion_depth);

    /* scars */
    if (r->total_scar > 0.01f) {
        printf("  [scars:");
        for (int c = 0; c < N_CHAMBERS; c++) {
            if (r->scars[c] > 0.01f)
                printf(" %s:%.2f", CH_NAMES[c], r->scars[c]);
        }
        printf(" total:%.2f]\n", r->total_scar);
    }

    /* prophetic premonition */
    if (r->is_prophetic) {
        printf("  ~premonition~ [");
        int dom = 0;
        for (int c = 1; c < N_CHAMBERS; c++)
            if (r->premonition[c] > r->premonition[dom]) dom = c;
        printf("->%s:%.2f", CH_NAMES[dom], r->premonition[dom]);
        printf(" dissonance:%.2f]\n", r->dissonance);
    }
}

/* ═══════════════════════════════════════════════════════════════
 * INTERACTIVE LOOP
 * ═══════════════════════════════════════════════════════════════ */

static void interactive(Klaus *k) {
    char prompt[MAX_PROMPT];
    printf("klaus> ");
    fflush(stdout);

    while (fgets(prompt, sizeof(prompt), stdin)) {
        /* strip newline */
        int len = (int)strlen(prompt);
        while (len > 0 && (prompt[len-1]=='\n' || prompt[len-1]=='\r'))
            prompt[--len] = '\0';
        if (len == 0) { printf("klaus> "); fflush(stdout); continue; }

        /* exit commands */
        if (strcmp(prompt, "exit") == 0 || strcmp(prompt, "quit") == 0 ||
            strcmp(prompt, "q") == 0) break;

        /* status command */
        if (strcmp(prompt, "status") == 0) {
            printf("  languages: %d", k->n_langs);
            for (int i = 0; i < k->n_langs; i++)
                printf(" [%s]", k->langs[i].code);
            printf("\n  memory: %d/%d slots\n", k->mem_n, MEM_SLOTS);
            printf("  chambers: ");
            for (int c = 0; c < N_CHAMBERS; c++)
                printf("%s:%.2f ", CH_NAMES[c], k->ch.act[c]);
            printf("\n  soma: ");
            for (int c = 0; c < N_CHAMBERS; c++)
                printf("%.2f ", k->ch.soma[c]);
            printf("\n  calendar dissonance: %.3f\n", calendar_dissonance(k));
            printf("  prophecies: %d active\n", k->n_prophecy);
            printf("  ghost interference: %.3f\n", k->ghost.interference);
            /* Level 3 status */
            printf("  --- Level 3: Schectman ---\n");
            printf("  interactions: %d\n", k->interaction_count);
            printf("  velocity: %s (intensity %.2f)\n",
                   VEL_NAMES[k->velocity.current], k->velocity.intensity);
            printf("  RBA-1: coherence %.3f, C-hat %.3f, entropy %.3f\n",
                   k->rba.coherence, k->rba.recursive_complexity, k->rba.entropy);
            printf("  Psi-layer: gate %.3f, phase_lock %.3f, deep=%d (ticks=%d)\n",
                   psi_phase_gate(k), k->rba.phase_lock,
                   k->rba.deep_somatic, k->rba.deep_ticks);
            printf("  sustained resonance: %.3f\n", k->rba.sustained_resonance);
            printf("  scars: ");
            for (int c = 0; c < N_CHAMBERS; c++)
                if (k->ch.scar[c] > 0.01f)
                    printf("%s:%.3f ", CH_NAMES[c], k->ch.scar[c]);
            printf("(total %.3f)\n", k->ch.total_scar);
            printf("  wormholes: %d logged\n", k->n_wormholes);
            printf("  meta-recursion depth: %d\n", k->meta.depth);
            printf("  soma file: %s\n", k->soma_path);
            printf("klaus> "); fflush(stdout);
            continue;
        }

        /* reset */
        if (strcmp(prompt, "reset") == 0) {
            chambers_init(&k->ch);
            chambers_init(&k->meta.meta_chambers);
            k->mem_n = k->mem_ptr = 0;
            k->n_prophecy = 0;
            k->n_used = 0;
            k->n_prev = 0;
            k->n_wormholes = 0;
            k->interaction_count = 0;
            k->coherence_ptr = 0;
            k->dark_matter_active = 0;
            memset(&k->rba, 0, sizeof(k->rba));
            memset(&k->velocity, 0, sizeof(k->velocity));
            memset(&k->experience, 0, sizeof(k->experience));
            memset(k->coherence_history, 0, sizeof(k->coherence_history));
            k->meta.depth = 0;
            /* remove soma file on explicit reset */
            remove(k->soma_path);
            printf("  [reset — all levels, soma file deleted]\n");
            printf("klaus> "); fflush(stdout);
            continue;
        }

        /* process prompt */
        KlausResponse r = klaus_process(k, prompt);
        print_response(&r);

        printf("klaus> "); fflush(stdout);
    }

    printf("\n[klaus] exhale. goodbye.\n");
}

/* ═══════════════════════════════════════════════════════════════
 * SINGLE-SHOT MODE (for testing)
 * ═══════════════════════════════════════════════════════════════ */

static void single_shot(Klaus *k, const char *prompt) {
    KlausResponse r = klaus_process(k, prompt);
    print_response(&r);
}

/* ═══════════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════════ */

int main(int argc, char **argv) {
    const char *base_dir = ".";
    int interactive_mode = 1;
    const char *single_prompt = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dir") == 0 && i+1 < argc) {
            base_dir = argv[++i];
        } else if (strcmp(argv[i], "--interactive") == 0) {
            interactive_mode = 1;
        } else if (strcmp(argv[i], "--prompt") == 0 && i+1 < argc) {
            single_prompt = argv[++i];
            interactive_mode = 0;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("KLAUS v%s — Kinetic Linguistic Adaptive Unified Sonar\n", KLAUS_VERSION);
            printf("Usage: %s [options]\n", argv[0]);
            printf("  --dir DIR       Base directory with inhale/ and exhale/ (default: .)\n");
            printf("  --interactive   Interactive mode (default)\n");
            printf("  --prompt TEXT   Single prompt mode\n");
            printf("  --help          This message\n");
            return 0;
        } else if (argv[i][0] != '-') {
            single_prompt = argv[i];
            interactive_mode = 0;
        }
    }

    Klaus *k = (Klaus *)calloc(1, sizeof(Klaus));
    if (!k) { fprintf(stderr, "ERROR: out of memory\n"); return 1; }

    if (!klaus_init(k, base_dir)) {
        for (int i = 0; i < k->n_cross_pairs; i++)
            free(k->cross_pairs[i].cross);
        free(k);
        return 1;
    }

    if (interactive_mode) {
        interactive(k);
    } else if (single_prompt) {
        single_shot(k, single_prompt);
    }

    /* free cross-affinity kernels */
    for (int i = 0; i < k->n_cross_pairs; i++)
        free(k->cross_pairs[i].cross);

    free(k);
    return 0;
}
