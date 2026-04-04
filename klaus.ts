#!/usr/bin/env npx ts-node
/*
 * klaus.ts — KLAUS: Kinetic Linguistic Adaptive Unified Sonar
 * TypeScript single-file inference. Zero dependencies beyond Node.js.
 * Identical architecture to klaus.c v1.1.0.
 *
 * Run:  npx ts-node klaus.ts                    # interactive
 *       npx ts-node klaus.ts "I am terrified"   # single shot
 *       npx ts-node klaus.ts --dir /path        # custom dir
 *
 * Or compile + run:
 *       tsc klaus.ts --target es2020 --module commonjs
 *       node klaus.js "мне страшно"
 *
 * No dependencies. No bullshit. The body speaks TypeScript too.
 *
 * (c) 2026 arianna method. resonance is unbreakable.
 */

import * as fs from "fs";
import * as path from "path";
import * as readline from "readline";

// ═══════════════════════════════════════════════════
// CONFIG
// ═══════════════════════════════════════════════════

const KLAUS_VERSION = "2.0.0";
const N_CH = 6;
const N_SUB = 4;
const CH_NAMES = ["FEAR", "LOVE", "RAGE", "VOID", "FLOW", "CMPLX"] as const;
const XFIRE_ITERS = 8;
const MEM_SLOTS = 32;
const MEM_DECAY = 0.85;
const MAX_RESPONSE = 12;
const GEN_TEMP = 0.75;
const TOP_K = 20;

// Full Dario 7-force
const DARIO_ALPHA = 0.30, DARIO_BETA = 0.15, DARIO_GAMMA = 0.25;
const DARIO_DELTA = 0.20, DARIO_ZETA = 0.35, BIGRAM_BASE = 1.0;

// Planetary
const ORBITAL_PERIOD = [87.97,224.70,365.25,686.97,4332.59,10759.22];
const J2000_LONGITUDE = [252.25,181.98,100.46,355.45,34.40,49.94];
const J2000_EPOCH_MS = new Date(2000,0,1,12,0,0).getTime();

const ANNUAL_DRIFT = 11.25;
const GREGORIAN_YEAR = 365.25;
const METONIC_LEAP = [3, 6, 8, 11, 14, 17, 19];
const MAX_UNCORRECTED = 33.0;
const EPOCH_MS = new Date(2024, 9, 3, 12, 0, 0).getTime();

const CH_DECAY: readonly number[] = [0.90, 0.93, 0.85, 0.97, 0.88, 0.94];

const COUPLING: readonly (readonly number[])[] = [
  [0.0, -0.3, 0.5, 0.4, -0.2, 0.1],
  [-0.3, 0.0, -0.4, -0.5, 0.5, 0.2],
  [0.5, -0.3, 0.0, 0.2, -0.3, 0.3],
  [0.4, -0.5, 0.3, 0.0, -0.3, 0.4],
  [-0.2, 0.4, -0.2, -0.3, 0.0, 0.3],
  [0.1, 0.2, 0.3, 0.4, 0.3, 0.0],
];

// ═══════════════════════════════════════════════════
// STATE-DEPENDENT GHOST WEIGHTS (from metaklaus.jl)
// [dominant_chamber][lang_idx] → weight
// lang indices: 0=en, 1=ru, 2=fr, 3=he, 4=other
// ═══════════════════════════════════════════════════

const GHOST_WEIGHT: readonly (readonly number[])[] = [
  /* FEAR */ [1.0, 1.2, 0.9, 1.8, 1.0],
  /* LOVE */ [1.0, 1.1, 1.7, 1.4, 1.0],
  /* RAGE */ [1.0, 1.8, 0.8, 1.3, 1.0],
  /* VOID */ [0.9, 1.5, 1.0, 1.6, 1.0],
  /* FLOW */ [1.0, 0.9, 1.5, 1.4, 1.0],
  /* CMPLX */ [1.0, 1.1, 1.2, 1.7, 1.0],
];

function ghostLangIdx(code: string): number {
  switch (code) {
    case "en": return 0;
    case "ru": return 1;
    case "fr": return 2;
    case "he": return 3;
    default: return 4;
  }
}

// Sub-chamber natural frequencies
const SUB_FREQ: readonly (readonly number[])[] = [
  /* FEAR */ [0.3, 1.2, 0.6, 0.9],
  /* LOVE */ [0.4, 0.6, 0.5, 0.3],
  /* RAGE */ [0.8, 1.5, 0.9, 1.1],
  /* VOID */ [0.1, 0.2, 0.15, 0.08],
  /* FLOW */ [0.5, 0.7, 0.6, 0.4],
  /* CMPLX */ [0.35, 0.55, 0.45, 0.65],
];

const INTRA_COUPLING: readonly (readonly number[])[] = [
  [0.0, 0.3, 0.2, 0.1],
  [0.3, 0.0, 0.15, -0.1],
  [0.2, 0.15, 0.0, 0.1],
  [0.1, -0.1, 0.1, 0.0],
];

// ═══════════════════════════════════════════════════
// TYPES
// ═══════════════════════════════════════════════════

interface Word {
  text: string;
  aff: number[]; // [6]
}

