"""
kk.py — Knowledge Kernel for Klaus (and any Arianna Method organism)

Inner library. Scans docs/ for .txt files, extracts mood vectors
and keywords, provides resonance signal for the Dario equation.

Import with fallback:
    try: from kk import KnowledgeKernel
    except ImportError: KnowledgeKernel = None

If KnowledgeKernel is None, organism runs without library. Silent.

(c) 2026 Arianna Method
"""

import os, re, math

N_CH = 6
CH_NAMES = ["FEAR", "LOVE", "RAGE", "VOID", "FLOW", "CMPLX"]
KK_MOOD_BLEND = 0.08
KK_CHUNK_WORDS = 200
DARIO_KAPPA = 0.20


def _vec_dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def _vec_norm(v):
    return math.sqrt(sum(x * x for x in v) + 1e-12)


def _word_similarity(a, b):
    la, lb = len(a), len(b)
    if la < 3 or lb < 3:
        return 1.0 if a == b else 0.0
    matches = 0
    for i in range(la - 2):
        for j in range(lb - 2):
            if a[i] == b[j] and a[i+1] == b[j+1] and a[i+2] == b[j+2]:
                matches += 1
                break
    total = la - 2
    return matches / total if total > 0 else 0.0


class KnowledgeKernel:
    """Inner library for any somatic organism.

    Usage:
        kk = KnowledgeKernel(base_dir=".", lang_packs=organism.lang_packs)
        kk.load()  # scans docs/
        # each interaction:
        emotion = kk.blend_mood(emotion)  # layer 1: background mood
        kk.compute_signal(chambers, lang, exhale_words)  # layer 2: force K
        k_value = kk.signal[w]  # use in Dario equation
    """

    def __init__(self, base_dir=".", lang_packs=None):
        self.base_dir = base_dir
        self.lang_packs = lang_packs or {}
        self.docs = []
        self.blended_mood = [0.0] * N_CH
        self.signal = []
        self.active_doc = -1
        self.active_chunk = -1

    def load(self):
        """Scan docs/ directory. Returns number of docs loaded."""
        docs_dir = os.path.join(self.base_dir, "docs")
        if not os.path.isdir(docs_dir):
            return 0
        for fname in sorted(os.listdir(docs_dir)):
            if not fname.endswith(".txt"):
                continue
            path = os.path.join(docs_dir, fname)
            try:
                text = open(path, "r", encoding="utf-8").read()
            except Exception:
                continue
            if len(text) < 20:
                continue
            doc = {"name": fname, "mood": [0.0] * N_CH, "chunks": []}
            doc["mood"] = self._text_mood(text)
            # chunk
            words = text.split()
            for i in range(0, len(words), KK_CHUNK_WORDS):
                chunk_text = " ".join(words[i:i + KK_CHUNK_WORDS])
                if len(chunk_text) < 20:
                    continue
                doc["chunks"].append({
                    "mood": self._text_mood(chunk_text),
                    "keywords": self._extract_keywords(chunk_text),
                })
            if doc["chunks"]:
                self.docs.append(doc)
                dom = max(range(N_CH), key=lambda c: doc["mood"][c])
                print(f"[kk] {fname} — {len(doc['chunks'])} chunks, "
                      f"mood [{CH_NAMES[dom]}:{doc['mood'][dom]:.2f}]")
        # blended mood
        if self.docs:
            for c in range(N_CH):
                self.blended_mood[c] = sum(d["mood"][c] for d in self.docs) / len(self.docs)
        print(f"[kk] {len(self.docs)} documents loaded")
        return len(self.docs)

    def blend_mood(self, emotion):
        """Layer 1: blend library mood into emotion vector. Returns new emotion."""
        if not self.docs:
            return emotion
        return [
            max(0, min(1, (1 - KK_MOOD_BLEND) * emotion[c] + KK_MOOD_BLEND * self.blended_mood[c]))
            for c in range(N_CH)
        ]

    def compute_signal(self, chambers, lang, exhale_list):
        """Layer 2: compute per-exhale knowledge resonance signal."""
        nex = len(exhale_list)
        self.signal = [0.0] * nex
        if not self.docs:
            return
        # choose resonant doc + chunk
        self.active_doc = self._choose_doc(chambers)
        if self.active_doc < 0:
            return
        self.active_chunk = self._choose_chunk(chambers, self.active_doc)
        if self.active_chunk < 0:
            return
        chunk = self.docs[self.active_doc]["chunks"][self.active_chunk]
        # prong 1: keyword matching
        for kw in chunk["keywords"]:
            for w in range(nex):
                text = exhale_list[w]["text"] if isinstance(exhale_list[w], dict) else exhale_list[w]
                if kw in text:
                    self.signal[w] += 1.0
                else:
                    sim = _word_similarity(kw, text)
                    if sim > 0.35:
                        self.signal[w] += sim * 0.6
        # prong 2: chunk mood × exhale affinity (chamber-mediated)
        cnorm = _vec_norm(chunk["mood"])
        if cnorm > 1e-6:
            for w in range(nex):
                aff = exhale_list[w]["aff"] if isinstance(exhale_list[w], dict) else [0] * N_CH
                dot = _vec_dot(chunk["mood"], aff)
                self.signal[w] += dot / cnorm * 0.8
        # normalize
        mx = max(self.signal) if self.signal else 0
        if mx > 0:
            self.signal = [s / mx for s in self.signal]

    def kappa_mod(self, chambers):
        """Somatic modulation of knowledge coefficient."""
        C = chambers
        return max(0.3, min(2.0, 1.0 + 0.4 * C[5] + 0.3 * C[4] - 0.2 * C[2]))

    def force_k(self, w, chambers):
        """Compute force K for exhale word index w."""
        if w >= len(self.signal):
            return 0.0
        return self.kappa_mod(chambers) * DARIO_KAPPA * self.signal[w]

    # ── internal ──

    def _text_mood(self, text):
        mood = [0.0] * N_CH
        total = 0
        words = re.split(r'[\s.,!?;:\"\'()\-/\[\]{}#*]+', text.lower())
        for w in words:
            if not w:
                continue
            for lp in self.lang_packs.values():
                inhale = lp if isinstance(lp, list) else lp.get("inhale", [])
                for iw in inhale:
                    iw_text = iw["text"] if isinstance(iw, dict) else iw
                    if iw_text == w:
                        aff = iw["aff"] if isinstance(iw, dict) else [0] * N_CH
                        for c in range(N_CH):
                            mood[c] += aff[c]
                        total += 1
                        break
        if total > 0:
            mood = [m / total for m in mood]
        return mood

    def _extract_keywords(self, text):
        counts = {}
        for w in re.split(r'[\s.,!?;:\"\'()\-/\[\]{}#*0-9]+', text.lower()):
            if len(w) < 4:
                continue
            counts[w] = counts.get(w, 0) + 1
        ranked = sorted(counts.items(), key=lambda x: -x[1])
        return [w for w, c in ranked[:16] if c >= 2]

    def _choose_doc(self, chambers):
        if not self.docs:
            return -1
        dom = max(range(N_CH), key=lambda c: chambers[c])
        best, best_score = 0, -1e30
        for di, doc in enumerate(self.docs):
            score = _vec_dot(chambers, doc["mood"]) + doc["mood"][dom] * 0.5
            if score > best_score:
                best_score = score
                best = di
        return best

    def _choose_chunk(self, chambers, doc_idx):
        doc = self.docs[doc_idx]
        if not doc["chunks"]:
            return -1
        best, best_score = 0, -1e30
        for ci, chunk in enumerate(doc["chunks"]):
            score = _vec_dot(chambers, chunk["mood"])
            if score > best_score:
                best_score = score
                best = ci
        return best
