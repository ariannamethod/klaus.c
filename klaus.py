#!/usr/bin/env python3
"""
KLAUS — Kinetic Linguistic Adaptive Unified Sonar
Python inference. Zero dependencies. Identical to klaus.c.

Usage:
    python3 klaus.py                    # interactive
    python3 klaus.py "I am afraid"      # single prompt
    python3 klaus.py --dir /path/to/dir # custom word files directory
"""

import os
import sys
import math
import time
import struct
from pathlib import Path

# ═══════════════════════════════════════════════════
# CONFIG
# ═══════════════════════════════════════════════════

KLAUS_VERSION = "1.0.0"
N_CHAMBERS = 6
CH_NAMES = ["FEAR", "LOVE", "RAGE", "VOID", "FLOW", "CMPLX"]
XFIRE_ITERS = 8
MEM_SLOTS = 32
MEM_DECAY = 0.85
MAX_RESPONSE = 12
GEN_TEMP = 0.75
TOP_K = 20

# Dario equation coefficients
ALPHA_SOM = 2.0
BETA_BIG = 0.5
GAMMA_HEB = 0.3
DELTA_RRPRAM = 0.4
EPSILON_PROP = 0.25
ZETA_META = 0.35

# Calendar
ANNUAL_DRIFT = 11.25
GREGORIAN_YEAR = 365.25
METONIC_LEAP = [3, 6, 8, 11, 14, 17, 19]
MAX_UNCORRECTED = 33.0

CH_DECAY = [0.90, 0.93, 0.85, 0.97, 0.88, 0.94]
COUPLING = [
    [0.00, -0.30, 0.50, 0.40, -0.20, 0.10],
    [-0.30, 0.00, -0.40, -0.50, 0.50, 0.20],
    [0.50, -0.30, 0.00, 0.20, -0.30, 0.30],
    [0.40, -0.50, 0.30, 0.00, -0.30, 0.40],
    [-0.20, 0.40, -0.20, -0.30, 0.00, 0.30],
    [0.10, 0.20, 0.30, 0.40, 0.30, 0.00],
]

# Anchors
ANCHORS = {
    "en": {0: ["fear","terror","panic","dread","horror","nightmare","anxiety","threat","danger","trapped"],
           1: ["love","warmth","tenderness","kindness","compassion","affection","care","embrace","comfort","gentle"],
           2: ["rage","fury","anger","burning","explosive","violent","hostile","aggressive","bitter","cruel"],
           3: ["empty","hollow","numb","void","darkness","silence","lonely","abandoned","despair","hopeless"],
           4: ["flow","rhythm","dance","pulse","harmony","resonance","vibration","wave","music","breath"],
           5: ["paradox","mystery","chaos","tension","complex","uncertain","transform","strange","enigma","spiral"]},
    "ru": {0: ["страх","ужас","паника","тревога","кошмар"], 1: ["любовь","тепло","нежность","забота"],
           2: ["ярость","гнев","злость"], 3: ["пустота","тишина","одиночество"],
           4: ["поток","ритм","гармония"], 5: ["хаос","тайна","парадокс"]},
    "fr": {0: ["peur","terreur","panique","horreur","angoisse","cauchemar"],
           1: ["amour","chaleur","tendresse","douceur","compassion"],
           2: ["rage","fureur","colère","violence","cruauté"],
           3: ["vide","silence","solitude","désespoir","ténèbres"],
           4: ["flux","rythme","harmonie","danse","résonance"],
           5: ["paradoxe","mystère","chaos","tension"]},
    "he": {0: ["פחד","אימה","חרדה"], 1: ["אהבה","חום","רוך"],
           2: ["זעם","כעס"], 3: ["ריקנות","בדידות","שתיקה"],
           4: ["קצב","הרמוניה"], 5: ["מסתורין","כאוס"]},
}


# ═══════════════════════════════════════════════════
# RNG
# ═══════════════════════════════════════════════════