interface MetaWeights {
  bigrams: Map<string, number>;
  hebbs: Map<string, number>;
}

interface LangPack {
  code: string;
  inhale: Word[];
  exhale: Word[];
  meta: MetaWeights;
  bpeMerges: [number, number, number][];
}

interface SubChamber {
  act: number;
  phase: number;
  freq: number;
}

interface Chambers {
  sub: SubChamber[][]; // [6][4]
  act: number[];       // [6] primary (mean of subs)
  soma: number[];      // [6]
  presence: number;
  trauma: number;
  debt: number;
}

interface SomaSlot {
  ch: number[];
  valence: number;
  arousal: number;
  cal: number;
}

interface CrossAffinity {
  matrix: Float32Array; // n_a * n_b flat
  n_a: number;
  n_b: number;
  langA: string;
  langB: string;
}

interface Prophecy {
  target: number;
  strength: number;
  age: number;
}

interface KlausResponse {
  lang: string;
  words: string[];
  chambers: number[];
  dominant: string;
  prem: number[];
  disc: number;
  ghostStrength: number;
  isProphetic: boolean;
}

// ═══════════════════════════════════════════════════
// RNG — xorshift64
// ═══════════════════════════════════════════════════

let rngState = BigInt(Date.now());

function rngNext(): bigint {
  rngState ^= rngState << 13n;
  rngState &= 0xFFFFFFFFFFFFFFFFn;
  rngState ^= rngState >> 7n;
  rngState ^= rngState << 17n;
  rngState &= 0xFFFFFFFFFFFFFFFFn;
  return rngState;
}

function randf(): number {
  return Number(rngNext() & 0x7FFFFFFFn) / 0x7fffffff;
}

function randn(std: number): number {
  const u1 = randf() + 1e-10;
  const u2 = randf();
  return std * Math.sqrt(-2 * Math.log(u1)) * Math.cos(2 * Math.PI * u2);
}

// ═══════════════════════════════════════════════════
// HASH
// ═══════════════════════════════════════════════════

function hashWord(w: string): bigint {
  let h = 0xcbf29ce484222325n;
  for (let i = 0; i < w.length; i++) {
    h ^= BigInt(w.charCodeAt(i));
    h = (h * 0x100000001b3n) & 0xFFFFFFFFFFFFFFFFn;
  }
  return h;
}

function wordSimilarity(a: string, b: string): number {
  if (a.length < 3 || b.length < 3) return a === b ? 1.0 : 0.0;
  let matches = 0;
  const total = a.length - 2;
  for (let i = 0; i <= a.length - 3; i++) {
    const tri = a.substring(i, i + 3);
    if (b.includes(tri)) matches++;
  }
  return total > 0 ? matches / total : 0;
}

// ═══════════════════════════════════════════════════
// ANCHORS
// ═══════════════════════════════════════════════════

const ANCHORS: Record<string, Record<number, string[]>> = {
  en: {
    0: ["fear", "terror", "panic", "dread", "horror", "nightmare", "anxiety", "threat", "danger", "trapped"],
    1: ["love", "warmth", "tenderness", "kindness", "compassion", "affection", "care", "embrace", "comfort", "gentle"],
    2: ["rage", "fury", "anger", "burning", "explosive", "violent", "hostile", "aggressive", "bitter", "cruel"],
    3: ["empty", "hollow", "numb", "void", "darkness", "silence", "lonely", "abandoned", "despair", "hopeless"],
    4: ["flow", "rhythm", "dance", "pulse", "harmony", "resonance", "vibration", "wave", "music", "breath"],
    5: ["paradox", "mystery", "chaos", "tension", "complex", "uncertain", "transform", "strange", "enigma", "spiral"],
  },
  ru: {
    0: ["страшно", "ужас", "паника", "тревога", "кошмар"],
    1: ["любовь", "тепло", "нежность", "забота"],
    2: ["ярость", "гнев", "злость", "бесит"],
    3: ["пустота", "тишина", "одиночество"],
    4: ["поток", "ритм", "гармония"],
    5: ["хаос", "тайна", "парадокс"],
  },
  fr: {
    0: ["peur", "terreur", "panique", "horreur", "angoisse"],
    1: ["amour", "chaleur", "tendresse", "douceur"],
    2: ["rage", "fureur", "colère", "violence"],
    3: ["vide", "silence", "solitude", "désespoir"],
    4: ["flux", "rythme", "harmonie", "danse"],
    5: ["paradoxe", "mystère", "chaos", "tension"],
  },
  he: {
    0: ["פחד", "אימה", "חרדה"],
    1: ["אהבה", "חום", "רוך"],
    2: ["זעם", "כעס"],
    3: ["ריקנות", "בדידות", "שתיקה"],
    4: ["קצב", "הרמוניה"],
    5: ["מסתורין", "כאוס"],
  },
};

