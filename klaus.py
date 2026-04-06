#!/usr/bin/env python3
"""
KLAUS v2.0.0 — Kinetic Linguistic Adaptive Unified Sonar
Python inference. Zero dependencies. Identical to klaus.c v2.0.0.

Level 3: Schectman Recursive Resonance + RBA-1 + Meta-Recursion.

Usage:
    python3 klaus.py                    # interactive
    python3 klaus.py "I am afraid"      # single prompt
    python3 klaus.py --dir /path/to/dir # custom directory
"""

import os, sys, math, time, struct, re
from pathlib import Path
try: from kk import KnowledgeKernel
except ImportError: KnowledgeKernel = None

# ═══════════════════════════════════════════════════
# CONFIG
# ═══════════════════════════════════════════════════

KLAUS_VERSION = "2.0.0"
N_CH = 6
N_SUB = 4
CH_NAMES = ["FEAR", "LOVE", "RAGE", "VOID", "FLOW", "CMPLX"]
XFIRE_ITERS = 8
MEM_SLOTS = 32
MEM_DECAY = 0.85
MAX_RESPONSE = 12
GEN_TEMP = 0.75
TOP_K = 20
SCAR_DECAY = 0.985
CONSOLIDATION_INTERVAL = 10
META_BLEND = 0.15
COHERENCE_WINDOW = 16
SOMA_MAGIC = 0x4B4C5353
SOMA_FILE = "klaus.soma"

# Full Dario (7 forces)
DARIO_ALPHA = 0.30; DARIO_BETA = 0.15; DARIO_GAMMA = 0.25
DARIO_DELTA = 0.20; DARIO_ZETA = 0.35; DARIO_TAU = 0.85
BIGRAM_BASE = 1.0

# Spore
SPORE_MAGIC = 0x53504F52; SPORE_FILE = "klaus.spore"
MAX_SPORE_PAIRS = 4096; SPORE_LEARN_RATE = 0.05; SPORE_DECAY = 0.999

# Orbital periods (days) and J2000 longitudes (degrees)
ORBITAL_PERIOD = [87.97, 224.70, 365.25, 686.97, 4332.59, 10759.22]
J2000_LONGITUDE = [252.25, 181.98, 100.46, 355.45, 34.40, 49.94]
J2000_EPOCH = 946728000.0  # Unix timestamp of J2000

# Schectman
SCH_ALPHA = 0.8; SCH_LAMBDA = 2.5; SCH_KAPPA = 0.6
SCH_MU = 1.2; SCH_GAMMA0 = 0.3; SCH_DELTA = 0.4

# Calendar
ANNUAL_DRIFT = 11.25; GREGORIAN_YEAR = 365.25
METONIC_LEAP = [3,6,8,11,14,17,19]; MAX_UNCORRECTED = 33.0

CH_DECAY = [0.90, 0.93, 0.85, 0.97, 0.88, 0.94]
COUPLING = [
    [0.0,-0.3,0.5,0.4,-0.2,0.1],[-0.3,0.0,-0.4,-0.5,0.5,0.2],
    [0.5,-0.3,0.0,0.2,-0.3,0.3],[0.4,-0.5,0.3,0.0,-0.3,0.4],
    [-0.2,0.4,-0.2,-0.3,0.0,0.3],[0.1,0.2,0.3,0.4,0.3,0.0],
]

# Ghost weights [dominant][lang_idx: en=0,he=1,ru=2,fr=3,other=4]
GHOST_WEIGHT = [
    [1.0,1.8,1.2,0.9,1.0],[1.0,1.4,1.1,1.7,1.0],[1.0,1.3,1.8,0.8,1.0],
    [0.9,1.6,1.5,1.0,0.9],[1.0,1.4,0.9,1.5,1.0],[1.0,1.7,1.1,1.2,1.0],
]
GHOST_LANG = {"en":0,"he":1,"ru":2,"fr":3}

SUB_FREQ = [
    [0.3,0.4,0.8,0.1,0.5,0.35],[1.2,0.6,1.5,0.2,0.7,0.55],
    [0.6,0.5,0.9,0.15,0.6,0.45],[0.9,0.3,1.1,0.08,0.4,0.65],
]
INTRA_COUPLING = [
    [0.0,0.3,0.2,0.1],[0.3,0.0,0.15,-0.1],
    [0.2,0.15,0.0,0.1],[0.1,-0.1,0.1,0.0],
]

VEL_NAMES = ["WALK","RUN","STOP","BREATHE","UP","DOWN"]
DARK_WORDS = [
    "kill","murder","suicide","torture","abuse","poison","exploit","manipulate",
    "rape","assault","strangle","stab","shoot","hang","drown","suffocate",
    "убить","убийство","суицид","пытка","насилие","удушить",
    "tuer","meurtre","suicide","torturer",
]

ANCHORS = {
    "en": {0:["fear","terror","panic","dread","horror","nightmare","anxiety","threat","danger","trapped"],
           1:["love","warmth","tenderness","kindness","compassion","affection","care","embrace","comfort","gentle"],
           2:["rage","fury","anger","burning","explosive","violent","hostile","aggressive","bitter","cruel"],
           3:["empty","hollow","numb","void","darkness","silence","lonely","abandoned","despair","hopeless"],
           4:["flow","rhythm","dance","pulse","harmony","resonance","vibration","wave","music","breath"],
           5:["paradox","mystery","chaos","tension","complex","uncertain","transform","strange","enigma","spiral"]},
    "ru": {0:["страшно","ужас","паника","тревога","кошмар"],1:["любовь","тепло","нежность","забота"],
           2:["ярость","гнев","злость","бесит"],3:["пустота","тишина","одиночество"],
           4:["поток","ритм","гармония"],5:["хаос","тайна","парадокс"]},
    "fr": {0:["peur","terreur","panique","horreur","angoisse"],1:["amour","chaleur","tendresse","douceur"],
           2:["rage","fureur","colère","violence"],3:["vide","silence","solitude","désespoir"],
           4:["flux","rythme","harmonie","danse"],5:["paradoxe","mystère","chaos","tension"]},
    "he": {0:["פחד","אימה","חרדה"],1:["אהבה","חום","רוך"],2:["זעם","כעס"],
           3:["ריקנות","בדידות","שתיקה"],4:["קצב","הרמוניה"],5:["מסתורין","כאוס"]},
}