class RNG:
    def __init__(self, seed=42):
        self.state = seed & 0xFFFFFFFFFFFFFFFF

    def next(self):
        self.state ^= (self.state << 13) & 0xFFFFFFFFFFFFFFFF
        self.state ^= (self.state >> 7) & 0xFFFFFFFFFFFFFFFF
        self.state ^= (self.state << 17) & 0xFFFFFFFFFFFFFFFF
        self.state &= 0xFFFFFFFFFFFFFFFF
        return self.state

    def randf(self):
        return (self.next() & 0x7FFFFFFF) / 0x7FFFFFFF

    def randn(self, std=1.0):
        u1 = self.randf() + 1e-10
        u2 = self.randf()
        return std * math.sqrt(-2 * math.log(u1)) * math.cos(2 * math.pi * u2)

rng = RNG(int(time.time()))


# ═══════════════════════════════════════════════════
# HASHING
# ═══════════════════════════════════════════════════

def hash_word(w):
    h = 0xcbf29ce484222325
    for ch in w:
        h ^= ord(ch)
        h = (h * 0x100000001b3) & 0xFFFFFFFFFFFFFFFF
    return h

def word_similarity(a, b):
    if len(a) < 3 or len(b) < 3:
        return 1.0 if a == b else 0.0
    matches = sum(1 for i in range(len(a)-2) if a[i:i+3] in b)
    total = len(a) - 2
    return matches / total if total > 0 else 0.0


# ═══════════════════════════════════════════════════
# WORD WITH AFFINITY
# ═══════════════════════════════════════════════════

def compute_affinity(word, lang_code):
    aff = [0.0] * N_CHAMBERS
    anchors = ANCHORS.get(lang_code, ANCHORS["en"])

    for c in range(N_CHAMBERS):
        if word in anchors.get(c, []):
            aff[c] = 1.0
            for j in range(N_CHAMBERS):
                if j != c:
                    aff[j] += 0.1 * abs(COUPLING[c][j])
            return aff

    best_sim = [0.0] * N_CHAMBERS
    for c in range(N_CHAMBERS):
        for aw in anchors.get(c, []):
            sim = word_similarity(word, aw)
            if sim > best_sim[c]:
                best_sim[c] = sim

    h = hash_word(word)
    for c in range(N_CHAMBERS):
        hash_aff = ((h >> (c * 8)) & 0xFF) / 255.0
        aff[c] = min(1.0, best_sim[c] * 0.7 + hash_aff * 0.3)

    mx = max(aff) if aff else 0
    if mx > 0:
        aff = [a / mx for a in aff]
    return aff


# ═══════════════════════════════════════════════════
# BPE
# ═══════════════════════════════════════════════════

def bpe_learn(data_bytes, num_merges):
    tokens = list(data_bytes)
    merges = []
    vocab_bytes = {i: bytes([i]) for i in range(256)}
    vocab_size = 256

    for m in range(num_merges):
        pairs = {}
        for i in range(len(tokens) - 1):
            key = (tokens[i], tokens[i+1])
            pairs[key] = pairs.get(key, 0) + 1

        if not pairs:
            break
        best = max(pairs, key=pairs.get)
        if pairs[best] < 2:
            break

        new_id = 256 + m
        merges.append((best[0], best[1], new_id))
        vocab_bytes[new_id] = vocab_bytes.get(best[0], b'') + vocab_bytes.get(best[1], b'')
        vocab_size = new_id + 1

        new_tokens = []
        i = 0
        while i < len(tokens):
            if i < len(tokens) - 1 and tokens[i] == best[0] and tokens[i+1] == best[1]:
                new_tokens.append(new_id)
                i += 2
            else:
                new_tokens.append(tokens[i])
                i += 1
        tokens = new_tokens

    return merges, vocab_size, tokens

def bpe_encode(text_bytes, merges):
    tokens = list(text_bytes)
    for a, b, new_id in merges:
        new_tokens = []
        i = 0
        while i < len(tokens):
            if i < len(tokens) - 1 and tokens[i] == a and tokens[i+1] == b:
                new_tokens.append(new_id)
                i += 2
            else:
                new_tokens.append(tokens[i])
                i += 1
        tokens = new_tokens
    return tokens