function computeAffinity(word: string, langCode: string): number[] {
  const aff = new Array(N_CH).fill(0);
  const anc = ANCHORS[langCode] || ANCHORS["en"];
  for (let c = 0; c < N_CH; c++) {
    if (anc[c]?.includes(word)) {
      aff[c] = 1.0;
      for (let j = 0; j < N_CH; j++) {
        if (j !== c) aff[j] += 0.1 * Math.abs(COUPLING[c][j]);
      }
      const mx = Math.max(...aff);
      if (mx > 0) for (let i = 0; i < N_CH; i++) aff[i] /= mx;
      return aff;
    }
  }
  const h = hashWord(word);
  for (let c = 0; c < N_CH; c++) {
    aff[c] = Number((h >> BigInt(c * 8)) & 0xFFn) / 255;
  }
  const mx = Math.max(...aff);
  if (mx > 0) for (let i = 0; i < N_CH; i++) aff[i] /= mx;
  return aff;
}

// ═══════════════════════════════════════════════════
// BPE
// ═══════════════════════════════════════════════════

function bpeLearn(data: string, numMerges: number): { merges: [number, number, number][]; tokens: number[] } {
  let tokens = Array.from(Buffer.from(data, "utf-8"));
  const merges: [number, number, number][] = [];
  for (let m = 0; m < numMerges; m++) {
    const pairs = new Map<string, number>();
    for (let i = 0; i < tokens.length - 1; i++) {
      const key = `${tokens[i]},${tokens[i + 1]}`;
      pairs.set(key, (pairs.get(key) || 0) + 1);
    }
    let bestKey = "";
    let bestCount = 1;
    for (const [k, v] of pairs) {
      if (v > bestCount) { bestCount = v; bestKey = k; }
    }
    if (!bestKey) break;
    const [a, b] = bestKey.split(",").map(Number);
    const newId = 256 + m;
    merges.push([a, b, newId]);
    const newTokens: number[] = [];
    let i = 0;
    while (i < tokens.length) {
      if (i < tokens.length - 1 && tokens[i] === a && tokens[i + 1] === b) {
        newTokens.push(newId); i += 2;
      } else { newTokens.push(tokens[i]); i++; }
    }
    tokens = newTokens;
  }
  return { merges, tokens };
}

function bpeEncode(data: string, merges: [number, number, number][]): number[] {
  let tokens = Array.from(Buffer.from(data, "utf-8"));
  for (const [a, b, newId] of merges) {
    const newTokens: number[] = [];
    let i = 0;
    while (i < tokens.length) {
      if (i < tokens.length - 1 && tokens[i] === a && tokens[i + 1] === b) {
        newTokens.push(newId); i += 2;
      } else { newTokens.push(tokens[i]); i++; }
    }
    tokens = newTokens;
  }
  return tokens;
}

// ═══════════════════════════════════════════════════
// METAWEIGHTS
// ═══════════════════════════════════════════════════

function metaBuild(ids: number[]): MetaWeights {
  const bigrams = new Map<string, number>();
  const aTotals = new Map<number, number>();
  for (let i = 0; i < ids.length - 1; i++) {
    const key = `${ids[i]},${ids[i + 1]}`;
    bigrams.set(key, (bigrams.get(key) || 0) + 1);
    aTotals.set(ids[i], (aTotals.get(ids[i]) || 0) + 1);
  }
  for (const [k, v] of bigrams) {
    const a = Number(k.split(",")[0]);
    const total = aTotals.get(a) || 1;
    bigrams.set(k, v / total);
  }
  const hebbMap = new Map<string, number>();
  const hn = Math.min(ids.length, 3000);
  const win = 5;
  for (let i = 0; i < hn; i++) {
    for (let j = Math.max(0, i - win); j <= Math.min(hn - 1, i + win); j++) {
      if (i === j) continue;
      const ka = Math.min(ids[i], ids[j]);
      const kb = Math.max(ids[i], ids[j]);
      const key = `${ka},${kb}`;
      const decay = 1 / (1 + Math.abs(i - j));
      hebbMap.set(key, (hebbMap.get(key) || 0) + decay);
    }
  }
  let maxH = 0;
  for (const v of hebbMap.values()) if (v > maxH) maxH = v;
  if (maxH > 0) for (const [k, v] of hebbMap) hebbMap.set(k, v / maxH);
  return { bigrams, hebbs: hebbMap };
}

function metaBigram(meta: MetaWeights, prev: number, next: number): number {
  return meta.bigrams.get(`${prev},${next}`) || 1e-10;
}

function metaHebbian(meta: MetaWeights, ctx: number[], V: number): number[] {
  const out = new Array(V).fill(0);
  for (const c of ctx) {
    for (const [k, s] of meta.hebbs) {
      const [a, b] = k.split(",").map(Number);
      if (a === c && b < V) out[b] += s;
      else if (b === c && a < V) out[a] += s;
    }
  }
  const mx = Math.max(...out);
  if (mx > 0) for (let i = 0; i < V; i++) out[i] /= mx;
  return out;
}

// ═══════════════════════════════════════════════════
// SENSITIVITY TENSOR 6×6×6
// ═══════════════════════════════════════════════════

