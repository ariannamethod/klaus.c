/*
 * kk.h — Knowledge Kernel (header-only) for Klaus and Arianna Method organisms
 *
 * Inner library. Scans docs/ for .txt files, extracts mood vectors
 * and keywords, provides resonance signal for the Dario equation.
 *
 * Usage:
 *   #define KK_IMPLEMENTATION   // in ONE .c file only
 *   #include "kk.h"
 *
 * If docs/ doesn't exist, all functions are no-ops. Silent fallback.
 *
 * (c) 2026 Arianna Method
 */

#ifndef KK_H
#define KK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <dirent.h>

#ifndef N_CHAMBERS
#define N_CHAMBERS 6
#endif

#define KK_MAX_DOCS      32
#define KK_MAX_CHUNKS    8
#define KK_MAX_KEYWORDS  16
#define KK_KEYWORD_LEN   48
#define KK_CHUNK_WORDS   200
#define KK_MOOD_BLEND    0.08f
#define KK_DARIO_KAPPA   0.20f
#define KK_MAX_EXHALE    600

/* ── types ── */

typedef struct {
    char keywords[KK_MAX_KEYWORDS][KK_KEYWORD_LEN];
    int n_keywords;
    float mood[N_CHAMBERS];
} KKChunk;

typedef struct {
    char name[128];
    float mood[N_CHAMBERS];
    KKChunk chunks[KK_MAX_CHUNKS];
    int n_chunks;
} KKDoc;

typedef struct {
    KKDoc docs[KK_MAX_DOCS];
    int n_docs;
    float blended_mood[N_CHAMBERS];
    int active_doc;
    int active_chunk;
    float signal[KK_MAX_EXHALE];
    float kappa_mod;
} KK;

/* ── function declarations ── */

/* word_similarity and vec_dot must be provided by the host.
 * declare them here so kk.h can call them. */
#ifndef KK_NO_EXTERN
extern float word_similarity(const char *a, const char *b);
extern float vec_dot(const float *a, const float *b, int n);
extern float vec_norm(const float *v, int n);
#endif

static inline float kk_clampf(float x, float lo, float hi) {
    return x < lo ? lo : x > hi ? hi : x;
}

/* Initialize KK (zero state) */
static inline void kk_init(KK *kk) { memset(kk, 0, sizeof(*kk)); }

/* Load docs from base_dir/docs/. Needs inhale vocab for mood parsing.
 * inhale_texts: array of word strings, inhale_affs: parallel array of float[N_CHAMBERS],
 * n_inhale: count. Call for each language, results accumulate.
 * Returns number of docs loaded. */
int kk_load(KK *kk, const char *base_dir,
            const char **inhale_texts, const float (*inhale_affs)[N_CHAMBERS],
            int n_inhale, const char *ch_names[N_CHAMBERS]);

/* Layer 1: blend library mood into emotion. Modifies emotion in-place. */
static inline void kk_blend_mood(const KK *kk, float *emotion) {
    if (kk->n_docs == 0) return;
    for (int c = 0; c < N_CHAMBERS; c++)
        emotion[c] = kk_clampf(
            (1.0f - KK_MOOD_BLEND) * emotion[c] + KK_MOOD_BLEND * kk->blended_mood[c],
            0.0f, 1.0f);
}

/* Layer 2: compute knowledge signal for current chambers + exhale vocabulary.
 * exhale_affs: float[n_exhale][N_CHAMBERS], exhale_texts: char*[n_exhale]. */
void kk_compute_signal(KK *kk, const float *chambers,
                       const char **exhale_texts,
                       const float (*exhale_affs)[N_CHAMBERS],
                       int n_exhale);

/* Get force K for word index w */
static inline float kk_force_k(const KK *kk, int w) {
    if (w < 0 || w >= KK_MAX_EXHALE || kk->n_docs == 0) return 0.0f;
    return kk->kappa_mod * KK_DARIO_KAPPA * kk->signal[w];
}

/* Update kappa_mod from chamber state */
static inline void kk_update_kappa(KK *kk, const float *chambers) {
    kk->kappa_mod = kk_clampf(
        1.0f + 0.4f * chambers[5] + 0.3f * chambers[4] - 0.2f * chambers[2],
        0.3f, 2.0f);
}

#endif /* KK_H */

/* ═══════════════════════════════════════════════════════════════
 * IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════ */

#ifdef KK_IMPLEMENTATION

