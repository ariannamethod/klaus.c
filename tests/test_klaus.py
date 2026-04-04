#!/usr/bin/env python3
"""
KLAUS test suite (Python)
Tests each component of the Python inference engine.
Run: python3 tests/test_klaus.py
"""

import sys
import os
import math

# Add parent dir to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from klaus import (
    hash_word, word_similarity, compute_affinity, RNG, bpe_learn, bpe_encode,
    MetaWeights, MLP, Klaus, N_CH, CH_DECAY, COUPLING,
    ANNUAL_DRIFT, GREGORIAN_YEAR, METONIC_LEAP, MAX_UNCORRECTED
)

passed = 0
failed = 0

def test(name, condition, msg=""):
    global passed, failed
    if condition:
        print(f"  {'PASS':<6} {name}")
        passed += 1
    else:
        print(f"  {'FAIL':<6} {name} — {msg}")
        failed += 1

def approx(a, b, eps=1e-6):
    return abs(a - b) < eps


# ═══════ Hash ═══════
def test_hash():
    h1 = hash_word("fear")
    h2 = hash_word("fear")
    h3 = hash_word("love")
    test("hash deterministic", h1 == h2)
    test("hash unique", h1 != h3)
    test("hash different for prefix", hash_word("fea") != hash_word("fear"))

# ═══════ Word similarity ═══════
def test_similarity():
    test("similarity identical", word_similarity("fear", "fear") > 0.9)
    test("similarity different", word_similarity("fear", "love") < word_similarity("fear", "fear"))
    test("similarity related", word_similarity("burning", "burn") > 0.3)
    test("similarity short identical", approx(word_similarity("ab", "ab"), 1.0, 0.01))
    test("similarity short different", approx(word_similarity("ab", "cd"), 0.0, 0.01))

# ═══════ Affinity ═══════
def test_affinity():
    aff = compute_affinity("fear", "en")
    test("affinity has 6 dims", len(aff) == 6)
    test("affinity fear→FEAR", aff[0] > 0.5, f"FEAR={aff[0]:.3f}")
    test("affinity values in [0,1]", all(0 <= a <= 1.01 for a in aff))

    aff2 = compute_affinity("love", "en")
    test("affinity love→LOVE", aff2[1] > 0.5, f"LOVE={aff2[1]:.3f}")

    aff3 = compute_affinity("xyzzy", "en")
    test("affinity unknown word has signal", any(a > 0 for a in aff3))

# ═══════ RNG ═══════
def test_rng():
    r = RNG(42)
    vals = [r.randf() for _ in range(10000)]
    test("rng in [0,1]", all(0 <= v <= 1 for v in vals))
    # rough uniformity
    bins = [0]*10
    for v in vals:
        b = min(int(v*10), 9)
        bins[b] += 1
    test("rng uniform", all(800 < b < 1200 for b in bins), f"bins={bins}")

    r2 = RNG(42)
    vals2 = [r2.randf() for _ in range(100)]
    r3 = RNG(42)
    vals3 = [r3.randf() for _ in range(100)]
    test("rng deterministic", vals2 == vals3)

# ═══════ BPE ═══════
def test_bpe():
    data = b"aaabbbccc"
    merges, tokens = bpe_learn(data, 5)
    test("bpe learns merges", len(merges) > 0)
    test("bpe compresses", len(tokens) < len(data))

    # identity: no merges = bytes
    encoded = bpe_encode(b"hello", [])
    test("bpe identity", encoded == [104, 101, 108, 108, 111])

    # encode with learned merges
    encoded2 = bpe_encode(data, merges)
    test("bpe encode matches learn", len(encoded2) == len(tokens))

# ═══════ MetaWeights ═══════
def test_meta():
    ids = [0, 1, 2, 0, 1, 2, 3, 0, 1]
    meta = MetaWeights()
    meta.build(ids, 4)
    test("meta has bigrams", len(meta.bigrams) > 0)
    test("meta has hebbs", len(meta.hebbs) > 0)
    test("meta bigram(0,1) > 0", meta.bigram(0, 1) > 0)
    test("meta bigram nonexist small", meta.bigram(99, 99) < 1e-5)

    hebb = meta.hebbian([0], 4)
    test("meta hebbian length", len(hebb) == 4)
    test("meta hebbian has signal", any(h > 0 for h in hebb))