function buildSensitivity(): number[][][] {
  const S: number[][][] = Array.from({ length: N_CH }, () =>
    Array.from({ length: N_CH }, () => new Array(N_CH).fill(0))
  );
  for (let d = 0; d < N_CH; d++) {
    for (let g = 0; g < N_CH; g++) {
      for (let p = 0; p < N_CH; p++) {
        const base = Math.abs(COUPLING[g][p]);
        if (g === d) S[d][g][p] = base * 2.0;
        else if (COUPLING[d][g] > 0.3) S[d][g][p] = base * 1.5;
        else if (COUPLING[d][g] < -0.3) S[d][g][p] = base * 0.5;
        else S[d][g][p] = base;
      }
    }
  }
  return S;
}

// ═══════════════════════════════════════════════════
// MLP
// ═══════════════════════════════════════════════════

interface MLP { w1: number[]; b1: number[]; w2: number[]; b2: number[]; w3: number[]; b3: number[]; }

function swish(x: number): number { return x / (1 + Math.exp(-x)); }
function sigmoid(x: number): number { return 1 / (1 + Math.exp(-Math.max(-500, Math.min(500, x)))); }

function initMLP(langPacks: Map<string, LangPack>): MLP {
  let seed = 0xcbf29ce484222325n;
  for (const lp of langPacks.values()) {
    for (const w of lp.inhale) seed ^= hashWord(w.text);
  }
  rngState = seed;
  const s1 = Math.sqrt(2 / 13), s2 = Math.sqrt(2 / 32), s3 = Math.sqrt(2 / 16);
  const w1 = Array.from({ length: 13 * 32 }, () => randn(s1));
  const b1 = new Array(32).fill(0);
  const w2 = Array.from({ length: 32 * 16 }, () => randn(s2));
  const b2 = new Array(16).fill(0);
  const w3 = Array.from({ length: 16 * 6 }, () => randn(s3));
  const b3 = new Array(6).fill(0);
  rngState = BigInt(Date.now());
  return { w1, b1, w2, b2, w3, b3 };
}

function mlpForward(mlp: MLP, input: number[]): number[] {
  const h1 = new Array(32);
  for (let i = 0; i < 32; i++) {
    let v = mlp.b1[i];
    for (let j = 0; j < 13; j++) v += input[j] * mlp.w1[j * 32 + i];
    h1[i] = swish(v);
  }
  const h2 = new Array(16);
  for (let i = 0; i < 16; i++) {
    let v = mlp.b2[i];
    for (let j = 0; j < 32; j++) v += h1[j] * mlp.w2[j * 16 + i];
    h2[i] = swish(v);
  }
  const out = new Array(6);
  for (let i = 0; i < 6; i++) {
    let v = mlp.b3[i];
    for (let j = 0; j < 16; j++) v += h2[j] * mlp.w3[j * 6 + i];
    out[i] = sigmoid(v);
  }
  return out;
}

// ═══════════════════════════════════════════════════
// HYPER-KURAMOTO — 24 oscillators
// ═══════════════════════════════════════════════════

function chambersInit(): Chambers {
  const sub: SubChamber[][] = [];
  for (let i = 0; i < N_CH; i++) {
    sub[i] = [];
    for (let s = 0; s < N_SUB; s++) {
      const initAct = (i === 1 ? 0.15 : i === 4 ? 0.10 : 0.0) / N_SUB;
      sub[i][s] = { act: initAct, phase: (i * 1.047 + s * 0.262), freq: SUB_FREQ[i][s] };
    }
  }
  return {
    sub,
    act: new Array(N_CH).fill(0),
    soma: new Array(N_CH).fill(0),
    presence: 0, trauma: 0, debt: 0,
  };
}

function chambersCrossfire(ch: Chambers, iters: number): void {
  for (let t = 0; t < iters; t++) {
    const old = ch.sub.map(row => row.map(s => ({ ...s })));
    for (let i = 0; i < N_CH; i++) {
      for (let si = 0; si < N_SUB; si++) {
        let dphase = old[i][si].freq;
        // intra-coupling
        for (let sj = 0; sj < N_SUB; sj++) {
          if (si === sj) continue;
          dphase += INTRA_COUPLING[si][sj] * Math.sin(old[i][sj].phase - old[i][si].phase);
        }
        // inter-coupling (mean phase of other primary)
        for (let j = 0; j < N_CH; j++) {
          if (i === j) continue;
          let meanPhase = 0;
          for (let s = 0; s < N_SUB; s++) meanPhase += old[j][s].phase;
          meanPhase /= N_SUB;
          dphase += COUPLING[i][j] * Math.sin(meanPhase - old[i][si].phase) * 0.03;
        }
        const newAct = Math.max(0, Math.min(1,
          old[i][si].act * CH_DECAY[i] + 0.03 * Math.sin(dphase) * 0.1));
        ch.sub[i][si] = { act: newAct, phase: old[i][si].phase + dphase * 0.1, freq: old[i][si].freq };
      }
    }
    // collapse to primary
    for (let i = 0; i < N_CH; i++) {
      let sum = 0;
      for (let s = 0; s < N_SUB; s++) sum += ch.sub[i][s].act;
      ch.act[i] = sum / N_SUB;
      ch.soma[i] = Math.max(0, Math.min(1, 0.92 * ch.soma[i] + 0.08 * ch.act[i]));
    }
    ch.presence = Math.max(0, Math.min(1,
      0.9 * ch.presence + 0.05 * (1 - ch.act[3]) * ch.act[4] + 0.03 * ch.soma[1]));
    ch.trauma *= 0.98;
    ch.debt *= 0.97;
  }
}