static int kk_extract_keywords(const char *text, char keywords[][KK_KEYWORD_LEN], int max_kw) {
    typedef struct { char w[KK_KEYWORD_LEN]; int count; } WC;
    WC counts[512];
    int nc = 0;
    char buf[8192];
    snprintf(buf, sizeof(buf), "%s", text);
    char *saveptr;
    char *tok = strtok_r(buf, " \t\n\r.,!?;:\"'()-/[]{}#*0123456789", &saveptr);
    while (tok) {
        char lower[KK_KEYWORD_LEN];
        int i;
        for (i = 0; tok[i] && i < KK_KEYWORD_LEN - 1; i++)
            lower[i] = tolower((unsigned char)tok[i]);
        lower[i] = '\0';
        if ((int)strlen(lower) < 4) {
            tok = strtok_r(NULL, " \t\n\r.,!?;:\"'()-/[]{}#*0123456789", &saveptr);
            continue;
        }
        int found = 0;
        for (int j = 0; j < nc; j++) {
            if (strcmp(counts[j].w, lower) == 0) { counts[j].count++; found = 1; break; }
        }
        if (!found && nc < 512) {
            snprintf(counts[nc].w, KK_KEYWORD_LEN, "%s", lower);
            counts[nc].count = 1;
            nc++;
        }
        tok = strtok_r(NULL, " \t\n\r.,!?;:\"'()-/[]{}#*0123456789", &saveptr);
    }
    /* selection sort by count desc */
    for (int i = 0; i < nc - 1; i++) {
        int best = i;
        for (int j = i + 1; j < nc; j++)
            if (counts[j].count > counts[best].count) best = j;
        if (best != i) { WC tmp = counts[i]; counts[i] = counts[best]; counts[best] = tmp; }
    }
    int n = 0;
    for (int i = 0; i < nc && n < max_kw; i++) {
        if (counts[i].count < 2) break;
        snprintf(keywords[n], KK_KEYWORD_LEN, "%s", counts[i].w);
        n++;
    }
    return n;
}

static void kk_text_mood(const char *text,
                         const char **inhale_texts, const float (*inhale_affs)[N_CHAMBERS],
                         int n_inhale, float *mood) {
    memset(mood, 0, N_CHAMBERS * sizeof(float));
    int total = 0;
    char buf[8192];
    snprintf(buf, sizeof(buf), "%s", text);
    char *saveptr;
    char *tok = strtok_r(buf, " \t\n\r.,!?;:\"'()-", &saveptr);
    while (tok) {
        char lower[80];
        int i;
        for (i = 0; tok[i] && i < 79; i++) lower[i] = tolower((unsigned char)tok[i]);
        lower[i] = '\0';
        for (int w = 0; w < n_inhale; w++) {
            if (strcmp(lower, inhale_texts[w]) == 0 || strcmp(tok, inhale_texts[w]) == 0) {
                for (int c = 0; c < N_CHAMBERS; c++) mood[c] += inhale_affs[w][c];
                total++;
                break;
            }
        }
        tok = strtok_r(NULL, " \t\n\r.,!?;:\"'()-", &saveptr);
    }
    if (total > 0)
        for (int c = 0; c < N_CHAMBERS; c++) mood[c] /= (float)total;
}