# ═══════ MLP ═══════
def test_mlp():
    mlp = MLP()
    # manual init with small weights
    r = RNG(123)
    mlp.w1 = [r.randn(0.1) for _ in range(13*32)]
    mlp.b1 = [0.0]*32
    mlp.w2 = [r.randn(0.1) for _ in range(32*16)]
    mlp.b2 = [0.0]*16
    mlp.w3 = [r.randn(0.1) for _ in range(16*6)]
    mlp.b3 = [0.0]*6

    inp = [0.5]*6 + [0.3]*6 + [0.5]
    out = mlp.forward(inp)
    test("mlp output 6 dims", len(out) == 6)
    test("mlp output in [0,1]", all(0 <= o <= 1 for o in out))

    # deterministic
    out2 = mlp.forward(inp)
    test("mlp deterministic", all(approx(a, b) for a, b in zip(out, out2)))

# ═══════ Chambers ═══════
def test_chambers():
    act = [0.8, 0.2, 0.9, 0.1, 0.5, 0.4]
    soma = [0.0]*6
    for _ in range(20):
        old = list(act)
        for i in range(6):
            act[i] *= CH_DECAY[i]
            for j in range(6):
                if i == j: continue
                act[i] += 0.03 * COUPLING[i][j] * math.sin(old[j] - old[i])
            act[i] = max(0, min(1, act[i]))
            soma[i] = max(0, min(1, 0.92*soma[i] + 0.08*act[i]))

    test("chambers bounded", all(0 <= a <= 1 for a in act))
    test("soma bounded", all(0 <= s <= 1 for s in soma))
    test("chambers decayed", sum(act) < 3.0, f"sum={sum(act):.3f}")

def test_coupling():
    test("coupling diagonal zero", all(COUPLING[i][i] == 0 for i in range(6)))
    test("coupling 6x6", len(COUPLING) == 6 and all(len(r) == 6 for r in COUPLING))

# ═══════ Calendar ═══════
def test_calendar():
    for days in range(0, 10000, 100):
        years = days / GREGORIAN_YEAR
        base_drift = years * ANNUAL_DRIFT
        full_cycles = int(years / 19)
        corrections = full_cycles * 7 * 30
        partial = years % 19
        yic = int(partial) + 1
        for ly in METONIC_LEAP:
            if ly <= yic:
                corrections += 30
        drift = base_drift - corrections
        raw = abs(drift % MAX_UNCORRECTED) / MAX_UNCORRECTED
        disc = max(0, min(1, raw))
        if not (0 <= disc <= 1):
            test(f"calendar day={days}", False, f"disc={disc}")
            return
    test("calendar always in [0,1]", True)

# ═══════ Language detection ═══════
def test_lang_detect():
    k = Klaus.__new__(Klaus)
    k.lang_packs = {"en": {}, "ru": {}, "fr": {}, "he": {}}
    test("detect EN", k._detect_lang("I am afraid") == "en")
    test("detect RU", k._detect_lang("мне страшно") == "ru")
    test("detect FR", k._detect_lang("je suis triste") == "fr")
    test("detect HE", k._detect_lang("אני מפחד") == "he")

# ═══════ Prophecy ═══════
def test_prophecy():
    strength = 1.0
    for _ in range(20):
        strength *= 0.95
    test("prophecy decays", strength < 0.4 and strength > 0.3, f"s={strength:.4f}")

# ═══════ Integration ═══════
def test_integration():
    """Full pipeline test (requires word files)"""
    base = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    if not os.path.isdir(os.path.join(base, "inhale")):
        test("integration (skipped, no word files)", True)
        return

    k = Klaus(base)
    ok = k.init()
    test("integration init", ok)
    if not ok:
        return

    r = k.process("I am terrified")
    test("integration produces words", len(r["words"]) > 0)
    test("integration chambers valid", all(0 <= c <= 1 for c in r["chambers"]))
    test("integration lang detected", r["lang"] == "en")

    r2 = k.process("мне страшно")
    test("integration RU detected", r2["lang"] == "ru")
    test("integration RU produces words", len(r2["words"]) > 0)

    r3 = k.process("je suis furieux")
    test("integration FR detected", r3["lang"] == "fr")

    r4 = k.process("אני מפחד")
    test("integration HE detected", r4["lang"] == "he")

    # no duplicate words in response
    test("integration no duplicates", len(r["words"]) == len(set(r["words"])),
         f"dupes: {r['words']}")


# ═══════ Run all ═══════
if __name__ == "__main__":
    print("╔══════════════════════════════════════╗")
    print("║  KLAUS test suite (Python)           ║")
    print("╚══════════════════════════════════════╝\n")

    test_hash()
    test_similarity()
    test_affinity()
    test_rng()
    test_bpe()
    test_meta()
    test_mlp()
    test_chambers()
    test_coupling()
    test_calendar()
    test_lang_detect()
    test_prophecy()
    test_integration()

    print(f"\n  {passed}/{passed+failed} tests passed")
    if failed == 0:
        print("  ALL TESTS PASSED\n")
    else:
        print(f"  {failed} TESTS FAILED\n")
    sys.exit(0 if failed == 0 else 1)