function dominant(ch: Chambers): number {
  let best = 0;
  for (let c = 1; c < N_CH; c++) if (ch.act[c] > ch.act[best]) best = c;
  return best;
}

// ═══════════════════════════════════════════════════
// LANGUAGE DETECTION
// ═══════════════════════════════════════════════════

function detectLanguage(text: string, langs: Map<string, LangPack>): string {
  let cyrillic = 0, hebrew = 0, accented = 0;
  for (let i = 0; i < text.length; i++) {
    const c = text.charCodeAt(i);
    if (c >= 0x0400 && c <= 0x04ff) cyrillic++;
    else if (c >= 0x0590 && c <= 0x05ff) hebrew++;
    else if (c >= 0x00c0 && c <= 0x00ff) accented++;
  }
  if (hebrew > 2 && langs.has("he")) return "he";
  if (cyrillic > 2 && langs.has("ru")) return "ru";
  if (accented > 1 && langs.has("fr")) return "fr";
  const frHints = ["je ", "tu ", "le ", "la ", "les ", "suis ", "est ", "dans "];
  for (const fw of frHints) if (text.includes(fw) && langs.has("fr")) return "fr";
  if (langs.has("en")) return "en";
  return langs.keys().next().value!;
}

// ═══════════════════════════════════════════════════
// CALENDAR
// ═══════════════════════════════════════════════════

function planetaryDissonance(): number {
  const days = (Date.now() - J2000_EPOCH_MS) / 86400000;
  let cosSum = 0, sinSum = 0;
  for (let i = 0; i < 6; i++) {
    const theta = ((J2000_LONGITUDE[i] + 360 * (days / ORBITAL_PERIOD[i])) % 360) * Math.PI / 180;
    cosSum += Math.cos(theta); sinSum += Math.sin(theta);
  }
  cosSum /= 6; sinSum /= 6;
  return Math.max(0, Math.min(1, 1 - Math.sqrt(cosSum*cosSum + sinSum*sinSum)));
}

function calendarDissonance(): number {
  const days = (Date.now() - EPOCH_MS) / 86400000;
  const years = days / GREGORIAN_YEAR;
  const baseDrift = years * ANNUAL_DRIFT;
  const fullCycles = Math.floor(years / 19);
  let corrections = fullCycles * 7 * 30;
  const partial = years % 19;
  const yic = Math.floor(partial) + 1;
  for (const ly of METONIC_LEAP) if (ly <= yic) corrections += 30;
  const drift = baseDrift - corrections;
  const raw = Math.abs(drift % MAX_UNCORRECTED) / MAX_UNCORRECTED;
  return Math.max(0, Math.min(1, raw));
}

function propheticPremonition(ch: Chambers, disc: number, memory: SomaSlot[]): number[] {
  const prem = new Array(N_CH).fill(0);
  if (disc < 0.3 || memory.length < 2) return prem;
  const m0 = memory[memory.length - 1];
  const m1 = memory[memory.length - 2];
  for (let c = 0; c < N_CH; c++) {
    const vel = m0.ch[c] - m1.ch[c];
    prem[c] = Math.max(0, Math.min(1, ch.act[c] + vel * disc * 2));
  }
  return prem;
}

// ═══════════════════════════════════════════════════
// METAKLAUS — ghost with sensitivity tensor
// ═══════════════════════════════════════════════════

function buildCrossAffinity(a: LangPack, b: LangPack): CrossAffinity {
  const n_a = a.exhale.length;
  const n_b = b.exhale.length;
  const matrix = new Float32Array(n_a * n_b);
  for (let i = 0; i < n_a; i++) {
    for (let j = 0; j < n_b; j++) {
      let s = 0;
      for (let c = 0; c < N_CH; c++) s += a.exhale[i].aff[c] * b.exhale[j].aff[c];
      matrix[i * n_b + j] = s;
    }
  }
  return { matrix, n_a, n_b, langA: a.code, langB: b.code };
}