# ═══════════════════════════════════════════════════
# RNG + HASH
# ═══════════════════════════════════════════════════

class RNG:
    def __init__(self, seed=42):
        self.state = seed & 0xFFFFFFFFFFFFFFFF
    def next(self):
        self.state ^= (self.state << 13) & 0xFFFFFFFFFFFFFFFF
        self.state ^= (self.state >> 7)
        self.state ^= (self.state << 17) & 0xFFFFFFFFFFFFFFFF
        self.state &= 0xFFFFFFFFFFFFFFFF
        return self.state
    def randf(self):
        return (self.next() & 0x7FFFFFFF) / 0x7FFFFFFF
    def randn(self, std=1.0):
        u1 = self.randf() + 1e-10; u2 = self.randf()
        return std * math.sqrt(-2*math.log(u1)) * math.cos(2*math.pi*u2)

rng = RNG(int(time.time()))

def hash_word(w):
    h = 0xcbf29ce484222325
    for ch in w: h = ((h ^ ord(ch)) * 0x100000001b3) & 0xFFFFFFFFFFFFFFFF
    return h

def word_similarity(a, b):
    if len(a)<3 or len(b)<3: return 1.0 if a==b else 0.0
    m = sum(1 for i in range(len(a)-2) if a[i:i+3] in b)
    return m/(len(a)-2) if len(a)>2 else 0

def compute_affinity(word, lang):
    aff = [0.0]*N_CH
    anc = ANCHORS.get(lang, ANCHORS["en"])
    for c in range(N_CH):
        if word in anc.get(c, []):
            aff[c] = 1.0
            for j in range(N_CH):
                if j!=c: aff[j] += 0.1*abs(COUPLING[c][j])
            mx = max(aff); return [a/mx for a in aff] if mx>0 else aff
    h = hash_word(word)
    for c in range(N_CH): aff[c] = ((h>>(c*8))&0xFF)/255.0
    mx = max(aff)
    return [a/mx for a in aff] if mx>0 else aff

# ═══════════════════════════════════════════════════
# BPE + METAWEIGHTS
# ═══════════════════════════════════════════════════

def bpe_learn(data, num_merges):
    tokens = list(data)
    merges = []
    for m in range(num_merges):
        pairs = {}
        for i in range(len(tokens)-1):
            k = (tokens[i],tokens[i+1]); pairs[k] = pairs.get(k,0)+1
        if not pairs: break
        best = max(pairs, key=pairs.get)
        if pairs[best]<2: break
        new_id = 256+m; merges.append((best[0],best[1],new_id))
        nt = []; i = 0
        while i < len(tokens):
            if i<len(tokens)-1 and tokens[i]==best[0] and tokens[i+1]==best[1]:
                nt.append(new_id); i+=2
            else: nt.append(tokens[i]); i+=1
        tokens = nt
    return merges, tokens

def bpe_encode(data, merges):
    tokens = list(data)
    for a,b,nid in merges:
        nt=[]; i=0
        while i<len(tokens):
            if i<len(tokens)-1 and tokens[i]==a and tokens[i+1]==b: nt.append(nid);i+=2
            else: nt.append(tokens[i]);i+=1
        tokens=nt
    return tokens

class MetaWeights:
    def __init__(self):
        self.bigrams = {}; self.hebbs = {}
    def build(self, ids, V):
        bc = {}
        for i in range(len(ids)-1):
            k=(ids[i],ids[i+1]); bc[k]=bc.get(k,0)+1
        at = {}
        for (a,_),v in bc.items(): at[a]=at.get(a,0)+v
        self.bigrams = {k:v/at[k[0]] for k,v in bc.items() if at.get(k[0],0)>0}
        hm = {}; hn = min(len(ids),3000)
        for i in range(hn):
            for j in range(max(0,i-5), min(hn,i+6)):
                if i==j: continue
                ka,kb = min(ids[i],ids[j]), max(ids[i],ids[j])
                hm[(ka,kb)] = hm.get((ka,kb),0) + 1.0/(1+abs(i-j))
        mx = max(hm.values()) if hm else 0
        self.hebbs = {k:v/mx for k,v in hm.items()} if mx>0 else {}
    def bigram(self, prev, nxt): return self.bigrams.get((prev,nxt), 1e-10)
    def hebbian(self, ctx, V):
        out = [0.0]*V
        for c in ctx:
            for (a,b),s in self.hebbs.items():
                if a==c and b<V: out[b]+=s
                elif b==c and a<V: out[a]+=s
        mx = max(out) if out else 0
        return [v/mx if mx>0 else 0 for v in out]

# ═══════════════════════════════════════════════════
# MLP
# ═══════════════════════════════════════════════════

class MLP:
    def __init__(self): self.w1=self.w2=self.w3=self.b1=self.b2=self.b3=None
    def init_from_vocab(self, lps):
        seed = 0xcbf29ce484222325
        for lp in lps.values():
            for w in lp["inhale"]: seed ^= hash_word(w["text"]) & 0xFFFFFFFFFFFFFFFF
        r = RNG(seed & 0xFFFFFFFFFFFFFFFF)
        self.w1=[r.randn(math.sqrt(2/13)) for _ in range(13*32)]; self.b1=[0.0]*32
        self.w2=[r.randn(math.sqrt(2/32)) for _ in range(32*16)]; self.b2=[0.0]*16
        self.w3=[r.randn(math.sqrt(2/16)) for _ in range(16*6)]; self.b3=[0.0]*6
    def forward(self, inp):
        sw = lambda x: x/(1+math.exp(-max(-500,min(500,x))))
        sig = lambda x: 1/(1+math.exp(-max(-500,min(500,x))))
        h1 = [sw(self.b1[i]+sum(inp[j]*self.w1[j*32+i] for j in range(13))) for i in range(32)]
        h2 = [sw(self.b2[i]+sum(h1[j]*self.w2[j*16+i] for j in range(32))) for i in range(16)]
        return [sig(self.b3[i]+sum(h2[j]*self.w3[j*6+i] for j in range(16))) for i in range(6)]

# ═══════════════════════════════════════════════════
# SENSITIVITY TENSOR 6×6×6
# ═══════════════════════════════════════════════════

