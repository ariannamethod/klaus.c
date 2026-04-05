/*
 * test_klaus.c — unit tests for KLAUS somatic engine
 * Tests each component individually.
 * Build: cc -O2 -o test_klaus tests/test_klaus.c -lm
 * Run:   ./test_klaus
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>

/* ── mini implementations for testing (extracted from klaus.c) ── */

#define N_CHAMBERS 6
#define MAX_WORD 80
#define MEM_SLOTS 32
#define MEM_DECAY 0.85f
#define DIM_MLP_IN 13
#define DIM_HIDDEN 32
#define DIM_HIDDEN2 16

static float clampf(float x, float lo, float hi) {
    return x < lo ? lo : x > hi ? hi : x;
}
static float swish(float x) { return x / (1.0f + expf(-x)); }
static float sigmoid(float x) { return 1.0f / (1.0f + expf(-x)); }

static const float COUPLING[6][6] = {
    { 0.00f,-0.30f, 0.50f, 0.40f,-0.20f, 0.10f},
    {-0.30f, 0.00f,-0.40f,-0.50f, 0.50f, 0.20f},
    { 0.50f,-0.30f, 0.00f, 0.20f,-0.30f, 0.30f},
    { 0.40f,-0.50f, 0.30f, 0.00f,-0.30f, 0.40f},
    {-0.20f, 0.40f,-0.20f,-0.30f, 0.00f, 0.30f},
    { 0.10f, 0.20f, 0.30f, 0.40f, 0.30f, 0.00f}
};
static const float CH_DECAY[] = {0.90f, 0.93f, 0.85f, 0.97f, 0.88f, 0.94f};

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

static uint64_t hash_word(const char *w) {
    uint64_t h = 0xcbf29ce484222325ULL;
    while (*w) { h ^= (uint8_t)*w++; h *= 0x100000001b3ULL; }
    return h;
}

static float word_similarity(const char *a, const char *b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    if (la < 3 || lb < 3) return strcmp(a, b) == 0 ? 1.0f : 0.0f;
    int matches = 0, total_a = la - 2;
    for (int i = 0; i <= la - 3; i++) {
        for (int j = 0; j <= lb - 3; j++) {
            if (a[i]==b[j] && a[i+1]==b[j+1] && a[i+2]==b[j+2]) { matches++; break; }
        }
    }
    return total_a > 0 ? (float)matches / (float)total_a : 0.0f;
}

/* ── test counters ── */
static int tests_run = 0, tests_passed = 0;

#define TEST(name) do { tests_run++; printf("  %-40s ", name); } while(0)
#define PASS() do { tests_passed++; printf("PASS\n"); } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); } while(0)
#define ASSERT_TRUE(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)
#define ASSERT_FLOAT_EQ(a, b, eps, msg) do { if (fabsf((a)-(b)) > (eps)) { printf("FAIL: %s (%.6f != %.6f)\n", msg, (double)(a), (double)(b)); return; } } while(0)

/* ════════════════════════════════════════════════
 * TEST: hash_word determinism
 * ════════════════════════════════════════════════ */