function metaklausCompute(
  ch: Chambers,
  primaryLang: string,
  langs: Map<string, LangPack>,
  crossAffinities: Map<string, CrossAffinity>,
  sensitivity: number[][][],
): { ghost: number[]; interference: number } {
  const primary = langs.get(primaryLang)!;
  const nEx = primary.exhale.length;
  const ghost = new Array(nEx).fill(0);
  const chNorm = Math.sqrt(ch.act.reduce((s, v) => s + v * v, 0) + 1e-12);
  if (chNorm < 1e-6) return { ghost, interference: 0 };

  const dom = dominant(ch);

  for (const [code, lp] of langs) {
    if (code === primaryLang) continue;
    const key = `${primaryLang}:${code}`;
    const ca = crossAffinities.get(key);
    if (!ca) continue;

    const wLang = GHOST_WEIGHT[dom][ghostLangIdx(code)];

    // attention to other language
    let bestJ = 0, bestSim = -1;
    for (let j = 0; j < lp.exhale.length; j++) {
      let sim = 0;
      for (let c = 0; c < N_CH; c++) sim += ch.act[c] * lp.exhale[j].aff[c];
      sim /= chNorm;
      if (sim > bestSim) { bestSim = sim; bestJ = j; }
    }

    // ghost with sensitivity tensor
    for (let w = 0; w < nEx; w++) {
      const raw = ca.matrix[w * ca.n_b + bestJ];
      let modulated = 0;
      for (let g = 0; g < N_CH; g++) {
        for (let p = 0; p < N_CH; p++) {
          modulated += lp.exhale[bestJ].aff[g] * primary.exhale[w].aff[p] * sensitivity[dom][g][p];
        }
      }
      const agreement = 0.6 * raw + 0.4 * modulated;
      const interf = (agreement - 0.5) * 2;
      ghost[w] += interf * bestSim * wLang;
    }
  }

  const nOther = langs.size - 1;
  if (nOther > 0) for (let w = 0; w < nEx; w++) ghost[w] /= nOther;
  const interference = ghost.reduce((s, v) => s + Math.abs(v), 0) / Math.max(nEx, 1);
  return { ghost, interference };
}

// ═══════════════════════════════════════════════════
// INHALE
// ═══════════════════════════════════════════════════