# ═══════════════════════════════════════════════════
# METAWEIGHTS
# ═══════════════════════════════════════════════════

class MetaWeights:
    def __init__(self):
        self.unigram = {}
        self.bigrams = {}
        self.hebbs = {}

    def build(self, ids, V):
        for i in ids:
            self.unigram[i] = self.unigram.get(i, 0) + 1
        tot = sum(self.unigram.values())
        if tot > 0:
            self.unigram = {k: v/tot for k, v in self.unigram.items()}

        bi_counts = {}
        for i in range(len(ids) - 1):
            key = (ids[i], ids[i+1])
            bi_counts[key] = bi_counts.get(key, 0) + 1
        a_totals = {}
        for (a, _), v in bi_counts.items():
            a_totals[a] = a_totals.get(a, 0) + v
        for key, v in bi_counts.items():
            if a_totals[key[0]] > 0:
                self.bigrams[key] = v / a_totals[key[0]]

        hn = min(len(ids), 3000)
        win = 5
        hebb_map = {}
        for i in range(hn):
            for j in range(max(0, i-win), min(hn, i+win+1)):
                if i == j:
                    continue
                ka, kb = min(ids[i], ids[j]), max(ids[i], ids[j])
                key = (ka, kb)
                decay = 1.0 / (1.0 + abs(i - j))
                hebb_map[key] = hebb_map.get(key, 0) + decay
        mx = max(hebb_map.values()) if hebb_map else 0
        if mx > 0:
            self.hebbs = {k: v/mx for k, v in hebb_map.items()}

    def bigram(self, prev, next_):
        return self.bigrams.get((prev, next_), 1e-10)

    def hebbian(self, ctx, V):
        out = [0.0] * V
        for c in ctx:
            for (a, b), s in self.hebbs.items():
                if a == c and b < V:
                    out[b] += s
                elif b == c and a < V:
                    out[a] += s
        mx = max(out) if out else 0
        if mx > 0:
            out = [v/mx for v in out]
        return out


# ═══════════════════════════════════════════════════
# MLP
# ═══════════════════════════════════════════════════

class MLP:
    def __init__(self):
        self.w1 = self.w2 = self.w3 = None
        self.b1 = self.b2 = self.b3 = None

    def init_from_vocab(self, lang_packs):
        seed = 0xcbf29ce484222325
        for code, lp in lang_packs.items():
            for w in lp["inhale"]:
                seed ^= hash_word(w["text"]) & 0xFFFFFFFFFFFFFFFF
        r = RNG(seed & 0xFFFFFFFFFFFFFFFF)
        s1 = math.sqrt(2.0 / 13)
        self.w1 = [r.randn(s1) for _ in range(13 * 32)]
        self.b1 = [0.0] * 32
        s2 = math.sqrt(2.0 / 32)
        self.w2 = [r.randn(s2) for _ in range(32 * 16)]
        self.b2 = [0.0] * 16
        s3 = math.sqrt(2.0 / 16)
        self.w3 = [r.randn(s3) for _ in range(16 * 6)]
        self.b3 = [0.0] * 6

    def forward(self, inp):
        def swish(x):
            return x / (1.0 + math.exp(-max(-500, min(500, x))))
        def sigmoid(x):
            return 1.0 / (1.0 + math.exp(-max(-500, min(500, x))))

        h1 = [0.0] * 32
        for i in range(32):
            v = self.b1[i]
            for j in range(13):
                v += inp[j] * self.w1[j * 32 + i]
            h1[i] = swish(v)
        h2 = [0.0] * 16
        for i in range(16):
            v = self.b2[i]
            for j in range(32):
                v += h1[j] * self.w2[j * 16 + i]
            h2[i] = swish(v)
        out = [0.0] * 6
        for i in range(6):
            v = self.b3[i]
            for j in range(16):
                v += h2[j] * self.w3[j * 6 + i]
            out[i] = sigmoid(v)
        return out


# ═══════════════════════════════════════════════════
# KLAUS ENGINE
# ═══════════════════════════════════════════════════