int kk_load(KK *kk, const char *base_dir,
            const char **inhale_texts, const float (*inhale_affs)[N_CHAMBERS],
            int n_inhale, const char *ch_names[N_CHAMBERS]) {
    char docs_dir[512];
    snprintf(docs_dir, sizeof(docs_dir), "%s/docs", base_dir);
    DIR *d = opendir(docs_dir);
    if (!d) { return 0; }
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL && kk->n_docs < KK_MAX_DOCS) {
        char *dot = strrchr(ent->d_name, '.');
        if (!dot || strcmp(dot, ".txt") != 0) continue;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", docs_dir, ent->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
        if (sz <= 0 || sz > 100000) { fclose(f); continue; }
        char *text = (char *)malloc(sz + 1);
        if (!text) { fclose(f); continue; }
        fread(text, 1, sz, f); text[sz] = '\0'; fclose(f);

        KKDoc *doc = &kk->docs[kk->n_docs];
        snprintf(doc->name, sizeof(doc->name), "%s", ent->d_name);
        kk_text_mood(text, inhale_texts, inhale_affs, n_inhale, doc->mood);

        doc->n_chunks = 0;
        char *ptr = text;
        while (*ptr && doc->n_chunks < KK_MAX_CHUNKS) {
            char *chunk_start = ptr;
            int wcount = 0;
            while (*ptr && wcount < KK_CHUNK_WORDS) {
                if (*ptr == ' ' || *ptr == '\n') wcount++;
                ptr++;
            }
            while (*ptr && *ptr != '.' && *ptr != '\n' && (ptr - chunk_start) < 8000) ptr++;
            if (*ptr) ptr++;
            int chunk_len = (int)(ptr - chunk_start);
            if (chunk_len < 20) continue;
            char chunk_buf[8192];
            int copy_len = chunk_len < 8191 ? chunk_len : 8191;
            memcpy(chunk_buf, chunk_start, copy_len);
            chunk_buf[copy_len] = '\0';
            KKChunk *ch = &doc->chunks[doc->n_chunks];
            kk_text_mood(chunk_buf, inhale_texts, inhale_affs, n_inhale, ch->mood);
            ch->n_keywords = kk_extract_keywords(chunk_buf, ch->keywords, KK_MAX_KEYWORDS);
            doc->n_chunks++;
        }
        if (doc->n_chunks > 0) {
            int dom = 0;
            for (int c = 1; c < N_CHAMBERS; c++)
                if (doc->mood[c] > doc->mood[dom]) dom = c;
            printf("[kk] %s — %d chunks, mood [%s:%.2f]\n",
                   doc->name, doc->n_chunks, ch_names[dom], doc->mood[dom]);
            kk->n_docs++;
        }
        free(text);
    }
    closedir(d);
    /* blended mood */
    memset(kk->blended_mood, 0, sizeof(kk->blended_mood));
    if (kk->n_docs > 0) {
        for (int di = 0; di < kk->n_docs; di++)
            for (int c = 0; c < N_CHAMBERS; c++)
                kk->blended_mood[c] += kk->docs[di].mood[c];
        for (int c = 0; c < N_CHAMBERS; c++)
            kk->blended_mood[c] /= (float)kk->n_docs;
    }
    printf("[kk] %d documents loaded\n", kk->n_docs);
    return kk->n_docs;
}

void kk_compute_signal(KK *kk, const float *chambers,
                       const char **exhale_texts,
                       const float (*exhale_affs)[N_CHAMBERS],
                       int n_exhale) {
    if (n_exhale > KK_MAX_EXHALE) n_exhale = KK_MAX_EXHALE;
    memset(kk->signal, 0, sizeof(kk->signal));
    if (kk->n_docs == 0) return;
    /* choose doc */
    int dom = 0;
    for (int c = 1; c < N_CHAMBERS; c++)
        if (chambers[c] > chambers[dom]) dom = c;
    float best_score = -1e30f;
    kk->active_doc = 0;
    for (int di = 0; di < kk->n_docs; di++) {
        float score = vec_dot(chambers, kk->docs[di].mood, N_CHAMBERS);
        score += kk->docs[di].mood[dom] * 0.5f;
        if (score > best_score) { best_score = score; kk->active_doc = di; }
    }
    /* choose chunk */
    KKDoc *doc = &kk->docs[kk->active_doc];
    best_score = -1e30f;
    kk->active_chunk = 0;
    for (int ci = 0; ci < doc->n_chunks; ci++) {
        float score = vec_dot(chambers, doc->chunks[ci].mood, N_CHAMBERS);
        if (score > best_score) { best_score = score; kk->active_chunk = ci; }
    }
    KKChunk *chunk = &doc->chunks[kk->active_chunk];
    /* prong 1: keywords */
    for (int kw = 0; kw < chunk->n_keywords; kw++) {
        for (int w = 0; w < n_exhale; w++) {
            if (strstr(exhale_texts[w], chunk->keywords[kw])) {
                kk->signal[w] += 1.0f;
            } else {
                float sim = word_similarity(chunk->keywords[kw], exhale_texts[w]);
                if (sim > 0.35f) kk->signal[w] += sim * 0.6f;
            }
        }
    }
    /* prong 2: mood × affinity */
    float cnorm = vec_norm(chunk->mood, N_CHAMBERS);
    if (cnorm > 1e-6f) {
        for (int w = 0; w < n_exhale; w++) {
            float dot = vec_dot(chunk->mood, exhale_affs[w], N_CHAMBERS);
            kk->signal[w] += dot / cnorm * 0.8f;
        }
    }
    /* normalize */
    float mx = 0;
    for (int w = 0; w < n_exhale; w++) if (kk->signal[w] > mx) mx = kk->signal[w];
    if (mx > 0) for (int w = 0; w < n_exhale; w++) kk->signal[w] /= mx;
    /* update kappa */
    kk_update_kappa(kk, chambers);
}

#endif /* KK_IMPLEMENTATION */