function inhaleProcess(lp: LangPack, prompt: string): number[] {
  const emotion = new Array(N_CH).fill(0);
  let matches = 0;
  const words = prompt.toLowerCase().split(/[\s\t\n\r.,!?;:"'()\-]+/).filter(Boolean);
  for (const w of words) {
    for (const iw of lp.inhale) {
      if (w === iw.text || prompt.includes(iw.text)) {
        for (let c = 0; c < N_CH; c++) emotion[c] += iw.aff[c];
        matches++; break;
      }
    }
  }
  if (matches === 0) {
    for (const w of words) {
      let bestSim = 0, bestW: Word | null = null;
      for (const iw of lp.inhale) {
        const sim = wordSimilarity(w, iw.text);
        if (sim > bestSim) { bestSim = sim; bestW = iw; }
      }
      if (bestW && bestSim > 0.2) {
        for (let c = 0; c < N_CH; c++) emotion[c] += bestW.aff[c] * bestSim;
        matches++;
      }
    }
  }
  if (matches > 0) for (let c = 0; c < N_CH; c++) emotion[c] /= matches;
  else {
    const h = hashWord(prompt);
    for (let c = 0; c < N_CH; c++) emotion[c] = Number((h >> BigInt(c * 8)) & 0xFFn) / 255;
  }
  return emotion;
}

// ═══════════════════════════════════════════════════
// EXHALE
// ═══════════════════════════════════════════════════

function exhaleGenerate(
  ch: Chambers, lp: LangPack, ghost: number[],
  prevExhale: number[], usedExhale: Set<string>,
  prophecies: Prophecy[],
): { words: number[]; newPrev: number[] } {
  const nEx = lp.exhale.length;
  if (nEx === 0) return { words: [], newPrev: [] };

  const hebb = metaHebbian(lp.meta, prevExhale, nEx);
  const result: number[] = [];
  let prev = prevExhale.length > 0 ? prevExhale[prevExhale.length - 1] : -1;
  const localUsed = new Set(usedExhale);
  const propPressure = prophecies.reduce((s, p) => s + p.strength, 0) / 3;

  for (let step = 0; step < MAX_RESPONSE; step++) {
    const logits = new Array(nEx);
    for (let w = 0; w < nEx; w++) {
      let somaScore = 0;
      for (let c = 0; c < N_CH; c++) somaScore += ch.act[c] * lp.exhale[w].aff[c];
      const biScore = prev >= 0 ? metaBigram(lp.meta, prev, w) : 0;
      // Full Dario 7-force
      const amod = Math.max(0.5,Math.min(2, 1+0.3*ch.act[1]-0.2*ch.act[2]+0.1*ch.act[4]));
      const tmod = Math.max(0.5,Math.min(2, 1+0.5*ch.act[4]-0.3*ch.act[0]));
      const vt = Math.max(0.1, tmod * GEN_TEMP);
      const B = biScore * BIGRAM_BASE;
      const H = amod * DARIO_ALPHA * hebb[w];
      const G = DARIO_ZETA * (ghost[w] || 0);
      logits[w] = (B + H + G + somaScore) / vt;
      if (localUsed.has(String(w)) || localUsed.has(lp.exhale[w].text)) logits[w] -= 100;
    }
    for (let w = 0; w < nEx; w++) logits[w] /= GEN_TEMP;
    // top-K
    const indices = Array.from({ length: nEx }, (_, i) => i);
    indices.sort((a, b) => logits[b] - logits[a]);
    const topK = indices.slice(0, TOP_K);
    const mx = logits[topK[0]];
    const probs = topK.map(i => Math.exp(logits[i] - mx));
    const sum = probs.reduce((a, b) => a + b, 0);
    for (let i = 0; i < probs.length; i++) probs[i] /= sum;
    const r = randf();
    let cum = 0, chosen = topK[0];
    for (let i = 0; i < topK.length; i++) {
      cum += probs[i];
      if (cum >= r) { chosen = topK[i]; break; }
    }
    result.push(chosen);
    localUsed.add(String(chosen));
    localUsed.add(lp.exhale[chosen].text);
    prev = chosen;
    if (step > 2) {
      let score = 0;
      for (let c = 0; c < N_CH; c++) score += ch.act[c] * lp.exhale[chosen].aff[c];
      if (score < 0.2) break;
    }
  }
  const newPrev = result.slice(-4);
  for (const w of result) { usedExhale.add(String(w)); usedExhale.add(lp.exhale[w].text); }
  return { words: result, newPrev };
}

// ═══════════════════════════════════════════════════
// MEMORY
// ═══════════════════════════════════════════════════

function memoryBlend(memory: SomaSlot[]): number[] {
  const out = new Array(N_CH).fill(0);
  if (memory.length === 0) return out;
  let totalW = 0;
  for (let i = 0; i < memory.length; i++) {
    const idx = memory.length - 1 - i;
    const w = Math.pow(MEM_DECAY, i);
    for (let c = 0; c < N_CH; c++) out[c] += memory[idx].ch[c] * w;
    totalW += w;
  }
  if (totalW > 0) for (let c = 0; c < N_CH; c++) out[c] /= totalW;
  return out;
}

// ═══════════════════════════════════════════════════
// KLAUS ENGINE
// ═══════════════════════════════════════════════════

class Klaus {
  langPacks = new Map<string, LangPack>();
  ch: Chambers = chambersInit();
  memory: SomaSlot[] = [];
  mlp!: MLP;
  sensitivity: number[][][] = buildSensitivity();
  crossAffinities = new Map<string, CrossAffinity>();
  prevExhale: number[] = [];
  usedExhale = new Set<string>();
  prophecies: Prophecy[] = [];

  constructor(private baseDir: string) {}

  init(): boolean {
    console.log("╔══════════════════════════════════════════════╗");
    console.log(`║  KLAUS — Kinetic Linguistic Adaptive         ║`);
    console.log(`║          Unified Sonar v${KLAUS_VERSION}                 ║`);
    console.log("║  TypeScript inference. Zero deps.             ║");
    console.log("╚══════════════════════════════════════════════╝\n");

    this.loadLanguages();
    if (this.langPacks.size === 0) { console.error("ERROR: no language packs"); return false; }
    console.log(`[klaus.ts] ${this.langPacks.size} language(s) loaded`);

    for (const [code, lp] of this.langPacks) this.initMeta(code);

    // build cross-affinity matrices
    const codes = [...this.langPacks.keys()];
    for (const a of codes) {
      for (const b of codes) {
        if (a === b) continue;
        this.crossAffinities.set(`${a}:${b}`,
          buildCrossAffinity(this.langPacks.get(a)!, this.langPacks.get(b)!));
      }
    }
    console.log(`[klaus.ts] ${this.crossAffinities.size} cross-affinity matrices`);

    this.mlp = initMLP(this.langPacks);
    console.log("[klaus.ts] MLP: 13 → 32 → 16 → 6");

    this.ch = chambersInit();
    const disc = calendarDissonance();
    console.log(`[klaus.ts] calendar dissonance: ${disc.toFixed(3)}`);
    console.log("[klaus.ts] ready. inhale.\n");
    return true;
  }

  private loadLanguages(): void {
    const inhaleDir = path.join(this.baseDir, "inhale");
    const exhaleDir = path.join(this.baseDir, "exhale");
    if (!fs.existsSync(inhaleDir)) return;
    for (const fname of fs.readdirSync(inhaleDir).sort()) {
      if (!fname.endsWith(".txt")) continue;
      const code = fname.slice(0, -4);
      const exPath = path.join(exhaleDir, `ex-${code}.txt`);
      if (!fs.existsSync(exPath)) continue;
      const inhale = this.loadWords(path.join(inhaleDir, fname), code);
      const exhale = this.loadWords(exPath, code);
      this.langPacks.set(code, { code, inhale, exhale, meta: { bigrams: new Map(), hebbs: new Map() }, bpeMerges: [] });
      console.log(`[klaus.ts] loaded ${code}: ${inhale.length} inhale, ${exhale.length} exhale`);
    }
  }

  private loadWords(filePath: string, langCode: string): Word[] {
    return fs.readFileSync(filePath, "utf-8")
      .split("\n")
      .map(l => l.trim())
      .filter(Boolean)
      .map(text => ({ text, aff: computeAffinity(text, langCode) }));
  }

  private initMeta(code: string): void {
    const lp = this.langPacks.get(code)!;
    const corpus = lp.exhale.map(w => w.text).join(" ") + " " + lp.inhale.map(w => w.text).join(" ");
    const { merges, tokens } = bpeLearn(corpus, 512);
    lp.bpeMerges = merges;
    const tokenIds = bpeEncode(corpus, merges);
    lp.meta = metaBuild(tokenIds);
    console.log(`[klaus.ts] ${code}: BPE ${merges.length} merges, meta ${lp.meta.bigrams.size} bi ${lp.meta.hebbs.size} hebb`);
  }

  process(prompt: string): KlausResponse {
    const lang = detectLanguage(prompt, this.langPacks);
    const lp = this.langPacks.get(lang)!;
    const emotion = inhaleProcess(lp, prompt);
    const memState = memoryBlend(this.memory);
    const disc = calendarDissonance();

    const mlpIn = [...emotion, ...memState, disc];
    const mlpOut = mlpForward(this.mlp, mlpIn);

    // inject emotion into sub-chambers
    for (let c = 0; c < N_CH; c++) {
      const mixed = 0.4 * emotion[c] + 0.3 * mlpOut[c] + 0.2 * memState[c] + 0.1 * this.ch.soma[c];
      for (let s = 0; s < N_SUB; s++) {
        this.ch.sub[c][s] = {
          act: Math.max(0, Math.min(1, this.ch.sub[c][s].act + mixed / N_SUB)),
          phase: this.ch.sub[c][s].phase,
          freq: this.ch.sub[c][s].freq,
        };
      }
    }

    chambersCrossfire(this.ch, XFIRE_ITERS);
    const dom = dominant(this.ch);
    const prem = propheticPremonition(this.ch, disc, this.memory);
    const isProphetic = disc > 0.3 && this.memory.length >= 2;

    const { ghost, interference } = metaklausCompute(
      this.ch, lang, this.langPacks, this.crossAffinities, this.sensitivity);

    const { words: wordIds, newPrev } = exhaleGenerate(
      this.ch, lp, ghost, this.prevExhale, this.usedExhale, this.prophecies);
    this.prevExhale = newPrev;

    // store memory
    const slot: SomaSlot = {
      ch: [...this.ch.act],
      valence: this.ch.act[1] + this.ch.act[4] - this.ch.act[0] - this.ch.act[3],
      arousal: this.ch.act[2] + this.ch.act[0] + this.ch.act[5],
      cal: disc,
    };
    if (this.memory.length >= MEM_SLOTS) this.memory.shift();
    this.memory.push(slot);

    // tick prophecies
    this.prophecies = this.prophecies.filter(p => {
      p.age++; p.strength *= 0.95;
      return p.age < 20 && p.strength > 0.01;
    });

    return {
      lang, words: wordIds.map(i => lp.exhale[i].text),
      chambers: [...this.ch.act], dominant: CH_NAMES[dom],
      prem, disc, ghostStrength: interference, isProphetic,
    };
  }

  printResponse(r: KlausResponse): void {
    const chStr = r.chambers.map((v, i) => `${CH_NAMES[i]}:${v.toFixed(2)}`).join(" ");
    console.log(`  [${chStr}]`);
    console.log(`  ${r.words.join(". ")}.`);
    if (r.ghostStrength > 0.1) {
      console.log(`  (metaklaus: ${r.dominant}-dominant, interference ${r.ghostStrength.toFixed(2)})`);
    }
    if (r.isProphetic) {
      const domP = r.prem.indexOf(Math.max(...r.prem));
      console.log(`  ~premonition~ [→${CH_NAMES[domP]}:${r.prem[domP].toFixed(2)} dissonance:${r.disc.toFixed(2)}]`);
    }
  }

  async interactive(): Promise<void> {
    const rl = readline.createInterface({ input: process.stdin, output: process.stdout });
    const ask = (): void => {
      rl.question("klaus> ", (line) => {
        const prompt = line.trim();
        if (!prompt) { ask(); return; }
        if (["exit", "quit", "q"].includes(prompt)) {
          console.log("\n[klaus.ts] exhale. goodbye.");
          rl.close(); return;
        }
        if (prompt === "status") {
          console.log(`  languages: ${[...this.langPacks.keys()].join(", ")}`);
          console.log(`  memory: ${this.memory.length}/${MEM_SLOTS}`);
          const chStr = this.ch.act.map((v, i) => `${CH_NAMES[i]}:${v.toFixed(2)}`).join(" ");
          console.log(`  chambers: ${chStr}`);
          console.log(`  calendar: ${calendarDissonance().toFixed(3)}`);
          ask(); return;
        }
        if (prompt === "reset") {
          this.ch = chambersInit(); this.memory = [];
          this.usedExhale.clear(); this.prevExhale = []; this.prophecies = [];
          console.log("  [reset]"); ask(); return;
        }
        const r = this.process(prompt);
        this.printResponse(r);
        ask();
      });
    };
    ask();
  }
}

// ═══════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════

async function main() {
  let baseDir = ".";
  let singlePrompt: string | null = null;
  const args = process.argv.slice(2);
  for (let i = 0; i < args.length; i++) {
    if (args[i] === "--dir" && args[i + 1]) { baseDir = args[++i]; }
    else if (args[i] === "--help") {
      console.log(`KLAUS v${KLAUS_VERSION} — TypeScript inference`);
      console.log("Usage: npx ts-node klaus.ts [--dir DIR] [PROMPT]");
      return;
    } else if (!args[i].startsWith("-")) { singlePrompt = args[i]; }
  }

  const k = new Klaus(baseDir);
  if (!k.init()) process.exit(1);

  if (singlePrompt) {
    const r = k.process(singlePrompt);
    k.printResponse(r);
  } else {
    await k.interactive();
  }
}

main();