class Klaus:
    def __init__(self, base_dir="."):
        self.base_dir = base_dir
        self.lang_packs = {}
        self.chambers = [0.0] * N_CHAMBERS
        self.soma = [0.0] * N_CHAMBERS
        self.phase = [i * 1.047198 for i in range(N_CHAMBERS)]
        self.presence = 0.0
        self.trauma = 0.0
        self.debt = 0.0
        self.memory = []
        self.mlp = MLP()
        self.prev_exhale = []
        self.used_exhale = set()
        self.prophecies = []
        self.epoch_t = time.mktime(time.strptime("2024-10-03 12:00:00", "%Y-%m-%d %H:%M:%S"))

    def init(self):
        print("╔══════════════════════════════════════════════╗")
        print(f"║  KLAUS — Kinetic Linguistic Adaptive         ║")
        print(f"║          Unified Sonar v{KLAUS_VERSION}                 ║")
        print("║  Zero weights. Pure resonance.               ║")
        print("╚══════════════════════════════════════════════╝")
        print()

        self._load_languages()
        if not self.lang_packs:
            print("ERROR: no language packs found")
            return False

        print(f"[klaus] {len(self.lang_packs)} language(s) loaded")

        for code, lp in self.lang_packs.items():
            self._init_meta(code)

        self.mlp.init_from_vocab(self.lang_packs)
        print(f"[klaus] MLP initialized: 13 → 32 → 16 → 6")

        self.chambers = [0.0] * N_CHAMBERS
        self.chambers[1] = 0.15
        self.chambers[4] = 0.10

        disc = self._calendar_dissonance()
        print(f"[klaus] calendar dissonance: {disc:.3f}")
        print("[klaus] ready. inhale.\n")
        return True

    def _load_languages(self):
        inhale_dir = os.path.join(self.base_dir, "inhale")
        exhale_dir = os.path.join(self.base_dir, "exhale")
        if not os.path.isdir(inhale_dir):
            return

        for fname in sorted(os.listdir(inhale_dir)):
            if not fname.endswith(".txt"):
                continue
            code = fname[:-4]
            ex_path = os.path.join(exhale_dir, f"ex-{code}.txt")
            if not os.path.exists(ex_path):
                print(f"[klaus] WARNING: no exhale for '{code}', skipping")
                continue

            inhale = self._load_words(os.path.join(inhale_dir, fname), code)
            exhale = self._load_words(ex_path, code)
            self.lang_packs[code] = {"inhale": inhale, "exhale": exhale, "meta": None, "bpe_merges": []}
            print(f"[klaus] loaded {code}: {len(inhale)} inhale, {len(exhale)} exhale")

    def _load_words(self, path, lang_code):
        words = []
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                w = line.strip()
                if w:
                    words.append({"text": w, "aff": compute_affinity(w, lang_code)})
        return words

    def _init_meta(self, code):
        lp = self.lang_packs[code]
        corpus = " ".join(w["text"] for w in lp["exhale"])
        corpus += " " + " ".join(w["text"] for w in lp["inhale"])
        data = corpus.encode("utf-8")
        merges, vocab_size, tokens = bpe_learn(data, 512)
        lp["bpe_merges"] = merges
        token_ids = bpe_encode(data, merges)
        meta = MetaWeights()
        meta.build(token_ids, len(lp["exhale"]))
        lp["meta"] = meta
        print(f"[klaus] {code}: BPE {len(merges)} merges, meta {len(meta.bigrams)} bi {len(meta.hebbs)} hebb")

    # ── Language detection ──
    def _detect_language(self, text):
        cyrillic = hebrew = accented = 0
        for ch in text:
            cp = ord(ch)
            if 0x0400 <= cp <= 0x04FF:
                cyrillic += 1
            elif 0x0590 <= cp <= 0x05FF:
                hebrew += 1
            elif 0x00C0 <= cp <= 0x00FF:
                accented += 1
        if hebrew > 2 and "he" in self.lang_packs:
            return "he"
        if cyrillic > 2 and "ru" in self.lang_packs:
            return "ru"
        if accented > 1 and "fr" in self.lang_packs:
            return "fr"
        fr_words = ["je ", "tu ", "le ", "la ", "les ", "suis ", "est ", "dans "]
        for fw in fr_words:
            if fw in text and "fr" in self.lang_packs:
                return "fr"
        if "en" in self.lang_packs:
            return "en"
        return list(self.lang_packs.keys())[0]

    # ── Kuramoto cross-fire ──
    def _crossfire(self, iters):
        for _ in range(iters):
            old = list(self.chambers)
            for i in range(N_CHAMBERS):
                self.chambers[i] *= CH_DECAY[i]
                for j in range(N_CHAMBERS):
                    if i == j:
                        continue
                    self.chambers[i] += 0.03 * COUPLING[i][j] * math.sin(old[j] - old[i])
                self.chambers[i] = max(0, min(1, self.chambers[i]))
                self.soma[i] = max(0, min(1, 0.92 * self.soma[i] + 0.08 * self.chambers[i]))
                self.phase[i] += 0.1 * self.chambers[i]
            self.presence = max(0, min(1,
                0.9 * self.presence + 0.05 * (1 - self.chambers[3]) * self.chambers[4]
                + 0.03 * self.soma[1]))
            self.trauma *= 0.98
            self.debt *= 0.97

    # ── Memory ──
    def _memory_store(self, cal_phase):
        slot = {
            "ch": list(self.chambers),
            "valence": self.chambers[1] + self.chambers[4] - self.chambers[0] - self.chambers[3],
            "arousal": self.chambers[2] + self.chambers[0] + self.chambers[5],
            "cal": cal_phase,
        }
        if len(self.memory) >= MEM_SLOTS:
            self.memory.pop(0)
        self.memory.append(slot)

    def _memory_blend(self):
        out = [0.0] * N_CHAMBERS
        if not self.memory:
            return out
        total_w = 0
        for i, slot in enumerate(reversed(self.memory)):
            w = MEM_DECAY ** i
            for c in range(N_CHAMBERS):
                out[c] += slot["ch"][c] * w
            total_w += w
        if total_w > 0:
            out = [v / total_w for v in out]
        return out

    # ── Calendar ──
    def _calendar_dissonance(self):
        days = (time.time() - self.epoch_t) / 86400.0
        years = days / GREGORIAN_YEAR
        base_drift = years * ANNUAL_DRIFT
        full_cycles = int(years / 19)
        corrections = full_cycles * 7 * 30
        partial = years % 19
        year_in_cycle = int(partial) + 1
        for ly in METONIC_LEAP:
            if ly <= year_in_cycle:
                corrections += 30
        drift = base_drift - corrections
        raw = abs(drift % MAX_UNCORRECTED) / MAX_UNCORRECTED
        return max(0, min(1, raw))

    def _prophetic_premonition(self, dissonance):
        prem = [0.0] * N_CHAMBERS
        if dissonance < 0.3 or len(self.memory) < 2:
            return prem
        m0 = self.memory[-1]
        m1 = self.memory[-2]
        for c in range(N_CHAMBERS):
            vel = m0["ch"][c] - m1["ch"][c]
            prem[c] = max(0, min(1, self.chambers[c] + vel * dissonance * 2))
        return prem

    # ── MetaKlaus ghost ──
    def _metaklaus(self, primary_lang):
        ghost = [0.0] * len(self.lang_packs[primary_lang]["exhale"])
        interference = 0.0
        codes = list(self.lang_packs.keys())
        if len(codes) <= 1:
            return ghost, interference
        primary = self.lang_packs[primary_lang]
        ch_norm = math.sqrt(sum(v*v for v in self.chambers) + 1e-12)
        if ch_norm < 1e-6:
            return ghost, interference

        for w in range(len(primary["exhale"])):
            ghost_sum = 0
            ghost_n = 0
            for code in codes:
                if code == primary_lang:
                    continue
                other = self.lang_packs[code]
                best_sim = -1
                best_idx = -1
                for ow in range(len(other["exhale"])):
                    sim = sum(self.chambers[c] * other["exhale"][ow]["aff"][c] for c in range(N_CHAMBERS)) / ch_norm
                    if sim > best_sim:
                        best_sim = sim
                        best_idx = ow
                if best_idx >= 0:
                    agreement = sum(primary["exhale"][w]["aff"][c] * other["exhale"][best_idx]["aff"][c] for c in range(N_CHAMBERS))
                    interf = (agreement - 0.5) * 2
                    ghost_sum += interf * best_sim
                    ghost_n += 1
            if ghost_n > 0:
                ghost[w] = ghost_sum / ghost_n

        total = sum(abs(v) for v in ghost)
        interference = total / len(primary["exhale"]) if primary["exhale"] else 0
        return ghost, interference

    # ── Inhale ──
    def _inhale(self, lp, prompt):
        emotion = [0.0] * N_CHAMBERS
        matches = 0
        import re
        words = re.split(r'[\s\t\n\r.,!?;:"\'()\-]+', prompt.lower())
        words = [w for w in words if w]

        for w in words:
            for iw in lp["inhale"]:
                if w == iw["text"] or iw["text"] in prompt:
                    for c in range(N_CHAMBERS):
                        emotion[c] += iw["aff"][c]
                    matches += 1
                    break

        if matches == 0:
            for w in words:
                best_sim = 0
                best_w = None
                for iw in lp["inhale"]:
                    sim = word_similarity(w, iw["text"])
                    if sim > best_sim:
                        best_sim = sim
                        best_w = iw
                if best_w and best_sim > 0.2:
                    for c in range(N_CHAMBERS):
                        emotion[c] += best_w["aff"][c] * best_sim
                    matches += 1

        if matches > 0:
            emotion = [v / matches for v in emotion]
        else:
            h = hash_word(prompt)
            emotion = [((h >> (c * 8)) & 0xFF) / 255.0 for c in range(N_CHAMBERS)]
        return emotion

    # ── Exhale ──
    def _exhale(self, lang_code, ghost):
        lp = self.lang_packs[lang_code]
        n_ex = len(lp["exhale"])
        if n_ex == 0:
            return []

        hebb = lp["meta"].hebbian(self.prev_exhale, n_ex)
        result = []
        prev = self.prev_exhale[-1] if self.prev_exhale else -1
        local_used = set(self.used_exhale)
        prop_pressure = sum(p["strength"] for p in self.prophecies) / 3.0

        for step in range(MAX_RESPONSE):
            logits = [0.0] * n_ex
            for w in range(n_ex):
                soma_score = sum(self.chambers[c] * lp["exhale"][w]["aff"][c] for c in range(N_CHAMBERS))
                bi_score = lp["meta"].bigram(prev, w) if prev >= 0 else 0.0
                logits[w] = (ALPHA_SOM * soma_score + BETA_BIG * bi_score
                           + GAMMA_HEB * hebb[w] + ZETA_META * ghost[w]
                           + EPSILON_PROP * (soma_score * 0.5 if prop_pressure > 0.3 else 0))
                if w in local_used or lp["exhale"][w]["text"] in local_used:
                    logits[w] -= 100

            logits = [l / GEN_TEMP for l in logits]
            indexed = sorted(enumerate(logits), key=lambda x: -x[1])[:TOP_K]
            mx = indexed[0][1]
            probs = [math.exp(v - mx) for _, v in indexed]
            s = sum(probs)
            probs = [p / s for p in probs]

            r = rng.randf()
            cum = 0
            chosen = indexed[0][0]
            for i, (idx, _) in enumerate(indexed):
                cum += probs[i]
                if cum >= r:
                    chosen = idx
                    break

            result.append(chosen)
            local_used.add(chosen)
            local_used.add(lp["exhale"][chosen]["text"])
            prev = chosen

            if step > 2:
                score = sum(self.chambers[c] * lp["exhale"][chosen]["aff"][c] for c in range(N_CHAMBERS))
                if score < 0.2:
                    break

        self.prev_exhale = result[-4:]
        self.used_exhale.update(result)
        self.used_exhale.update(lp["exhale"][i]["text"] for i in result)
        return [lp["exhale"][i]["text"] for i in result]

    # ── Process ──
    def process(self, prompt):
        lang = self._detect_language(prompt)
        lp = self.lang_packs[lang]
        emotion = self._inhale(lp, prompt)
        mem_state = self._memory_blend()
        disc = self._calendar_dissonance()

        mlp_in = emotion + mem_state + [disc]
        mlp_out = self.mlp.forward(mlp_in)

        for c in range(N_CHAMBERS):
            self.chambers[c] = max(0, min(1,
                0.4 * emotion[c] + 0.3 * mlp_out[c] + 0.2 * mem_state[c] + 0.1 * self.soma[c]))

        self._crossfire(XFIRE_ITERS)
        prem = self._prophetic_premonition(disc)
        is_prophetic = disc > 0.3 and len(self.memory) >= 2
        ghost, ghost_strength = self._metaklaus(lang)
        words = self._exhale(lang, ghost)
        self._memory_store(disc)

        self.prophecies = [p for p in self.prophecies
                          if (p.update({"age": p["age"]+1, "strength": p["strength"]*0.95}) or True)
                          and p["age"] < 20 and p["strength"] > 0.01]

        return {
            "lang": lang,
            "words": words,
            "chambers": list(self.chambers),
            "prem": prem,
            "disc": disc,
            "ghost_strength": ghost_strength,
            "is_prophetic": is_prophetic,
        }

    def print_response(self, r):
        ch_str = " ".join(f"{CH_NAMES[c]}:{r['chambers'][c]:.2f}" for c in range(N_CHAMBERS))
        print(f"  [{ch_str}]")
        print(f"  {'. '.join(r['words'])}.")
        if r["ghost_strength"] > 0.1:
            print(f"  (metaklaus: interference {r['ghost_strength']:.2f})")
        if r["is_prophetic"]:
            dom = r["prem"].index(max(r["prem"]))
            print(f"  ~premonition~ [→{CH_NAMES[dom]}:{r['prem'][dom]:.2f} dissonance:{r['disc']:.2f}]")

    def interactive(self):
        while True:
            try:
                prompt = input("klaus> ")
            except (EOFError, KeyboardInterrupt):
                print("\n[klaus] exhale. goodbye.")
                break
            prompt = prompt.strip()
            if not prompt:
                continue
            if prompt in ("exit", "quit", "q"):
                print("[klaus] exhale. goodbye.")
                break
            if prompt == "status":
                codes = list(self.lang_packs.keys())
                print(f"  languages: {len(codes)} {codes}")
                print(f"  memory: {len(self.memory)}/{MEM_SLOTS} slots")
                ch_str = " ".join(f"{CH_NAMES[c]}:{self.chambers[c]:.2f}" for c in range(N_CHAMBERS))
                print(f"  chambers: {ch_str}")
                print(f"  calendar dissonance: {self._calendar_dissonance():.3f}")
                continue
            if prompt == "reset":
                self.chambers = [0.0] * N_CHAMBERS
                self.chambers[1] = 0.15
                self.chambers[4] = 0.10
                self.memory = []
                self.used_exhale = set()
                self.prev_exhale = []
                self.prophecies = []
                print("  [reset]")
                continue

            r = self.process(prompt)
            self.print_response(r)


def main():
    base_dir = "."
    single_prompt = None
    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] == "--dir" and i + 1 < len(args):
            base_dir = args[i + 1]
            i += 2
        elif args[i] == "--help":
            print(f"KLAUS v{KLAUS_VERSION} — Kinetic Linguistic Adaptive Unified Sonar")
            print(f"Usage: python3 klaus.py [--dir DIR] [PROMPT]")
            return
        elif not args[i].startswith("-"):
            single_prompt = args[i]
            i += 1
        else:
            i += 1

    k = Klaus(base_dir)
    if not k.init():
        sys.exit(1)

    if single_prompt:
        r = k.process(single_prompt)
        k.print_response(r)
    else:
        k.interactive()


if __name__ == "__main__":
    main()