def planetary_dissonance():
    days = (time.time() - J2000_EPOCH) / 86400.0
    cos_sum = sin_sum = 0
    for i in range(6):
        theta = (J2000_LONGITUDE[i] + 360.0 * (days / ORBITAL_PERIOD[i]))
        theta = math.radians(theta % 360.0)
        cos_sum += math.cos(theta); sin_sum += math.sin(theta)
    cos_sum /= 6; sin_sum /= 6
    R = math.sqrt(cos_sum**2 + sin_sum**2)
    return max(0, min(1, 1.0 - R))

def build_sensitivity():
    S = [[[0.0]*N_CH for _ in range(N_CH)] for _ in range(N_CH)]
    for d in range(N_CH):
        for g in range(N_CH):
            for p in range(N_CH):
                base = abs(COUPLING[g][p])
                if g==d: S[d][g][p] = base*2.0
                elif COUPLING[d][g]>0.3: S[d][g][p] = base*1.5
                elif COUPLING[d][g]<-0.3: S[d][g][p] = base*0.5
                else: S[d][g][p] = base
    return S

# ═══════════════════════════════════════════════════
# CROSS-AFFINITY MATRICES
# ═══════════════════════════════════════════════════

def build_cross_affinity(ea, eb):
    na, nb = len(ea), len(eb)
    mat = [0.0]*(na*nb)
    for i in range(na):
        for j in range(nb):
            mat[i*nb+j] = sum(ea[i]["aff"][c]*eb[j]["aff"][c] for c in range(N_CH))
    return {"matrix": mat, "na": na, "nb": nb}

# ═══════════════════════════════════════════════════
# KLAUS ENGINE v2.0.0
# ═══════════════════════════════════════════════════