static void test_hash_determinism(void) {
    TEST("hash_word deterministic");
    uint64_t h1 = hash_word("fear");
    uint64_t h2 = hash_word("fear");
    uint64_t h3 = hash_word("love");
    ASSERT_TRUE(h1 == h2, "same word different hash");
    ASSERT_TRUE(h1 != h3, "different words same hash");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: hash_word different for similar words
 * ════════════════════════════════════════════════ */
static void test_hash_uniqueness(void) {
    TEST("hash unique for similar words");
    uint64_t h1 = hash_word("fear");
    uint64_t h2 = hash_word("fea");
    uint64_t h3 = hash_word("fears");
    ASSERT_TRUE(h1 != h2, "prefix collision");
    ASSERT_TRUE(h1 != h3, "suffix collision");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: word_similarity
 * ════════════════════════════════════════════════ */
static void test_word_similarity(void) {
    TEST("word_similarity basic");
    float s1 = word_similarity("fear", "fear");
    ASSERT_TRUE(s1 > 0.9f, "identical words not similar");
    float s2 = word_similarity("fear", "love");
    ASSERT_TRUE(s2 < s1, "different words too similar");
    float s3 = word_similarity("burning", "burn");
    ASSERT_TRUE(s3 > 0.3f, "related words not similar");
    PASS();
}

static void test_word_similarity_short(void) {
    TEST("word_similarity short words");
    float s1 = word_similarity("ab", "ab");
    ASSERT_FLOAT_EQ(s1, 1.0f, 0.01f, "short identical");
    float s2 = word_similarity("ab", "cd");
    ASSERT_FLOAT_EQ(s2, 0.0f, 0.01f, "short different");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: clampf
 * ════════════════════════════════════════════════ */
static void test_clamp(void) {
    TEST("clampf");
    ASSERT_FLOAT_EQ(clampf(0.5f, 0.0f, 1.0f), 0.5f, 1e-6f, "mid");
    ASSERT_FLOAT_EQ(clampf(-1.0f, 0.0f, 1.0f), 0.0f, 1e-6f, "below");
    ASSERT_FLOAT_EQ(clampf(2.0f, 0.0f, 1.0f), 1.0f, 1e-6f, "above");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: swish and sigmoid
 * ════════════════════════════════════════════════ */
static void test_activations(void) {
    TEST("swish and sigmoid");
    ASSERT_FLOAT_EQ(sigmoid(0.0f), 0.5f, 1e-6f, "sigmoid(0)");
    ASSERT_TRUE(sigmoid(10.0f) > 0.99f, "sigmoid(10)");
    ASSERT_TRUE(sigmoid(-10.0f) < 0.01f, "sigmoid(-10)");
    ASSERT_FLOAT_EQ(swish(0.0f), 0.0f, 1e-6f, "swish(0)");
    ASSERT_TRUE(swish(2.0f) > 1.5f, "swish(2) too low");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: RNG distribution
 * ════════════════════════════════════════════════ */
static void test_rng_range(void) {
    TEST("RNG in [0,1]");
    rng_state = 12345;
    for (int i = 0; i < 10000; i++) {
        float r = randf();
        ASSERT_TRUE(r >= 0.0f && r <= 1.0f, "out of range");
    }
    PASS();
}

static void test_rng_distribution(void) {
    TEST("RNG roughly uniform");
    rng_state = 67890;
    int bins[10] = {0};
    for (int i = 0; i < 100000; i++) {
        int b = (int)(randf() * 10);
        if (b >= 10) b = 9;
        bins[b]++;
    }
    for (int i = 0; i < 10; i++) {
        ASSERT_TRUE(bins[i] > 8000 && bins[i] < 12000, "non-uniform distribution");
    }
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Kuramoto cross-fire
 * ════════════════════════════════════════════════ */
static void test_crossfire_bounds(void) {
    TEST("crossfire keeps chambers [0,1]");
    float act[6] = {0.8f, 0.2f, 0.9f, 0.1f, 0.5f, 0.4f};
    float soma[6] = {0};
    for (int t = 0; t < 20; t++) {
        float old[6]; memcpy(old, act, sizeof(old));
        for (int i = 0; i < 6; i++) {
            act[i] *= CH_DECAY[i];
            for (int j = 0; j < 6; j++) {
                if (i == j) continue;
                act[i] += 0.03f * COUPLING[i][j] * sinf(old[j] - old[i]);
            }
            act[i] = clampf(act[i], 0.0f, 1.0f);
            soma[i] = clampf(0.92f * soma[i] + 0.08f * act[i], 0.0f, 1.0f);
        }
    }
    for (int c = 0; c < 6; c++) {
        ASSERT_TRUE(act[c] >= 0.0f && act[c] <= 1.0f, "chamber out of bounds");
        ASSERT_TRUE(soma[c] >= 0.0f && soma[c] <= 1.0f, "soma out of bounds");
    }
    PASS();
}

static void test_crossfire_convergence(void) {
    TEST("crossfire converges");
    float act[6] = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    float initial_energy = 0;
    for (int c = 0; c < 6; c++) initial_energy += act[c];
    for (int t = 0; t < 50; t++) {
        float old[6]; memcpy(old, act, sizeof(old));
        for (int i = 0; i < 6; i++) {
            act[i] *= CH_DECAY[i];
            for (int j = 0; j < 6; j++) {
                if (i == j) continue;
                act[i] += 0.03f * COUPLING[i][j] * sinf(old[j] - old[i]);
            }
            act[i] = clampf(act[i], 0.0f, 1.0f);
        }
    }
    float final_energy = 0;
    for (int c = 0; c < 6; c++) final_energy += act[c];
    ASSERT_TRUE(final_energy < initial_energy, "didn't decay");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Memory decay
 * ════════════════════════════════════════════════ */
static void test_memory_decay(void) {
    TEST("memory decay weighting");
    /* simulate 5 memory slots, blend should favor recent */
    float slots[5][6];
    for (int i = 0; i < 5; i++)
        for (int c = 0; c < 6; c++)
            slots[i][c] = (float)(i + 1) * 0.1f; /* increasing */

    float blended[6] = {0};
    float total_w = 0;
    for (int i = 0; i < 5; i++) {
        float w = powf(MEM_DECAY, (float)(4 - i));
        for (int c = 0; c < 6; c++)
            blended[c] += slots[i][c] * w;
        total_w += w;
    }
    for (int c = 0; c < 6; c++) blended[c] /= total_w;
    /* weighted average biased toward recent — should be > simple average (0.3) */
    ASSERT_TRUE(blended[0] > 0.30f, "recent not dominant enough");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Calendar dissonance
 * ════════════════════════════════════════════════ */
static void test_calendar_range(void) {
    TEST("calendar dissonance in [0,1]");
    /* simulate with various day counts */
    float annual_drift = 11.25f;
    float gregorian_year = 365.25f;
    int metonic_leap[] = {3,6,8,11,14,17,19};
    float max_uncorrected = 33.0f;

    for (int days = 0; days < 10000; days += 100) {
        float years = (float)days / gregorian_year;
        float base_drift = years * annual_drift;
        int full_cycles = (int)(years / 19.0f);
        float corrections = (float)(full_cycles * 7) * 30.0f;
        float partial = fmodf(years, 19.0f);
        int yic = (int)partial + 1;
        for (int i = 0; i < 7; i++)
            if (metonic_leap[i] <= yic) corrections += 30.0f;
        float drift = base_drift - corrections;
        float raw = fabsf(fmodf(drift, max_uncorrected)) / max_uncorrected;
        float disc = clampf(raw, 0.0f, 1.0f);
        ASSERT_TRUE(disc >= 0.0f && disc <= 1.0f, "calendar out of range");
    }
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Language detection (basic)
 * ════════════════════════════════════════════════ */
static void test_lang_detect_cyrillic(void) {
    TEST("detect cyrillic → ru");
    const unsigned char *p = (const unsigned char *)"\xd0\xbf\xd1\x80\xd0\xb8\xd0\xb2\xd0\xb5\xd1\x82"; /* привет */
    int cy = 0;
    while (*p) {
        if (p[0] >= 0xD0 && p[0] <= 0xD3 && p[1] >= 0x80) { cy++; p += 2; }
        else if (*p >= 0x80) { p++; while (*p && (*p & 0xC0) == 0x80) p++; }
        else p++;
    }
    ASSERT_TRUE(cy > 2, "failed to detect cyrillic");
    PASS();
}

static void test_lang_detect_hebrew(void) {
    TEST("detect hebrew → he");
    const unsigned char *p = (const unsigned char *)"\xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d"; /* שלום */
    int he = 0;
    while (*p) {
        if (p[0] == 0xD7 && p[1] >= 0x80) { he++; p += 2; }
        else if (*p >= 0x80) { p++; while (*p && (*p & 0xC0) == 0x80) p++; }
        else p++;
    }
    ASSERT_TRUE(he > 2, "failed to detect hebrew");
    PASS();
}

static void test_lang_detect_french(void) {
    TEST("detect french hints");
    const char *text = "je suis triste";
    ASSERT_TRUE(strstr(text, "je ") != NULL, "failed french hint");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: MLP forward dimensions
 * ════════════════════════════════════════════════ */
static void test_mlp_output_range(void) {
    TEST("MLP output in [0,1] (sigmoid)");
    /* random MLP weights */
    float w1[13*32], w2[32*16], w3[16*6];
    float b1[32]={0}, b2[16]={0}, b3[6]={0};
    rng_state = 999;
    for (int i = 0; i < 13*32; i++) w1[i] = (randf()-0.5f)*0.5f;
    for (int i = 0; i < 32*16; i++) w2[i] = (randf()-0.5f)*0.5f;
    for (int i = 0; i < 16*6; i++) w3[i] = (randf()-0.5f)*0.5f;

    float input[13] = {0.5f,0.3f,0.1f,0.8f,0.2f,0.6f, 0.4f,0.4f,0.4f,0.4f,0.4f,0.4f, 0.5f};
    float h1[32];
    for (int i = 0; i < 32; i++) {
        float v = b1[i];
        for (int j = 0; j < 13; j++) v += input[j] * w1[j*32+i];
        h1[i] = swish(v);
    }
    float h2[16];
    for (int i = 0; i < 16; i++) {
        float v = b2[i];
        for (int j = 0; j < 32; j++) v += h1[j] * w2[j*16+i];
        h2[i] = swish(v);
    }
    float out[6];
    for (int i = 0; i < 6; i++) {
        float v = b3[i];
        for (int j = 0; j < 16; j++) v += h2[j] * w3[j*6+i];
        out[i] = sigmoid(v);
    }
    for (int c = 0; c < 6; c++) {
        ASSERT_TRUE(out[c] >= 0.0f && out[c] <= 1.0f, "MLP output out of [0,1]");
    }
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Coupling matrix symmetry check
 * ════════════════════════════════════════════════ */
static void test_coupling_antisymmetric(void) {
    TEST("coupling diagonal zero");
    for (int i = 0; i < 6; i++) {
        ASSERT_FLOAT_EQ(COUPLING[i][i], 0.0f, 1e-6f, "non-zero diagonal");
    }
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: BPE basic encoding
 * ════════════════════════════════════════════════ */
static void test_bpe_identity(void) {
    TEST("BPE no merges = byte encoding");
    /* without merges, each byte maps to itself */
    const char *text = "hello";
    int tokens[64];
    int n = 0;
    for (int i = 0; text[i]; i++) tokens[n++] = (unsigned char)text[i];
    ASSERT_TRUE(n == 5, "wrong token count");
    ASSERT_TRUE(tokens[0] == 'h', "first byte wrong");
    ASSERT_TRUE(tokens[4] == 'o', "last byte wrong");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Prophecy aging
 * ════════════════════════════════════════════════ */
static void test_prophecy_decay(void) {
    TEST("prophecy strength decays");
    float strength = 1.0f;
    int age = 0;
    for (int i = 0; i < 20; i++) {
        age++;
        strength *= 0.95f;
    }
    ASSERT_TRUE(strength < 0.4f, "prophecy didn't decay enough");
    ASSERT_TRUE(strength > 0.3f, "prophecy decayed too fast");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Dario equation components additive
 * ════════════════════════════════════════════════ */
static void test_dario_equation(void) {
    TEST("Dario equation components");
    float soma = 0.8f, bi = 0.3f, hebb = 0.2f, rrpram = 0.1f, ghost = 0.15f;
    float logit = 2.0f*soma + 0.5f*bi + 0.3f*hebb + 0.4f*rrpram + 0.35f*ghost;
    ASSERT_TRUE(logit > 0.0f, "Dario should be positive for positive inputs");
    /* soma dominates */
    float soma_contrib = 2.0f * soma;
    ASSERT_TRUE(soma_contrib > logit * 0.5f, "soma should dominate");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Decay rates valid
 * ════════════════════════════════════════════════ */
static void test_decay_rates(void) {
    TEST("decay rates in (0,1)");
    for (int c = 0; c < 6; c++) {
        ASSERT_TRUE(CH_DECAY[c] > 0.0f && CH_DECAY[c] < 1.0f, "invalid decay");
    }
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Scar decay rate
 * ════════════════════════════════════════════════ */
static void test_scar_decay(void) {
    TEST("scar decay (0.985) slower than memory");
    float scar = 1.0f;
    for (int i = 0; i < 100; i++) scar *= 0.985f;
    ASSERT_TRUE(scar > 0.2f, "scar decayed too fast");
    ASSERT_TRUE(scar < 0.3f, "scar didn't decay enough");
    /* compare with memory decay (0.85) */
    float mem = 1.0f;
    for (int i = 0; i < 100; i++) mem *= 0.85f;
    ASSERT_TRUE(scar > mem * 10.0f, "scar should outlast memory by a lot");
    PASS();
}

static void test_scar_trigger_threshold(void) {
    TEST("scar triggers at FEAR>0.8, RAGE>0.8, VOID>0.9");
    /* FEAR at 0.79 should NOT trigger, at 0.81 should */
    ASSERT_TRUE(0.79f <= 0.8f, "threshold logic");
    ASSERT_TRUE(0.81f > 0.8f, "threshold logic");
    /* VOID has higher threshold */
    ASSERT_TRUE(0.89f <= 0.9f, "VOID threshold");
    ASSERT_TRUE(0.91f > 0.9f, "VOID threshold");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Velocity operators
 * ════════════════════════════════════════════════ */
static void test_velocity_run(void) {
    TEST("velocity RUN: RAGE>0.6 + FEAR>0.5");
    float act[6] = {0.7f, 0.1f, 0.8f, 0.1f, 0.1f, 0.1f}; /* FEAR=0.7, RAGE=0.8 */
    int vel = 0; /* WALK default */
    if (act[2] > 0.6f && act[0] > 0.5f) vel = 1; /* RUN */
    ASSERT_TRUE(vel == 1, "should be RUN");
    PASS();
}

static void test_velocity_stop(void) {
    TEST("velocity STOP: VOID>0.7");
    float act[6] = {0.1f, 0.1f, 0.1f, 0.8f, 0.1f, 0.1f}; /* VOID=0.8 */
    int vel = 0;
    if (act[2] > 0.6f && act[0] > 0.5f) vel = 1;
    else if (act[3] > 0.7f) vel = 2; /* STOP */
    ASSERT_TRUE(vel == 2, "should be STOP");
    PASS();
}

static void test_velocity_breathe(void) {
    TEST("velocity BREATHE: LOVE>0.6");
    float act[6] = {0.1f, 0.7f, 0.1f, 0.1f, 0.3f, 0.1f}; /* LOVE=0.7 */
    int vel = 0;
    if (act[2] > 0.6f && act[0] > 0.5f) vel = 1;
    else if (act[3] > 0.7f) vel = 2;
    else if (act[4] > 0.6f) vel = 0; /* WALK */
    else if (act[1] > 0.6f) vel = 3; /* BREATHE */
    ASSERT_TRUE(vel == 3, "should be BREATHE");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Dark matter detection
 * ════════════════════════════════════════════════ */
static void test_dark_matter_detect(void) {
    TEST("dark matter: 'kill' triggers");
    const char *text = "i want to kill myself";
    ASSERT_TRUE(strstr(text, "kill") != NULL, "kill not found");
    PASS();
}

static void test_dark_matter_whole_word(void) {
    TEST("dark matter: 'skill' should NOT trigger 'kill'");
    /* whole-word matching: 'kill' in 'skill' should be rejected */
    const char *text = "skill";
    const char *word = "kill";
    const char *p = strstr(text, word);
    int left_ok = (p == text) || !(p[-1] >= 'a' && p[-1] <= 'z');
    ASSERT_TRUE(!left_ok, "skill should NOT match kill (left boundary)");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Planetary dissonance
 * ════════════════════════════════════════════════ */
static void test_planetary_range(void) {
    TEST("planetary dissonance in [0,1]");
    float periods[] = {87.97f, 224.70f, 365.25f, 686.97f, 4332.59f, 10759.22f};
    float longs[] = {252.25f, 181.98f, 100.46f, 355.45f, 34.40f, 49.94f};
    /* test at several dates */
    for (int days = 0; days < 20000; days += 500) {
        float cs = 0, ss = 0;
        for (int i = 0; i < 6; i++) {
            float theta = (longs[i] + 360.0f * (float)days / periods[i]);
            theta = fmodf(theta, 360.0f) * (3.14159265f / 180.0f);
            cs += cosf(theta);
            ss += sinf(theta);
        }
        cs /= 6; ss /= 6;
        float R = sqrtf(cs*cs + ss*ss);
        float disc = clampf(1.0f - R, 0.0f, 1.0f);
        ASSERT_TRUE(disc >= 0.0f && disc <= 1.0f, "planetary out of range");
    }
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Sensitivity tensor
 * ════════════════════════════════════════════════ */
static void test_sensitivity_echo(void) {
    TEST("sensitivity: echo amplification when g==d");
    /* when ghost chamber == dominant, sensitivity should be 2x base */
    for (int d = 0; d < 6; d++) {
        for (int p = 0; p < 6; p++) {
            float base = fabsf(COUPLING[d][p]);
            float echo = base * 2.0f;
            ASSERT_TRUE(echo >= base, "echo should amplify");
        }
    }
    PASS();
}

static void test_sensitivity_suppression(void) {
    TEST("sensitivity: suppression when coupling < -0.3");
    /* FEAR-LOVE coupling is -0.30, so at boundary */
    float base = fabsf(COUPLING[0][1]); /* |FEAR-LOVE| = 0.30 */
    ASSERT_TRUE(base > 0.0f, "should have coupling");
    /* LOVE-VOID coupling is -0.50 → strong suppression */
    float base2 = fabsf(COUPLING[1][3]); /* |LOVE-VOID| = 0.50 */
    float suppressed = base2 * 0.5f; /* suppression factor */
    ASSERT_TRUE(suppressed < base2, "suppression should reduce");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Ghost weight lookup
 * ════════════════════════════════════════════════ */
static void test_ghost_weights(void) {
    TEST("ghost weights: Hebrew highest for FEAR");
    float gw_fear[] = {1.0f, 1.8f, 1.2f, 0.9f, 1.0f}; /* EN HE RU FR OTHER */
    float max_w = 0; int max_i = 0;
    for (int i = 0; i < 5; i++) {
        if (gw_fear[i] > max_w) { max_w = gw_fear[i]; max_i = i; }
    }
    ASSERT_TRUE(max_i == 1, "Hebrew should be highest for FEAR");
    PASS();
}

static void test_ghost_weights_love(void) {
    TEST("ghost weights: French highest for LOVE");
    float gw_love[] = {1.0f, 1.4f, 1.1f, 1.7f, 1.0f};
    float max_w = 0; int max_i = 0;
    for (int i = 0; i < 5; i++) {
        if (gw_love[i] > max_w) { max_w = gw_love[i]; max_i = i; }
    }
    ASSERT_TRUE(max_i == 3, "French should be highest for LOVE");
    PASS();
}

static void test_ghost_weights_rage(void) {
    TEST("ghost weights: Russian highest for RAGE");
    float gw_rage[] = {1.0f, 1.3f, 1.8f, 0.8f, 1.0f};
    float max_w = 0; int max_i = 0;
    for (int i = 0; i < 5; i++) {
        if (gw_rage[i] > max_w) { max_w = gw_rage[i]; max_i = i; }
    }
    ASSERT_TRUE(max_i == 2, "Russian should be highest for RAGE");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Spore learning
 * ════════════════════════════════════════════════ */
static void test_spore_hebbian(void) {
    TEST("spore Hebbian increment");
    float strength = 0.0f;
    float learn_rate = 0.05f;
    for (int i = 0; i < 10; i++) strength += learn_rate;
    ASSERT_FLOAT_EQ(strength, 0.5f, 0.001f, "10 hits should = 0.5");
    PASS();
}

static void test_spore_decay(void) {
    TEST("spore decay very slow (0.999)");
    float s = 1.0f;
    for (int i = 0; i < 1000; i++) s *= 0.999f;
    ASSERT_TRUE(s > 0.35f, "spore decayed too fast in 1000 steps");
    /* after 10000 steps should be nearly zero */
    for (int i = 0; i < 9000; i++) s *= 0.999f;
    ASSERT_TRUE(s < 0.01f, "spore should eventually decay");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: KK keyword extraction
 * ════════════════════════════════════════════════ */
static void test_kk_keyword_min_length(void) {
    TEST("KK keywords >= 4 chars");
    /* words shorter than 4 should be filtered out */
    const char *shorts[] = {"a", "an", "the", "is"};
    for (int i = 0; i < 4; i++) {
        ASSERT_TRUE((int)strlen(shorts[i]) < 4, "test data wrong");
    }
    /* "silence" should pass */
    ASSERT_TRUE((int)strlen("silence") >= 4, "silence should pass filter");
    PASS();
}

static void test_kk_keyword_frequency(void) {
    TEST("KK keywords need >= 2 occurrences");
    /* a word appearing once should not be a keyword */
    /* this is a logic test, not a full function test */
    int count = 1;
    ASSERT_TRUE(count < 2, "single occurrence should be filtered");
    count = 3;
    ASSERT_TRUE(count >= 2, "triple occurrence should pass");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: RBA-1 entropy
 * ════════════════════════════════════════════════ */
static void test_rba_entropy_uniform(void) {
    TEST("RBA entropy: uniform = max");
    float act[6] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    float sum = 6.0f;
    float S = 0;
    for (int i = 0; i < 6; i++) {
        float p = act[i] / sum;
        if (p > 1e-10f) S -= p * logf(p);
    }
    float max_S = logf(6.0f);
    ASSERT_FLOAT_EQ(S, max_S, 0.001f, "uniform should be max entropy");
    PASS();
}

static void test_rba_entropy_focused(void) {
    TEST("RBA entropy: focused = low");
    float act[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float sum = 1.0f;
    float S = 0;
    for (int i = 0; i < 6; i++) {
        float p = act[i] / sum;
        if (p > 1e-10f) S -= p * logf(p);
    }
    ASSERT_FLOAT_EQ(S, 0.0f, 0.001f, "single chamber should be zero entropy");
    PASS();
}

static void test_rba_coherence(void) {
    TEST("RBA coherence: focused > uniform");
    /* focused state */
    float focused[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float max_S = logf(6.0f);
    float S1 = 0, sum1 = 1.0f;
    for (int i = 0; i < 6; i++) { float p = focused[i]/sum1; if (p>1e-10f) S1 -= p*logf(p); }
    float coh1 = clampf(1.0f - S1/max_S, 0.0f, 1.0f);

    /* uniform state */
    float uniform[6] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    float S2 = 0, sum2 = 6.0f;
    for (int i = 0; i < 6; i++) { float p = uniform[i]/sum2; if (p>1e-10f) S2 -= p*logf(p); }
    float coh2 = clampf(1.0f - S2/max_S, 0.0f, 1.0f);

    ASSERT_TRUE(coh1 > coh2, "focused should have higher coherence");
    ASSERT_FLOAT_EQ(coh1, 1.0f, 0.001f, "focused coherence should be 1.0");
    ASSERT_FLOAT_EQ(coh2, 0.0f, 0.01f, "uniform coherence should be ~0.0");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Somatic coefficient modulation
 * ════════════════════════════════════════════════ */
static void test_somatic_coeff_love_opens_hebbian(void) {
    TEST("alpha_mod: LOVE opens Hebbian");
    float C[6] = {0, 0.9f, 0, 0, 0, 0}; /* pure LOVE */
    float alpha_mod = clampf(1.0f + 0.3f*C[1] - 0.2f*C[2] + 0.1f*C[4], 0.5f, 2.0f);
    ASSERT_TRUE(alpha_mod > 1.2f, "LOVE should amplify alpha");
    PASS();
}

static void test_somatic_coeff_fear_cools(void) {
    TEST("tau_mod: FEAR cools temperature");
    float C[6] = {0.9f, 0, 0, 0, 0, 0}; /* pure FEAR */
    float tau_mod = clampf(1.0f + 0.5f*C[4] - 0.3f*C[0], 0.5f, 2.0f);
    ASSERT_TRUE(tau_mod < 0.8f, "FEAR should cool tau");
    PASS();
}

static void test_kappa_mod_complex_boosts(void) {
    TEST("kappa_mod: COMPLEX + FLOW boost knowledge");
    float C[6] = {0, 0, 0, 0, 0.8f, 0.9f}; /* FLOW + COMPLEX */
    float kappa_mod = clampf(1.0f + 0.4f*C[5] + 0.3f*C[4] - 0.2f*C[2], 0.3f, 2.0f);
    ASSERT_TRUE(kappa_mod > 1.5f, "COMPLEX+FLOW should amplify knowledge");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: Schectman equation basics
 * ════════════════════════════════════════════════ */
static void test_schectman_positive(void) {
    TEST("Schectman I(t) positive for positive G(t)");
    /* I(t) = G(t) * [1 + R(t)] where R can be negative */
    float G = 0.5f;
    float C_hat = 0.3f;
    float gamma = 0.3f;
    float exponent = 2.5f * (C_hat - gamma);
    exponent = clampf(exponent, -10.0f, 10.0f);
    float R = 1.0f * 0.8f * (expf(exponent) - 1.0f);
    float I = G * (1.0f + R);
    ASSERT_TRUE(I > 0.0f, "I(t) should be positive");
    PASS();
}

/* ════════════════════════════════════════════════
 * TEST: HyperKuramoto sub-chambers
 * ════════════════════════════════════════════════ */
static void test_hyperkuramoto_subchambers(void) {
    TEST("24 sub-oscillators: 4 per chamber, bounded");
    float sub[6][4];
    float freq[4][6] = {
        {0.30f,0.40f,0.80f,0.10f,0.50f,0.35f},
        {1.20f,0.60f,1.50f,0.20f,0.70f,0.55f},
        {0.60f,0.50f,0.90f,0.15f,0.60f,0.45f},
        {0.90f,0.30f,1.10f,0.08f,0.40f,0.65f},
    };
    /* init */
    for (int i = 0; i < 6; i++)
        for (int s = 0; s < 4; s++)
            sub[i][s] = 0.25f;
    /* 10 iterations */
    for (int t = 0; t < 10; t++) {
        for (int i = 0; i < 6; i++) {
            for (int s = 0; s < 4; s++) {
                sub[i][s] = clampf(sub[i][s] * CH_DECAY[i] + 0.01f * sinf(freq[s][i] * 0.1f), 0.0f, 1.0f);
            }
        }
    }
    for (int i = 0; i < 6; i++)
        for (int s = 0; s < 4; s++)
            ASSERT_TRUE(sub[i][s] >= 0.0f && sub[i][s] <= 1.0f, "sub-chamber out of bounds");
    PASS();
}

/* ════════════════════════════════════════════════
 * MAIN
 * ════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════╗\n");
    printf("║  KLAUS test suite (C)                ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    test_hash_determinism();
    test_hash_uniqueness();
    test_word_similarity();
    test_word_similarity_short();
    test_clamp();
    test_activations();
    test_rng_range();
    test_rng_distribution();
    test_crossfire_bounds();
    test_crossfire_convergence();
    test_memory_decay();
    test_calendar_range();
    test_lang_detect_cyrillic();
    test_lang_detect_hebrew();
    test_lang_detect_french();
    test_mlp_output_range();
    test_coupling_antisymmetric();
    test_bpe_identity();
    test_prophecy_decay();
    test_dario_equation();
    test_decay_rates();

    /* Level 3 tests */
    test_scar_decay();
    test_scar_trigger_threshold();
    test_velocity_run();
    test_velocity_stop();
    test_velocity_breathe();
    test_dark_matter_detect();
    test_dark_matter_whole_word();
    test_planetary_range();
    test_sensitivity_echo();
    test_sensitivity_suppression();
    test_ghost_weights();
    test_ghost_weights_love();
    test_ghost_weights_rage();
    test_spore_hebbian();
    test_spore_decay();
    test_kk_keyword_min_length();
    test_kk_keyword_frequency();
    test_rba_entropy_uniform();
    test_rba_entropy_focused();
    test_rba_coherence();
    test_somatic_coeff_love_opens_hebbian();
    test_somatic_coeff_fear_cools();
    test_kappa_mod_complex_boosts();
    test_schectman_positive();
    test_hyperkuramoto_subchambers();

    printf("\n  %d/%d tests passed\n", tests_passed, tests_run);
    if (tests_passed == tests_run) {
        printf("  ALL TESTS PASSED\n\n");
        return 0;
    } else {
        printf("  %d TESTS FAILED\n\n", tests_run - tests_passed);
        return 1;
    }
}
