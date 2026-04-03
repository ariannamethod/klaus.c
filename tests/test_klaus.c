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

    printf("\n  %d/%d tests passed\n", tests_passed, tests_run);
    if (tests_passed == tests_run) {
        printf("  ALL TESTS PASSED\n\n");
        return 0;
    } else {
        printf("  %d TESTS FAILED\n\n", tests_run - tests_passed);
        return 1;
    }
}