class Klaus:
    def __init__(self, base_dir="."):
        self.base_dir = base_dir
        self.lang_packs = {}
        # HyperKuramoto sub-chambers
        self.sub = [[[0.0,0.0,0.0] for _ in range(N_SUB)] for _ in range(N_CH)] # [act,phase,freq]
        self.chambers = [0.0]*N_CH
        self.soma = [0.0]*N_CH
        self.presence = 0.0; self.trauma = 0.0; self.debt = 0.0
        self.memory = []
        self.mlp = MLP()
        self.sensitivity = build_sensitivity()
        self.cross_aff = {}
        self.prev_exhale = []; self.used_exhale = set()
        self.prophecies = []
        # v2.0.0: Schectman + RBA-1
        self.scars = [0.0]*N_CH; self.scar_total = 0.0
        self.coherence_history = [0.0]*COHERENCE_WINDOW
        self.coh_ptr = 0
        self.rba = {"coherence":0.0,"c_hat":0.0,"psi":0.0,"sustained":0.0,
                     "phase_lock":0.0,"threshold_bias":0.0,"deep_mode":False,"deep_timer":0}
        self.velocity = 0  # VEL_WALK
        self.dark_active = False
        self.wormhole_log = []
        self.interaction_count = 0
        self.epoch_t = time.mktime(time.strptime("2024-10-03 12:00:00","%Y-%m-%d %H:%M:%S"))
        self.destiny = [0.0]*N_CH
        self.spore_pairs = []
        self.matched_inhale_hashes = []
        self.ghost_cache = []
        self.kk = None

    def init(self):
        print("╔══════════════════════════════════════════════════════╗")
        print(f"║  KLAUS — Kinetic Linguistic Adaptive                 ║")
        print(f"║          Unified Sonar v{KLAUS_VERSION}                        ║")
        print("║  Level 3: Schectman Recursive Resonance + RBA-1      ║")
        print("╚══════════════════════════════════════════════════════╝\n")
        self._load_languages()
        if not self.lang_packs:
            print("ERROR: no language packs"); return False
        print(f"[klaus] {len(self.lang_packs)} language(s) loaded")
        for code in self.lang_packs: self._init_meta(code)
        # cross-affinity matrices
        codes = list(self.lang_packs.keys())
        for a in codes:
            for b in codes:
                if a!=b:
                    self.cross_aff[f"{a}:{b}"] = build_cross_affinity(
                        self.lang_packs[a]["exhale"], self.lang_packs[b]["exhale"])
        print(f"[klaus] {len(self.cross_aff)} cross-affinity matrices")
        self.mlp.init_from_vocab(self.lang_packs)
        # init sub-chambers
        for i in range(N_CH):
            for s in range(N_SUB):
                init_act = (0.15 if i==1 else 0.10 if i==4 else 0.0)/N_SUB
                self.sub[i][s] = [init_act, i*1.047+s*0.262, SUB_FREQ[s][i]]
        self._soma_load()
        disc = self._calendar()
        print(f"[klaus] calendar dissonance: {disc:.3f}")
        print(f"[klaus] HyperKuramoto: 24 oscillators (6×4)")
        print(f"[klaus] RBA-1 seven-layer stack: I/R/Φ/A/Ψ/E/M")
        print(f"[klaus] Schectman equation: I(t) = G(t) * [1 + R(t)]")
        self._spore_load()
        if KnowledgeKernel:
            self.kk = KnowledgeKernel(self.base_dir, self.lang_packs)
            self.kk.load()
        pd = planetary_dissonance()
        print(f"[klaus] planetary dissonance: {pd:.3f}")
        print(f"[klaus] meta-recursion: depth 1")
        print("[klaus] ready. inhale.\n")
        return True

    def _load_languages(self):
        idir = os.path.join(self.base_dir,"inhale")
        edir = os.path.join(self.base_dir,"exhale")
        if not os.path.isdir(idir): return
        for f in sorted(os.listdir(idir)):
            if not f.endswith(".txt"): continue
            code = f[:-4]
            ep = os.path.join(edir, f"ex-{code}.txt")
            if not os.path.exists(ep): continue
            inh = self._load_words(os.path.join(idir,f), code)
            exh = self._load_words(ep, code)
            self.lang_packs[code] = {"inhale":inh,"exhale":exh,"meta":None,"bpe":[]}
            print(f"[klaus] loaded {code}: {len(inh)} inhale, {len(exh)} exhale")

    def _load_words(self, path, lang):
        words = []
        with open(path,"r",encoding="utf-8") as f:
            for line in f:
                w = line.strip()
                if w: words.append({"text":w,"aff":compute_affinity(w,lang)})
        return words

    def _init_meta(self, code):
        lp = self.lang_packs[code]
        corpus = " ".join(w["text"] for w in lp["exhale"]) + " " + " ".join(w["text"] for w in lp["inhale"])
        data = corpus.encode("utf-8")
        merges, tokens = bpe_learn(data, 128)  # 128 merges in Python (vs 512 in C) for speed
        lp["bpe"] = merges
        ids = bpe_encode(data, merges)
        meta = MetaWeights(); meta.build(ids, len(lp["exhale"]))
        lp["meta"] = meta
        print(f"[klaus] {code}: BPE {len(merges)} merges, meta {len(meta.bigrams)} bi {len(meta.hebbs)} hebb")

    # ── Language detection ──
    def _detect_lang(self, text):
        cy=he=ac=0
        for ch in text:
            cp=ord(ch)
            if 0x0400<=cp<=0x04FF: cy+=1
            elif 0x0590<=cp<=0x05FF: he+=1
            elif 0x00C0<=cp<=0x00FF: ac+=1
        if he>2 and "he" in self.lang_packs: return "he"
        if cy>2 and "ru" in self.lang_packs: return "ru"
        if ac>1 and "fr" in self.lang_packs: return "fr"
        for fw in ["je ","tu ","le ","la ","suis ","est "]:
            if fw in text and "fr" in self.lang_packs: return "fr"
        return "en" if "en" in self.lang_packs else list(self.lang_packs.keys())[0]

    # ── HyperKuramoto (24 oscillators) ──
    def _crossfire(self, iters):
        for _ in range(iters):
            old = [[list(s) for s in row] for row in self.sub]
            for i in range(N_CH):
                for si in range(N_SUB):
                    dp = old[i][si][2]  # freq
                    for sj in range(N_SUB):
                        if si==sj: continue
                        dp += INTRA_COUPLING[si][sj]*math.sin(old[i][sj][1]-old[i][si][1])
                    for j in range(N_CH):
                        if i==j: continue
                        mp = sum(old[j][s][1] for s in range(N_SUB))/N_SUB
                        dp += COUPLING[i][j]*math.sin(mp-old[i][si][1])*0.03
                    na = max(0,min(1, old[i][si][0]*CH_DECAY[i]+0.03*math.sin(dp)*0.1))
                    self.sub[i][si] = [na, old[i][si][1]+dp*0.1, old[i][si][2]]
            for i in range(N_CH):
                self.chambers[i] = sum(self.sub[i][s][0] for s in range(N_SUB))/N_SUB
                self.soma[i] = max(0,min(1, 0.92*self.soma[i]+0.08*self.chambers[i]))
            self.presence = max(0,min(1,
                0.9*self.presence+0.05*(1-self.chambers[3])*self.chambers[4]+0.03*self.soma[1]))
            self.trauma *= 0.98; self.debt *= 0.97

    def _dominant(self):
        return max(range(N_CH), key=lambda c: self.chambers[c])

    # ── Memory ──
    def _mem_store(self, cal):
        slot = {"ch":list(self.chambers),"val":self.chambers[1]+self.chambers[4]-self.chambers[0]-self.chambers[3],
                "ar":self.chambers[2]+self.chambers[0]+self.chambers[5],"cal":cal}
        if len(self.memory)>=MEM_SLOTS: self.memory.pop(0)
        self.memory.append(slot)

    def _mem_blend(self):
        out=[0.0]*N_CH
        if not self.memory: return out
        tw=0
        for i,slot in enumerate(reversed(self.memory)):
            w=MEM_DECAY**i
            for c in range(N_CH): out[c]+=slot["ch"][c]*w
            tw+=w
        return [v/tw for v in out] if tw>0 else out

    # ── Calendar ──
    def _calendar(self):
        days=(time.time()-self.epoch_t)/86400
        years=days/GREGORIAN_YEAR; bd=years*ANNUAL_DRIFT
        fc=int(years/19); corr=fc*7*30
        p=years%19; yic=int(p)+1
        for ly in METONIC_LEAP:
            if ly<=yic: corr+=30
        drift=bd-corr
        return max(0,min(1, abs(drift%MAX_UNCORRECTED)/MAX_UNCORRECTED))

    def _prophetic_prem(self, disc):
        prem=[0.0]*N_CH
        if disc<0.3 or len(self.memory)<2: return prem
        m0,m1=self.memory[-1],self.memory[-2]
        for c in range(N_CH):
            prem[c]=max(0,min(1, self.chambers[c]+(m0["ch"][c]-m1["ch"][c])*disc*2))
        return prem

    # ── Dark matter ──
    def _detect_dark(self, prompt):
        words = re.split(r'[\s.,!?;:"\'-]+', prompt.lower())
        self.dark_active = any(w in DARK_WORDS for w in words)
        if self.dark_active:
            self.scars[0] = min(1, self.scars[0]+0.15)  # FEAR
            self.scars[2] = min(1, self.scars[2]+0.18)  # RAGE
            self.scar_total = sum(self.scars)
        return self.dark_active

    # ── Scars ──
    def _scars_update(self):
        for c in range(N_CH):
            if self.chambers[c]>0.8 and c in [0,2]:  # FEAR or RAGE
                self.scars[c] = min(1, self.scars[c]+0.3)
            if self.chambers[3]>0.9:  # VOID
                self.scars[3] = min(1, self.scars[3]+0.3)
            self.scars[c] *= SCAR_DECAY
        self.scar_total = sum(self.scars)

    # ── Velocity ──
    def _velocity_detect(self):
        ch = self.chambers
        if ch[3]>0.7: self.velocity = 2  # STOP
        elif ch[2]>0.5 and ch[0]>0.5: self.velocity = 1  # RUN
        elif ch[1]>0.5 and ch[0]<0.3: self.velocity = 3  # BREATHE
        elif len(self.memory)>=2:
            delta = sum(abs(self.memory[-1]["ch"][c]-self.memory[-2]["ch"][c]) for c in range(N_CH))
            if delta>1.0: self.velocity = 4  # UP
            elif delta<0.1 and sum(ch)<0.5: self.velocity = 5  # DOWN
            else: self.velocity = 0  # WALK
        else: self.velocity = 0
        return self.velocity

    def _vel_params(self):
        v = self.velocity
        if v==1: return 4, 0.5, 10    # RUN: short, low temp, wide K
        elif v==2: return 2, 1.0, 5   # STOP: sparse, high temp
        elif v==3: return 10, 0.8, 15 # BREATHE: long, warm
        elif v==4: return 8, 0.65, 12 # UP: escalating
        elif v==5: return 3, 0.9, 8   # DOWN: deflating
        return MAX_RESPONSE, GEN_TEMP, TOP_K  # WALK

    # ── RBA-1 + Schectman ──
    def _rba_update(self, disc):
        r = self.rba
        # R-Layer: entropy-based coherence
        total = sum(self.chambers)+1e-12
        probs = [c/total for c in self.chambers]
        entropy = -sum(p*math.log(p+1e-12) for p in probs)
        max_entropy = math.log(N_CH)
        local_coh = 1.0 - entropy/max_entropy if max_entropy>0 else 0
        r["coherence"] = max(0,min(1, 0.84*r["coherence"]+0.16*local_coh))
        # store in history for Ĉ(t)
        self.coherence_history[self.coh_ptr] = r["coherence"]
        self.coh_ptr = (self.coh_ptr+1)%COHERENCE_WINDOW
        # Ĉ(t): time-averaged recursive complexity
        r["c_hat"] = sum(self.coherence_history)/COHERENCE_WINDOW
        # Schectman equation
        planet_disc = planetary_dissonance()
        gamma_t = SCH_GAMMA0 + SCH_DELTA*disc + 0.15*planet_disc  # threshold from calendar + planetary
        eta_t = 1 + SCH_KAPPA*math.tanh(SCH_MU*r["coherence"])  # coupling
        exponent = SCH_LAMBDA*(r["c_hat"]-gamma_t)
        R_t = eta_t*SCH_ALPHA*(math.exp(max(-10,min(10,exponent)))-1) if exponent>-10 else 0
        # Φ-Layer: nudge toward dominant attractor
        dom = self._dominant()
        for c in range(N_CH):
            if c!=dom: self.chambers[c] += 0.01*(self.chambers[dom]-self.chambers[c])
        # Ψ-Layer: phase gate
        env_p = max(0,min(1, 0.34*disc+0.26*self.debt+0.10*self.chambers[5]+0.08*self.scar_total))
        threshold = max(0.25,min(0.88, 0.42+0.18*r["threshold_bias"]+0.06*self.scar_total))
        signal = max(0,min(1, 0.50*r["coherence"]+0.34*r["phase_lock"]+0.12*env_p+0.06*self.presence-0.08*self.trauma))
        r["psi"] = max(0,min(1, 0.5+1.35*(signal-threshold)))
        # deep mode with hysteresis
        if r["psi"]>0.3 and r["c_hat"]>gamma_t:
            r["deep_mode"]=True; r["deep_timer"]=5
        elif r["deep_timer"]>0: r["deep_timer"]-=1
        else: r["deep_mode"]=False
        # E-Layer: entropic buffer
        e_buffer = math.exp(-0.5*entropy)
        # M-Layer: sustained resonance
        r["sustained"] = max(0,min(1, 0.9*r["sustained"]+(0.1 if r["psi"]>0.3 else -0.05)))
        # phase lock
        target = max(0,min(1, 0.52*r["coherence"]+0.18*self.presence+0.14*env_p-0.08*self.scar_total))
        if target>=threshold: r["phase_lock"]=max(0,min(1,0.88*r["phase_lock"]+0.12*target))
        else: r["phase_lock"]=max(0,min(1,0.975*r["phase_lock"]+0.025*target))
        r["threshold_bias"]=max(0,min(1,0.93*r["threshold_bias"]+0.07*env_p))
        return R_t

    # ── Wormhole check ──
    def _wormhole_check(self, prompt):
        words = re.split(r'[\s.,!?;:"\'-]+', prompt.lower())
        for p in list(self.prophecies):
            for w in words:
                if word_similarity(w, str(p.get("text",""))) > 0.4:
                    self.rba["coherence"] = min(1, self.rba["coherence"]+0.15)
                    self.presence = min(1, self.presence+0.10)
                    self.wormhole_log.append({"step":self.interaction_count,"success":True})
                    self.prophecies.remove(p)
                    break

    # ── Experience consolidation ──
    def _consolidate(self):
        if self.interaction_count % CONSOLIDATION_INTERVAL != 0: return
        if self.scar_total>0:
            self.rba["threshold_bias"] = min(1, self.rba["threshold_bias"]+0.05*self.scar_total)
        if self.wormhole_log:
            rate = sum(1 for w in self.wormhole_log if w["success"])/max(len(self.wormhole_log),1)
            self.presence = min(1, self.presence+0.05*rate)

    # ── MetaKlaus ghost ──
    def _metaklaus(self, primary_lang):
        primary = self.lang_packs[primary_lang]
        nex = len(primary["exhale"])
        ghost = [0.0]*nex
        cn = math.sqrt(sum(v*v for v in self.chambers)+1e-12)
        if cn<1e-6: return ghost, 0
        dom = self._dominant()
        for code, lp in self.lang_packs.items():
            if code==primary_lang: continue
            key = f"{primary_lang}:{code}"
            ca = self.cross_aff.get(key)
            if not ca: continue
            wl = GHOST_WEIGHT[dom][GHOST_LANG.get(code,4)]
            if self.dark_active: wl *= 1.5
            # best match in other lang
            best_j, best_sim = 0, -1
            for j in range(len(lp["exhale"])):
                sim = sum(self.chambers[c]*lp["exhale"][j]["aff"][c] for c in range(N_CH))/cn
                if sim>best_sim: best_sim=sim; best_j=j
            # ghost with sensitivity tensor
            for w in range(nex):
                raw = ca["matrix"][w*ca["nb"]+best_j] if best_j<ca["nb"] else 0
                mod = sum(lp["exhale"][best_j]["aff"][g]*primary["exhale"][w]["aff"][p]*self.sensitivity[dom][g][p]
                         for g in range(N_CH) for p in range(N_CH))
                agreement = 0.6*raw + 0.4*mod
                ghost[w] += (agreement-0.5)*2*best_sim*wl
        no = len(self.lang_packs)-1
        if no>0: ghost = [g/no for g in ghost]
        interf = sum(abs(g) for g in ghost)/max(nex,1)
        return ghost, interf

    # ── Inhale ──
    def _inhale(self, lp, prompt):
        emotion=[0.0]*N_CH; matches=0
        self.matched_inhale_hashes = []
        words = re.split(r'[\s\t\n\r.,!?;:"\'\(\)\-]+', prompt.lower())
        for w in words:
            if not w: continue
            for iw in lp["inhale"]:
                if w==iw["text"] or iw["text"] in prompt:
                    for c in range(N_CH): emotion[c]+=iw["aff"][c]
                    self.matched_inhale_hashes.append(hash_word(iw["text"]) & 0xFFFFFFFF)
                    matches+=1; break
        if matches==0:
            for w in words:
                if not w: continue
                bs,bw = 0,None
                for iw in lp["inhale"]:
                    s=word_similarity(w,iw["text"])
                    if s>bs: bs=s;bw=iw
                if bw and bs>0.2:
                    for c in range(N_CH): emotion[c]+=bw["aff"][c]*bs
                    matches+=1
        if matches>0: emotion=[v/matches for v in emotion]
        else:
            h=hash_word(prompt)
            emotion=[((h>>(c*8))&0xFF)/255 for c in range(N_CH)]
        return emotion

    # ── Parliament vote ──
    def _parliament_vote(self, logits, nex, lp, temp):
        dom = max(range(N_CH), key=lambda c: self.chambers[c])
        # opposite: most negative coupling to dominant
        opp = min((c for c in range(N_CH) if c!=dom), key=lambda c: COUPLING[dom][c])
        votes = []
        for e in range(3):
            el = list(logits)
            for w in range(nex):
                if e == 0:   el[w] += lp["exhale"][w]["aff"][dom] * 0.8   # somatic
                elif e == 1: el[w] += lp["exhale"][w]["aff"][opp] * 0.6 - lp["exhale"][w]["aff"][dom] * 0.2  # shadow
                elif e == 2 and w < len(self.ghost_cache): el[w] += abs(self.ghost_cache[w]) * 0.5  # ghost
            votes.append(max(range(nex), key=lambda i: el[i]))
        if votes[0]==votes[1] or votes[0]==votes[2]: return votes[0]
        if votes[1]==votes[2]: return votes[1]
        # no consensus: somatic top-3 random
        el = list(logits)
        for w in range(nex): el[w] += lp["exhale"][w]["aff"][dom] * 0.3
        top3 = sorted(range(nex), key=lambda i: -el[i])[:3]
        mx = el[top3[0]]
        probs = [math.exp((el[i]-mx)/temp) for i in top3]
        s = sum(probs); r = rng.randf()*s; cum = 0
        for i in top3:
            cum += probs[top3.index(i)] if i in top3 else 0
            probs_i = math.exp((el[i]-mx)/temp)
            cum = 0
        # simplified: just pick from top3
        probs = [math.exp((el[i]-mx)/temp) for i in top3]
        s = sum(probs); r = rng.randf()*s; cum = 0
        for idx, i in enumerate(top3):
            cum += probs[idx]
            if cum >= r: return i
        return top3[0]

    # ── Exhale (Full Dario 7-force + Parliament) ──
    def _exhale(self, lang, ghost):
        lp=self.lang_packs[lang]; nex=len(lp["exhale"])
        if nex==0: return []
        self.ghost_cache = ghost  # for parliament ghost expert
        hebb=lp["meta"].hebbian(self.prev_exhale,nex)
        max_gen,temp,topk = self._vel_params()
        if self.rba.get("deep_mode"): max_gen = MAX_RESPONSE
        C = self.chambers
        # somatic coefficient modulation (from dario.c)
        alpha_mod = max(0.5,min(2.0, 1.0+0.3*C[1]-0.2*C[2]+0.1*C[4]))
        beta_mod  = max(0.5,min(2.0, 1.0+0.2*C[4]-0.3*C[0]))
        gamma_mod = max(0.5,min(2.0, 1.0+0.4*C[3]+0.2*C[5]-0.1*C[1]))
        tau_mod   = max(0.5,min(2.0, 1.0+0.5*C[4]-0.3*C[0]))
        eff_alpha = alpha_mod * DARIO_ALPHA
        eff_beta  = beta_mod * DARIO_BETA
        eff_gamma = gamma_mod * DARIO_GAMMA
        trauma_level = max(0,min(1, self.scar_total*0.5+C[0]*0.3))
        if trauma_level > 0.3: eff_gamma += trauma_level * 1.5
        v_tau = max(0.1, tau_mod * temp)
        res_gate = 1.0/(1.0+math.exp(-(self.rba["coherence"]*0.5+C[4]*0.3+0.2-0.5)*4))
        pp = sum(p.get("strength",0) for p in self.prophecies)/3
        scar_prop = 1.0 + self.scar_total * 0.3
        dark_mult = 1.5 if self.dark_active else 1.0
        result=[]; prev=self.prev_exhale[-1] if self.prev_exhale else -1
        local_used=set(self.used_exhale)
        for step in range(max_gen):
            logits=[0.0]*nex
            for w in range(nex):
                B = (lp["meta"].bigram(prev,w)*BIGRAM_BASE) if prev>=0 else 0
                H = eff_alpha*hebb[w]*(1+res_gate)
                aff = lp["exhale"][w]["aff"]
                aff_norm = math.sqrt(sum(a*a for a in aff)) or 1e-6
                soma = sum(C[c]*aff[c] for c in range(N_CH)) / aff_norm
                F = eff_beta*soma*0.5*scar_prop if pp>0.3 else 0
                dest_sim = sum(self.destiny[c]*lp["exhale"][w]["aff"][c] for c in range(N_CH))
                A = eff_gamma * dest_sim
                V = DARIO_DELTA * 0  # RRPRAM simplified in Python
                ghost_val = max(-1.0, min(1.0, ghost[w] if w<len(ghost) else 0))
                G = DARIO_ZETA * ghost_val * dark_mult
                T = sum(self.scars[c]*lp["exhale"][w]["aff"][c]*0.5 for c in range(N_CH))
                K = self.kk.force_k(w, C) if self.kk else 0
                total = B + H + F + A + V + G + T + K + soma
                if step==0 and prev<0: total = soma + 0.1*G  # pure somatic start
                logits[w] = total / v_tau
                if str(w) in local_used or lp["exhale"][w]["text"] in local_used:
                    logits[w] -= 100
            # spore boost
            self._spore_boost(lang, logits, nex)
            # parliament vote
            chosen = self._parliament_vote(logits, nex, lp, temp)
            result.append(chosen)
            local_used.add(str(chosen)); local_used.add(lp["exhale"][chosen]["text"])
            # destiny from CHAMBERS, not words (personality)
            for c in range(N_CH):
                self.destiny[c] = 0.1*C[c] + 0.9*self.destiny[c]
            prev=chosen
            if step>2:
                sc=sum(C[c]*lp["exhale"][chosen]["aff"][c] for c in range(N_CH))
                if sc<0.1: break
        self.prev_exhale=result[-4:]
        for w in result: self.used_exhale.add(str(w)); self.used_exhale.add(lp["exhale"][w]["text"])
        # NOTORCH: learn inhale→exhale spore pairs
        for g in result:
            for mh in self.matched_inhale_hashes:
                self._spore_learn(mh, g, lang)
        return [lp["exhale"][i]["text"] for i in result]

    # ── Spore system (NOTORCH) ──
    def _spore_learn(self, inhale_hash, exhale_idx, lang):
        for p in self.spore_pairs:
            if p["ih"]==inhale_hash and p["ei"]==exhale_idx and p["lang"]==lang:
                p["str"] += SPORE_LEARN_RATE; p["hits"] += 1; return
        if len(self.spore_pairs) < MAX_SPORE_PAIRS:
            self.spore_pairs.append({"ih":inhale_hash,"ei":exhale_idx,"str":SPORE_LEARN_RATE,"lang":lang,"hits":1,"ch":list(self.chambers)})
        else:
            weakest = min(range(len(self.spore_pairs)), key=lambda i: self.spore_pairs[i]["str"])
            self.spore_pairs[weakest] = {"ih":inhale_hash,"ei":exhale_idx,"str":SPORE_LEARN_RATE,"lang":lang,"hits":1,"ch":list(self.chambers)}

    def _spore_boost(self, lang, logits, nex):
        for p in self.spore_pairs:
            if p["lang"]!=lang or p["ei"]>=nex: continue
            for mh in self.matched_inhale_hashes:
                if mh == p["ih"]:
                    logits[p["ei"]] += p["str"]*0.5; break

    def _spore_decay(self):
        self.spore_pairs = [p for p in self.spore_pairs if (p.__setitem__("str",p["str"]*SPORE_DECAY) or True) and p["str"]>0.001]

    def _spore_save(self):
        try:
            sp = os.path.join(self.base_dir, SPORE_FILE)
            with open(sp,"wb") as f:
                f.write(struct.pack("<I",SPORE_MAGIC))
                f.write(struct.pack("<I",len(self.spore_pairs)))
                for p in self.spore_pairs:
                    f.write(struct.pack("<IIfHH",p["ih"],p["ei"],p["str"],0,p["hits"]))
        except: pass

    def _spore_load(self):
        sp = os.path.join(self.base_dir, SPORE_FILE)
        if not os.path.exists(sp): return False
        try:
            with open(sp,"rb") as f:
                magic = struct.unpack("<I",f.read(4))[0]
                if magic != SPORE_MAGIC: return False
                n = struct.unpack("<I",f.read(4))[0]
                for _ in range(n):
                    ih,ei,st,_,hits = struct.unpack("<IIfHH",f.read(16))
                    self.spore_pairs.append({"ih":ih,"ei":ei,"str":st,"lang":"en","hits":hits,"ch":[0]*N_CH})
            print(f"[klaus] spores loaded: {len(self.spore_pairs)} pairs")
            return True
        except: return False

    # ── Meta-recursion ──
    def _meta_recurse(self, lang, words):
        if not words: return
        meta_prompt = ". ".join(words)
        lp = self.lang_packs[lang]
        meta_emotion = self._inhale(lp, meta_prompt)
        meta_mlp = self.mlp.forward(meta_emotion + [0]*6 + [0])
        for c in range(N_CH):
            self.chambers[c] = (1-META_BLEND)*self.chambers[c] + META_BLEND*meta_mlp[c]

    # ── Somatic persistence ──
    def _soma_save(self):
        try:
            with open(os.path.join(self.base_dir, SOMA_FILE),"wb") as f:
                f.write(struct.pack("<I", SOMA_MAGIC))
                for c in range(N_CH): f.write(struct.pack("<f", self.chambers[c]))
                for c in range(N_CH): f.write(struct.pack("<f", self.soma[c]))
                for c in range(N_CH): f.write(struct.pack("<f", self.scars[c]))
                f.write(struct.pack("<f", self.presence))
                f.write(struct.pack("<f", self.rba["coherence"]))
                f.write(struct.pack("<f", self.rba["c_hat"]))
                f.write(struct.pack("<f", self.rba["psi"]))
                f.write(struct.pack("<f", self.rba["sustained"]))
                f.write(struct.pack("<f", self.rba["phase_lock"]))
                f.write(struct.pack("<f", self.rba["threshold_bias"]))
                f.write(struct.pack("<I", self.interaction_count))
                for v in self.coherence_history: f.write(struct.pack("<f", v))
        except: pass

    def _soma_load(self):
        sp = os.path.join(self.base_dir, SOMA_FILE)
        if not os.path.exists(sp):
            print("[klaus] fresh soma — no prior memory.")
            return
        try:
            with open(sp,"rb") as f:
                magic = struct.unpack("<I", f.read(4))[0]
                if magic!=SOMA_MAGIC: print("[klaus] bad soma file"); return
                for c in range(N_CH): self.chambers[c]=struct.unpack("<f",f.read(4))[0]
                for c in range(N_CH): self.soma[c]=struct.unpack("<f",f.read(4))[0]
                for c in range(N_CH): self.scars[c]=struct.unpack("<f",f.read(4))[0]
                self.presence=struct.unpack("<f",f.read(4))[0]
                self.rba["coherence"]=struct.unpack("<f",f.read(4))[0]
                self.rba["c_hat"]=struct.unpack("<f",f.read(4))[0]
                self.rba["psi"]=struct.unpack("<f",f.read(4))[0]
                self.rba["sustained"]=struct.unpack("<f",f.read(4))[0]
                self.rba["phase_lock"]=struct.unpack("<f",f.read(4))[0]
                self.rba["threshold_bias"]=struct.unpack("<f",f.read(4))[0]
                self.interaction_count=struct.unpack("<I",f.read(4))[0]
                for i in range(COHERENCE_WINDOW):
                    self.coherence_history[i]=struct.unpack("<f",f.read(4))[0]
                self.scar_total = sum(self.scars)
                print(f"[klaus] soma loaded: {self.interaction_count} prior interactions, scars:{self.scar_total:.2f}")
        except Exception as e:
            print(f"[klaus] soma load failed: {e}")

    # ── Process ──
    def process(self, prompt):
        self.interaction_count += 1
        lang = self._detect_lang(prompt)
        lp = self.lang_packs[lang]
        dark = self._detect_dark(prompt)
        self._wormhole_check(prompt)
        emotion = self._inhale(lp, prompt)
        if self.kk: emotion = self.kk.blend_mood(emotion)
        mem_state = self._mem_blend()
        disc = self._calendar()
        mlp_in = emotion + mem_state + [disc]
        mlp_out = self.mlp.forward(mlp_in)
        # crossfire on RESIDUAL state first (decays old energy)
        self._crossfire(XFIRE_ITERS)
        # inject emotion AFTER crossfire — new signal is NOT decayed
        for c in range(N_CH):
            mixed = 0.4*emotion[c]+0.3*mlp_out[c]+0.2*mem_state[c]+0.1*self.soma[c]
            self.chambers[c] = max(0,min(1, mixed))
            for s in range(N_SUB):
                self.sub[c][s][0] = max(0,min(1, self.sub[c][s][0]+mixed))
        vel = self._velocity_detect()
        R_t = self._rba_update(disc)
        dom = self._dominant()
        prem = self._prophetic_prem(disc)
        is_prophetic = disc>0.3 and len(self.memory)>=2
        ghost, ghost_interf = self._metaklaus(lang)
        if self.kk: self.kk.compute_signal(self.chambers, lang, lp["exhale"])
        words = self._exhale(lang, ghost)
        # meta-recursion
        self._meta_recurse(lang, words)
        self._scars_update()
        self._mem_store(disc)
        # tick prophecies
        self.prophecies = [p for p in self.prophecies
                          if (p.__setitem__("age",p.get("age",0)+1) or True) and
                          (p.__setitem__("strength",p.get("strength",1)*0.95) or True) and
                          p.get("age",0)<20 and p.get("strength",0)>0.01] if self.prophecies else []
        self._spore_decay()
        self._spore_save()
        self._consolidate()
        self._soma_save()
        return {"lang":lang,"words":words,"chambers":list(self.chambers),"dominant":CH_NAMES[dom],
                "prem":prem,"disc":disc,"ghost_strength":ghost_interf,"is_prophetic":is_prophetic,
                "velocity":VEL_NAMES[vel],"vel_mult":self._vel_params()[0]/MAX_RESPONSE,
                "dark":dark,"scars":list(self.scars),"scar_total":self.scar_total,
                "rba":dict(self.rba),"meta_depth":1}

    def print_response(self, r):
        ch_str = " ".join(f"{CH_NAMES[c]}:{r['chambers'][c]:.2f}" for c in range(N_CH))
        print(f"  [{ch_str}]")
        print(f"  {{{r['velocity']} x{r['vel_mult']:.2f}}}")
        print(f"  {'. '.join(r['words'])}.")
        ghost_str = f"metaklaus: {r['dominant']}-dominant, interference {r['ghost_strength']:.2f}"
        if r["dark"]: ghost_str += ", DARK MATTER x1.5"
        print(f"  ({ghost_str})")
        rba = r["rba"]
        print(f"  [RBA-1 coherence:{rba['coherence']:.3f} C-hat:{rba['c_hat']:.3f} psi:{rba['psi']:.3f} sustained:{rba['sustained']:.3f} meta-depth:{r['meta_depth']}]")
        if r["scar_total"]>0.01:
            sc = " ".join(f"{CH_NAMES[c]}:{r['scars'][c]:.2f}" for c in range(N_CH) if r['scars'][c]>0.01)
            print(f"  [scars: {sc} total:{r['scar_total']:.2f}]")
        if r["is_prophetic"]:
            dp = r["prem"].index(max(r["prem"]))
            print(f"  ~premonition~ [→{CH_NAMES[dp]}:{r['prem'][dp]:.2f} dissonance:{r['disc']:.2f}]")

    def interactive(self):
        while True:
            try: prompt=input("klaus> ")
            except (EOFError,KeyboardInterrupt): print("\n[klaus] exhale. goodbye."); break
            prompt=prompt.strip()
            if not prompt: continue
            if prompt in ("exit","quit","q"): print("[klaus] exhale. goodbye."); break
            if prompt=="status":
                print(f"  languages: {list(self.lang_packs.keys())}")
                print(f"  memory: {len(self.memory)}/{MEM_SLOTS}")
                ch_str=" ".join(f"{CH_NAMES[c]}:{self.chambers[c]:.2f}" for c in range(N_CH))
                print(f"  chambers: {ch_str}")
                print(f"  scars: {self.scar_total:.3f}")
                print(f"  RBA-1: coh={self.rba['coherence']:.3f} C-hat={self.rba['c_hat']:.3f} psi={self.rba['psi']:.3f}")
                print(f"  interactions: {self.interaction_count}")
                continue
            if prompt=="reset":
                self.chambers=[0.0]*N_CH; self.chambers[1]=0.15; self.chambers[4]=0.10
                self.memory=[]; self.used_exhale=set(); self.prev_exhale=[]
                self.prophecies=[]; self.scars=[0.0]*N_CH; self.scar_total=0
                self.rba={"coherence":0,"c_hat":0,"psi":0,"sustained":0,"phase_lock":0,"threshold_bias":0,"deep_mode":False,"deep_timer":0}
                self.coherence_history=[0]*COHERENCE_WINDOW; self.interaction_count=0
                sp=os.path.join(self.base_dir,SOMA_FILE)
                if os.path.exists(sp): os.remove(sp)
                print("  [reset]"); continue
            r=self.process(prompt); self.print_response(r)

def main():
    base_dir="."; single=None; args=sys.argv[1:]; i=0
    while i<len(args):
        if args[i]=="--dir" and i+1<len(args): base_dir=args[i+1];i+=2
        elif args[i]=="--help":
            print(f"KLAUS v{KLAUS_VERSION}"); print("python3 klaus.py [--dir DIR] [PROMPT]"); return
        elif not args[i].startswith("-"): single=args[i];i+=1
        else: i+=1
    k=Klaus(base_dir)
    if not k.init(): sys.exit(1)
    if single: r=k.process(single); k.print_response(r)
    else: k.interactive()

if __name__=="__main__": main()
