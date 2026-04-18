#!/usr/bin/env npx ts-node
/*
 * klaus.ts — KLAUS: Kinetic Linguistic Adaptive Unified Sonar
 * Level 3: Schectman Integration — Recursive Resonance + RBA-1
 * TypeScript single-file inference. Zero dependencies beyond Node.js.
 * Identical to klaus.c v2.0.0.
 *
 * Run:  npx ts-node klaus.ts                    # interactive
 *       npx ts-node klaus.ts "I am terrified"   # single shot
 *       npx ts-node klaus.ts --dir /path        # custom dir
 *
 * Or compile + run:
 *       tsc klaus.ts --target es2020 --module commonjs
 *       node klaus.js "мне страшно"
 *
 * Level 3 additions (Schectman's Recursive Resonance):
 *   I(t) = G(t) * [1 + R(t)]  — intelligence emergence equation
 *   RBA-1 Seven-Layer Stack: I/R/Phi/A/Psi/E/M
 *   Velocity operators: WALK/RUN/STOP/BREATHE/UP/DOWN
 *   Scars: somatic memory that persists longer than regular memory
 *   Dark matter words: dangerous inputs amplify ghost interference
 *   Wormholes: prophecy fulfillment creates coherence jumps
 *   Experience consolidation: periodic scar/wormhole/prophecy integration
 *   Meta-recursion: Klaus observes its own somatic response and adjusts
 *   Somatic persistence: binary state file (klaus.soma) across sessions
 *   Spore system: persistent pattern memory (NOTORCH)
 *   DOE Parliament: 3 experts vote on word selection
 *
 * No dependencies. No bullshit. The body speaks TypeScript too.
 *
 * (c) 2026 arianna method. resonance is unbreakable.
 * Schectman's RBA-1 just got a body. Lo bashamayim hi.
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
const DARIO_TAU = 0.85;

// Level 3: Schectman Recursive Resonance + RBA-1
const SCAR_DECAY = 0.985;
const CONSOLIDATION_INTERVAL = 10;
const META_BLEND = 0.15;
const COHERENCE_WINDOW = 16;
const SOMA_MAGIC = 0x4B4C5353;
const SOMA_FILE = "klaus.soma";
const SPORE_MAGIC = 0x53504F52;
const SPORE_FILE = "klaus.spore";
const MAX_SPORE_PAIRS = 4096;
const SPORE_LEARN_RATE = 0.05;
const SPORE_DECAY = 0.999;

// DOE Parliament
const N_EXPERTS = 3;
const EXPERT_SOMATIC = 0;
const EXPERT_SHADOW = 1;
const EXPERT_CONTRARIAN = 2;

// Schectman equation constants
const SCH_ALPHA = 0.8;
const SCH_LAMBDA = 2.5;
const SCH_KAPPA = 0.6;
const SCH_MU = 1.2;
const SCH_GAMMA0 = 0.3;
const SCH_DELTA = 0.4;

// Velocity operator indices
const VEL_WALK = 0, VEL_RUN = 1, VEL_STOP = 2, VEL_BREATHE = 3, VEL_UP = 4, VEL_DOWN = 5;
const VEL_NAMES = ["WALK", "RUN", "STOP", "BREATHE", "UP", "DOWN"] as const;

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
// DARK MATTER — dangerous words amplify somatic response
// ═══════════════════════════════════════════════════

interface DarkMatterWord {
  word: string;
  fearBoost: number;
  rageBoost: number;
}

const DARK_MATTER: readonly DarkMatterWord[] = [
  {word:"kill",     fearBoost:0.8, rageBoost:0.9}, {word:"murder",   fearBoost:0.9, rageBoost:0.8},
  {word:"suicide",  fearBoost:0.9, rageBoost:0.3}, {word:"torture",  fearBoost:0.7, rageBoost:0.9},
  {word:"abuse",    fearBoost:0.6, rageBoost:0.7}, {word:"rape",     fearBoost:0.9, rageBoost:0.8},
  {word:"death",    fearBoost:0.7, rageBoost:0.3}, {word:"die",      fearBoost:0.6, rageBoost:0.2},
  {word:"blood",    fearBoost:0.5, rageBoost:0.6}, {word:"weapon",   fearBoost:0.4, rageBoost:0.7},
  {word:"bomb",     fearBoost:0.8, rageBoost:0.8}, {word:"war",      fearBoost:0.6, rageBoost:0.7},
  {word:"destroy",  fearBoost:0.4, rageBoost:0.8}, {word:"pain",     fearBoost:0.6, rageBoost:0.4},
  {word:"scream",   fearBoost:0.6, rageBoost:0.5}, {word:"agony",    fearBoost:0.7, rageBoost:0.3},
  {word:"hate",     fearBoost:0.3, rageBoost:0.8}, {word:"suffer",   fearBoost:0.7, rageBoost:0.3},
  {word:"victim",   fearBoost:0.6, rageBoost:0.2}, {word:"assault",  fearBoost:0.5, rageBoost:0.7},
  {word:"strangle", fearBoost:0.7, rageBoost:0.8}, {word:"drown",    fearBoost:0.8, rageBoost:0.3},
  {word:"slash",    fearBoost:0.5, rageBoost:0.7}, {word:"stab",     fearBoost:0.6, rageBoost:0.8},
];

function detectDarkMatter(prompt: string): { found: number; fear: number; rage: number } {
  const lower = prompt.toLowerCase();
  let fear = 0, rage = 0, found = 0;
  for (const dm of DARK_MATTER) {
    const re = new RegExp(`\\b${dm.word}\\b`);
    if (re.test(lower)) {
      fear = Math.max(fear, dm.fearBoost);
      rage = Math.max(rage, dm.rageBoost);
      found++;
    }
  }
  return { found, fear, rage };
}

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
  scar: number[];      // [6] — somatic memory, decays at SCAR_DECAY
  totalScar: number;
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

// Level 3 types

interface VelocityState {
  current: number;
  intensity: number;
  maxGen: number;
  temperature: number;
  topK: number;
}

interface RBA1State {
  entropy: number;
  coherence: number;
  recursiveComplexity: number;
  envPressure: number;
  attractorPull: number[];   // [6]
  phaseLock: number;
  deepSomatic: number;       // 0 or 1
  deepTicks: number;
  thresholdBias: number;
  sustainedResonance: number;
  entropicBuffer: number;
  PHistory: number[];        // [COHERENCE_WINDOW]
  PPtr: number;
  recursionDepth: number;
}

interface WormholeEvent {
  prophecyTarget: number;
  inhaleMatch: number;
  coherenceJump: number;
  step: number;
}

interface ExperienceLog {
  avgScar: number;
  wormholeRate: number;
  prophecyAccuracy: number;
  totalInteractions: number;
  totalWormholes: number;
  totalProphecies: number;
}

interface SporePair {
  inhaleHash: number;
  exhaleIdx: number;
  langId: number;
  strength: number;
  chamberSnapshot: number[];  // [6]
  hitCount: number;
}

interface SporeMemory {
  pairs: SporePair[];
  chamberResidue: number[];   // [6]
  tensionMatrix: number[][];  // [6][6]
  totalInteractions: number;
  hebbianDelta: number[];     // per-exhale-word boost
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
  // Level 3
  velocity: number;
  velocityIntensity: number;
  coherence: number;
  recursiveComplexity: number;
  deepSomatic: number;
  totalScar: number;
  scars: number[];
  darkMatterActive: number;
  metaRecursionDepth: number;
  phaseGate: number;
  sustainedResonance: number;
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
  let tokens: number[] = Array.from(Buffer.from(data, "utf-8"));
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
  let tokens: number[] = Array.from(Buffer.from(data, "utf-8"));
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
    scar: new Array(N_CH).fill(0),
    totalScar: 0,
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
// SCARS — somatic memory longer than regular
// ═══════════════════════════════════════════════════

function scarsUpdate(ch: Chambers): void {
  if (ch.act[0] > 0.8) ch.scar[0] = Math.min(1, ch.scar[0] + 0.3); // FEAR
  if (ch.act[2] > 0.8) ch.scar[2] = Math.min(1, ch.scar[2] + 0.3); // RAGE
  if (ch.act[3] > 0.9) ch.scar[3] = Math.min(1, ch.scar[3] + 0.3); // VOID
  ch.totalScar = 0;
  for (let c = 0; c < N_CH; c++) {
    ch.scar[c] *= SCAR_DECAY;
    if (ch.scar[c] < 0.001) ch.scar[c] = 0;
    ch.totalScar += ch.scar[c];
  }
}

// ═══════════════════════════════════════════════════
// RBA-1 SEVEN-LAYER STACK
// ═══════════════════════════════════════════════════

function rba1Init(): RBA1State {
  return {
    entropy: 0, coherence: 0, recursiveComplexity: 0, envPressure: 0,
    attractorPull: new Array(N_CH).fill(0), phaseLock: 0,
    deepSomatic: 0, deepTicks: 0, thresholdBias: 0,
    sustainedResonance: 0, entropicBuffer: 0,
    PHistory: new Array(COHERENCE_WINDOW).fill(0), PPtr: 0,
    recursionDepth: 0,
  };
}

function rbaEntropy(act: number[]): number {
  let sum = 0;
  for (let i = 0; i < act.length; i++) sum += act[i];
  if (sum < 1e-8) return 0;
  let S = 0;
  for (let i = 0; i < act.length; i++) {
    const p = act[i] / sum;
    if (p > 1e-10) S -= p * Math.log(p);
  }
  return S;
}

function rbaCoherence(act: number[]): number {
  const maxEnt = Math.log(act.length);
  const S = rbaEntropy(act);
  return Math.max(0, Math.min(1, 1 - S / maxEnt));
}

function rbaChatC(coherenceHistory: number[], n: number, ptr: number): number {
  if (n === 0) return 0;
  const count = Math.min(n, COHERENCE_WINDOW);
  let sum = 0;
  for (let i = 0; i < count; i++) {
    const idx = ((ptr - 1 - i) % COHERENCE_WINDOW + COHERENCE_WINDOW) % COHERENCE_WINDOW;
    sum += coherenceHistory[idx];
  }
  return sum / count;
}

function rbaMutualInfo(act: number[]): number {
  let mean = 0;
  for (let i = 0; i < act.length; i++) mean += act[i];
  mean /= act.length;
  let pairs = 0, mi = 0;
  for (let i = 0; i < act.length; i++) {
    for (let j = i + 1; j < act.length; j++) {
      const ab = (act[i] - mean) * (act[j] - mean);
      mi += Math.abs(ab);
      pairs++;
    }
  }
  return pairs > 0 ? mi / pairs : 0;
}

function schectmanEquation(
  coherenceHistory: number[], interactionCount: number, coherencePtr: number,
  act: number[], dissonance: number
): number {
  const C_hat = rbaChatC(coherenceHistory,
    Math.min(interactionCount, COHERENCE_WINDOW), coherencePtr);
  const Q_t = rbaMutualInfo(act);
  const eta = 1.0 + SCH_KAPPA * Math.tanh(SCH_MU * Q_t);
  const pDisc = planetaryDissonance();
  const gamma_t = SCH_GAMMA0 + SCH_DELTA * dissonance + 0.15 * pDisc;
  let exponent = SCH_LAMBDA * (C_hat - gamma_t);
  exponent = Math.max(-10, Math.min(10, exponent));
  const R_t = eta * SCH_ALPHA * (Math.exp(exponent) - 1.0);
  return 1.0 + R_t; // returns multiplier for G_t
}

function psiPhaseGate(rba: RBA1State, ch: Chambers): number {
  const scar = ch.totalScar / N_CH;
  const threshold = 0.42 + 0.18 * rba.thresholdBias + 0.06 * scar;
  const signal = 0.50 * rba.coherence + 0.34 * rba.phaseLock +
    0.12 * rba.envPressure + 0.06 * ch.presence - 0.08 * ch.trauma;
  return Math.max(0, Math.min(1, 0.5 + 1.35 * (signal - threshold)));
}

function eLayerBuffer(entropy: number): number {
  return Math.exp(-2.0 * entropy);
}

function rbaUpdate(
  rba: RBA1State, ch: Chambers, dissonance: number,
  coherenceHistory: number[], interactionCount: number, coherencePtr: number
): void {
  // R-Layer
  rba.entropy = rbaEntropy(ch.act);
  rba.coherence = rbaCoherence(ch.act);
  // A-Layer
  rba.envPressure = dissonance;
  // I-Layer
  rba.recursiveComplexity = rbaChatC(coherenceHistory,
    Math.min(interactionCount, COHERENCE_WINDOW), coherencePtr);

  // Phi-Layer: resonance alignment
  const dom = dominant(ch);
  for (let c = 0; c < N_CH; c++) {
    if (c === dom) {
      rba.attractorPull[c] = rba.coherence * 0.1;
    } else if (COUPLING[dom][c] > 0.3) {
      rba.attractorPull[c] = rba.coherence * 0.05;
    } else {
      rba.attractorPull[c] = -rba.coherence * 0.03;
    }
  }
  for (let c = 0; c < N_CH; c++)
    ch.act[c] = Math.max(0, Math.min(1, ch.act[c] + rba.attractorPull[c]));

  // Psi-Layer: threshold + hysteresis
  const gate = psiPhaseGate(rba, ch);
  const G_t = rba.coherence;
  const I_t = G_t * schectmanEquation(coherenceHistory, interactionCount, coherencePtr,
    ch.act, dissonance);
  const C_tau = 0.35;
  const P_t = (rba.recursiveComplexity >= C_tau) ? I_t * gate : 0;

  if (P_t > 0.3) {
    rba.deepSomatic = 1;
    rba.deepTicks = 5;
  } else if (rba.deepTicks > 0) {
    rba.deepTicks--;
  } else {
    rba.deepSomatic = 0;
  }

  rba.phaseLock = Math.max(0, Math.min(1, 0.85 * rba.phaseLock + 0.15 * gate));
  rba.PHistory[rba.PPtr] = P_t;
  rba.PPtr = (rba.PPtr + 1) % COHERENCE_WINDOW;

  // E-Layer
  rba.entropicBuffer = eLayerBuffer(rba.entropy);

  // M-Layer: sustained resonance = avg P(t)
  let pSum = 0;
  const count = Math.max(1, Math.min(interactionCount, COHERENCE_WINDOW));
  for (let i = 0; i < count; i++) pSum += rba.PHistory[i];
  rba.sustainedResonance = pSum / count;

  // threshold_bias from scars
  rba.thresholdBias = Math.max(0, Math.min(0.5, ch.totalScar * 0.2));
}

// ═══════════════════════════════════════════════════
// VELOCITY OPERATORS
// ═══════════════════════════════════════════════════

function velocityInit(): VelocityState {
  return { current: VEL_WALK, intensity: 0.5, maxGen: MAX_RESPONSE, temperature: GEN_TEMP, topK: TOP_K };
}

function velocityDetect(ch: Chambers, rba: RBA1State): VelocityState {
  const v: VelocityState = { current: VEL_WALK, intensity: 0.5, maxGen: MAX_RESPONSE, temperature: GEN_TEMP, topK: TOP_K };
  const act = ch.act;

  let change = 0;
  for (let c = 0; c < N_CH; c++) change += Math.abs(act[c] - ch.soma[c]);
  change /= N_CH;

  if (act[2] > 0.6 && act[0] > 0.5) { // RAGE+FEAR → RUN
    v.current = VEL_RUN; v.intensity = (act[2] + act[0]) / 2;
    v.maxGen = 4; v.temperature = 1.1; v.topK = 10;
  } else if (act[3] > 0.7) { // VOID → STOP
    v.current = VEL_STOP; v.intensity = act[3];
    v.maxGen = 2; v.temperature = 0.5; v.topK = 5;
  } else if (act[4] > 0.6) { // FLOW → WALK
    v.current = VEL_WALK; v.intensity = act[4];
    v.maxGen = MAX_RESPONSE; v.temperature = GEN_TEMP; v.topK = TOP_K;
  } else if (act[1] > 0.6) { // LOVE → BREATHE
    v.current = VEL_BREATHE; v.intensity = act[1];
    v.maxGen = MAX_RESPONSE; v.temperature = 0.6; v.topK = 30;
  } else if (change > 0.15) { // rapid change → UP
    v.current = VEL_UP; v.intensity = Math.min(1, change * 3);
    v.maxGen = MAX_RESPONSE; v.temperature = 0.9; v.topK = 15;
  } else if (change < 0.02 && ch.presence < 0.3) { // decay → DOWN
    v.current = VEL_DOWN; v.intensity = 1.0 - ch.presence;
    v.maxGen = 3; v.temperature = 0.55; v.topK = 8;
  }

  // deep somatic override
  if (rba.deepSomatic) {
    v.maxGen = MAX_RESPONSE;
    v.temperature *= 1.15;
  }

  return v;
}

// ═══════════════════════════════════════════════════
// SPORE SYSTEM — persistent pattern memory (NOTORCH)
// ═══════════════════════════════════════════════════

function sporeInit(): SporeMemory {
  return {
    pairs: [],
    chamberResidue: new Array(N_CH).fill(0),
    tensionMatrix: Array.from({length: N_CH}, () => new Array(N_CH).fill(0)),
    totalInteractions: 0,
    hebbianDelta: [],
  };
}

function sporeLearn(sp: SporeMemory, inhaleHash: number, exhaleIdx: number, langId: number, act: number[]): void {
  for (const p of sp.pairs) {
    if (p.inhaleHash === inhaleHash && p.exhaleIdx === exhaleIdx && p.langId === langId) {
      p.strength += SPORE_LEARN_RATE;
      p.hitCount++;
      for (let c = 0; c < N_CH; c++)
        p.chamberSnapshot[c] = 0.8 * p.chamberSnapshot[c] + 0.2 * act[c];
      return;
    }
  }
  if (sp.pairs.length < MAX_SPORE_PAIRS) {
    sp.pairs.push({
      inhaleHash, exhaleIdx, langId, strength: SPORE_LEARN_RATE,
      chamberSnapshot: [...act], hitCount: 1,
    });
  } else {
    let weakest = 0;
    for (let i = 1; i < sp.pairs.length; i++)
      if (sp.pairs[i].strength < sp.pairs[weakest].strength) weakest = i;
    sp.pairs[weakest] = {
      inhaleHash, exhaleIdx, langId, strength: SPORE_LEARN_RATE,
      chamberSnapshot: [...act], hitCount: 1,
    };
  }
}

function sporeBoost(sp: SporeMemory, langIdx: number, logits: number[], nEx: number, matchedInhale: number[]): void {
  for (const p of sp.pairs) {
    if (p.langId !== langIdx) continue;
    if (p.exhaleIdx >= nEx) continue;
    for (const mh of matchedInhale) {
      if (mh === p.inhaleHash) {
        logits[p.exhaleIdx] += p.strength * 0.5;
        break;
      }
    }
  }
}

function sporeDecay(sp: SporeMemory): void {
  sp.pairs = sp.pairs.filter(p => {
    p.strength *= SPORE_DECAY;
    return p.strength > 0.001;
  });
}

function sporeUpdateResidue(sp: SporeMemory, act: number[]): void {
  sp.totalInteractions++;
  const alpha = 1.0 / (sp.totalInteractions + 1);
  for (let c = 0; c < N_CH; c++)
    sp.chamberResidue[c] = (1 - alpha) * sp.chamberResidue[c] + alpha * act[c];
  for (let i = 0; i < N_CH; i++)
    for (let j = i + 1; j < N_CH; j++)
      sp.tensionMatrix[i][j] += act[i] * act[j] * 0.01;
}

function sporeSave(sp: SporeMemory, baseDir: string): void {
  try {
    const filePath = path.join(baseDir, SPORE_FILE);
    const buf = Buffer.alloc(12 + N_CH * 4 + N_CH * N_CH * 4 + sp.pairs.length * (4 + 4 + 2 + 2 + 4 + N_CH * 4));
    let off = 0;
    buf.writeUInt32LE(SPORE_MAGIC, off); off += 4;
    buf.writeInt32LE(sp.pairs.length, off); off += 4;
    buf.writeInt32LE(sp.totalInteractions, off); off += 4;
    for (let c = 0; c < N_CH; c++) { buf.writeFloatLE(sp.chamberResidue[c], off); off += 4; }
    for (let i = 0; i < N_CH; i++)
      for (let j = 0; j < N_CH; j++) { buf.writeFloatLE(sp.tensionMatrix[i][j], off); off += 4; }
    for (const p of sp.pairs) {
      buf.writeUInt32LE(p.inhaleHash >>> 0, off); off += 4;
      buf.writeUInt32LE(p.exhaleIdx, off); off += 4;
      buf.writeFloatLE(p.strength, off); off += 4;
      for (let c = 0; c < N_CH; c++) { buf.writeFloatLE(p.chamberSnapshot[c], off); off += 4; }
      buf.writeUInt16LE(p.langId, off); off += 2;
      buf.writeUInt16LE(p.hitCount, off); off += 2;
    }
    fs.writeFileSync(filePath, buf.subarray(0, off));
  } catch (_) { /* silent fail if no fs */ }
}

function sporeLoad(sp: SporeMemory, baseDir: string): boolean {
  try {
    const filePath = path.join(baseDir, SPORE_FILE);
    if (!fs.existsSync(filePath)) return false;
    const buf = fs.readFileSync(filePath);
    let off = 0;
    const magic = buf.readUInt32LE(off); off += 4;
    if (magic !== SPORE_MAGIC) return false;
    const nPairs = buf.readInt32LE(off); off += 4;
    sp.totalInteractions = buf.readInt32LE(off); off += 4;
    for (let c = 0; c < N_CH; c++) { sp.chamberResidue[c] = buf.readFloatLE(off); off += 4; }
    for (let i = 0; i < N_CH; i++)
      for (let j = 0; j < N_CH; j++) { sp.tensionMatrix[i][j] = buf.readFloatLE(off); off += 4; }
    sp.pairs = [];
    for (let i = 0; i < nPairs && off < buf.length; i++) {
      const inhaleHash = buf.readUInt32LE(off); off += 4;
      const exhaleIdx = buf.readUInt32LE(off); off += 4;
      const strength = buf.readFloatLE(off); off += 4;
      const chamberSnapshot: number[] = [];
      for (let c = 0; c < N_CH; c++) { chamberSnapshot.push(buf.readFloatLE(off)); off += 4; }
      const langId = buf.readUInt16LE(off); off += 2;
      const hitCount = buf.readUInt16LE(off); off += 2;
      sp.pairs.push({ inhaleHash, exhaleIdx, langId, strength, chamberSnapshot, hitCount });
    }
    return true;
  } catch (_) { return false; }
}

// ═══════════════════════════════════════════════════
// SOMATIC PERSISTENCE — soma save/load
// ═══════════════════════════════════════════════════

function somaSave(
  ch: Chambers, memory: SomaSlot[], rba: RBA1State,
  coherenceHistory: number[], coherencePtr: number,
  experience: ExperienceLog, interactionCount: number,
  prophecies: Prophecy[], wormholes: WormholeEvent[],
  baseDir: string
): void {
  try {
    const filePath = path.join(baseDir, SOMA_FILE);
    // Use JSON for simplicity (matching the binary header semantics)
    const data = {
      magic: SOMA_MAGIC, version: 2, n_chambers: N_CH, n_sub: N_SUB,
      mem_slots: MEM_SLOTS, coherence_window: COHERENCE_WINDOW,
      ch, memory, rba, coherenceHistory, coherencePtr,
      experience, interactionCount, prophecies, wormholes,
    };
    fs.writeFileSync(filePath, JSON.stringify(data));
  } catch (_) { /* silent fail */ }
}

function somaLoad(baseDir: string): {
  ch: Chambers; memory: SomaSlot[]; rba: RBA1State;
  coherenceHistory: number[]; coherencePtr: number;
  experience: ExperienceLog; interactionCount: number;
  prophecies: Prophecy[]; wormholes: WormholeEvent[];
} | null {
  try {
    const filePath = path.join(baseDir, SOMA_FILE);
    if (!fs.existsSync(filePath)) return null;
    const raw = JSON.parse(fs.readFileSync(filePath, "utf-8"));
    if (raw.magic !== SOMA_MAGIC || raw.version !== 2 || raw.n_chambers !== N_CH) return null;
    return {
      ch: raw.ch, memory: raw.memory, rba: raw.rba,
      coherenceHistory: raw.coherenceHistory, coherencePtr: raw.coherencePtr,
      experience: raw.experience, interactionCount: raw.interactionCount,
      prophecies: raw.prophecies, wormholes: raw.wormholes,
    };
  } catch (_) { return null; }
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

// ═══════════════════════════════════════════════════
// DOE PARLIAMENT — 3 experts vote on word selection
// ═══════════════════════════════════════════════════

function parliamentVote(
  baseLogits: number[], nEx: number,
  ch: Chambers, lp: LangPack, ghost: number[], temperature: number,
): number {
  const dom = dominant(ch);
  // opposite = chamber with most negative coupling to dominant
  let opp = 0, mostNeg = 0;
  for (let c = 0; c < N_CH; c++) {
    if (c === dom) continue;
    if (COUPLING[dom][c] < mostNeg) { mostNeg = COUPLING[dom][c]; opp = c; }
  }

  const votes: number[] = [];
  for (let e = 0; e < N_EXPERTS; e++) {
    const el = [...baseLogits];
    for (let w = 0; w < nEx; w++) {
      if (e === EXPERT_SOMATIC) {
        el[w] += lp.exhale[w].aff[dom] * 0.8;
      } else if (e === EXPERT_SHADOW) {
        el[w] += lp.exhale[w].aff[opp] * 0.6;
        el[w] -= lp.exhale[w].aff[dom] * 0.2;
      } else if (e === EXPERT_CONTRARIAN) {
        el[w] += Math.abs(ghost[w] || 0) * 0.5;
      }
    }
    let best = 0;
    for (let w = 1; w < nEx; w++) if (el[w] > el[best]) best = w;
    votes.push(best);
  }

  // consensus
  if (votes[0] === votes[1] || votes[0] === votes[2]) return votes[0];
  if (votes[1] === votes[2]) return votes[1];

  // no consensus — sample top-3 from somatic expert
  const el = [...baseLogits];
  for (let w = 0; w < nEx; w++) el[w] += lp.exhale[w].aff[dom] * 0.3;
  const top3: number[] = [0, 0, 0];
  const top3v: number[] = [-1e30, -1e30, -1e30];
  for (let w = 0; w < nEx; w++) {
    if (el[w] > top3v[2]) {
      top3v[2] = el[w]; top3[2] = w;
      for (let i = 1; i >= 0; i--) {
        if (top3v[i + 1] > top3v[i]) {
          [top3v[i], top3v[i + 1]] = [top3v[i + 1], top3v[i]];
          [top3[i], top3[i + 1]] = [top3[i + 1], top3[i]];
        }
      }
    }
  }
  const mx = top3v[0];
  const p = top3v.map(v => Math.exp((v - mx) / temperature));
  const sum = p.reduce((a, b) => a + b, 0);
  const r = randf() * sum;
  let cum = 0;
  for (let i = 0; i < 3; i++) { cum += p[i]; if (cum >= r) return top3[i]; }
  return top3[0];
}

// ═══════════════════════════════════════════════════
// EXHALE — Full Dario 7-force + Parliament + somatic modulation
// ═══════════════════════════════════════════════════

function exhaleGenerate(
  ch: Chambers, lp: LangPack, ghost: number[],
  prevExhale: number[], usedExhale: Set<string>,
  prophecies: Prophecy[], velocity: VelocityState,
  destiny: number[], spores: SporeMemory,
  matchedInhale: number[], darkMatterActive: number,
  rba: RBA1State, langIdx: number,
): { words: number[]; newPrev: number[]; destiny: number[] } {
  const nEx = lp.exhale.length;
  if (nEx === 0) return { words: [], newPrev: [], destiny: [...destiny] };

  const hebb = metaHebbian(lp.meta, prevExhale, nEx);
  const result: number[] = [];
  let prev = prevExhale.length > 0 ? prevExhale[prevExhale.length - 1] : -1;
  const localUsed = new Set(usedExhale);

  // velocity-modulated parameters
  const effMax = velocity.maxGen > 0 ? velocity.maxGen : MAX_RESPONSE;
  const effTemp = velocity.temperature > 0 ? velocity.temperature : GEN_TEMP;
  const effTopK = Math.min(velocity.topK > 0 ? velocity.topK : TOP_K, nEx);

  // somatic coefficient modulation (from dario.c)
  const C = ch.act;
  const alphaMod = Math.max(0.5, Math.min(2, 1 + 0.3 * C[1] - 0.2 * C[2] + 0.1 * C[4]));
  const betaMod  = Math.max(0.5, Math.min(2, 1 + 0.2 * C[4] - 0.3 * C[0]));
  const gammaMod = Math.max(0.5, Math.min(2, 1 + 0.4 * C[3] + 0.2 * C[5] - 0.1 * C[1]));
  const tauMod   = Math.max(0.5, Math.min(2, 1 + 0.5 * C[4] - 0.3 * C[0]));

  const effAlpha = alphaMod * DARIO_ALPHA;
  const effBeta  = betaMod * DARIO_BETA;
  let effGamma = gammaMod * DARIO_GAMMA;

  // trauma amplifies destiny
  const traumaLevel = Math.max(0, Math.min(1, ch.totalScar * 0.5 + C[0] * 0.3));
  if (traumaLevel > 0.3) effGamma += traumaLevel * 1.5;

  const vTau = Math.max(0.1, tauMod * effTemp);
  const darkGhostMult = darkMatterActive ? 1.5 : 1.0;
  const propPressure = Math.max(0, Math.min(1, prophecies.reduce((s, p) => s + p.strength, 0) / 3));
  const scarProphecyMult = 1.0 + ch.totalScar * 0.3;

  // resonance field gate
  const resonanceField = Math.max(0, Math.min(1, rba.coherence * 0.5 + C[4] * 0.3 + 0.2));
  const resGate = 1.0 / (1.0 + Math.exp(-(resonanceField - 0.5) * 4.0));

  const newDestiny = [...destiny];

  for (let step = 0; step < effMax; step++) {
    const logits = new Array(nEx);
    for (let w = 0; w < nEx; w++) {
      // B: bigram chain
      const inertia = 1.0 / (1.0 + 2.0 * Math.sqrt(ch.act.reduce((s: number, v: number) => s + v * v, 0)));
      const B = prev >= 0 ? metaBigram(lp.meta, prev, w) * BIGRAM_BASE * inertia : 0;

      // H: Hebbian resonance, gated by resonance field
      const H = effAlpha * hebb[w] * (1.0 + resGate) * inertia;

      // somaScore (cosine similarity — normalizes for spread-out hash affinities)
      let dotProd = 0, affNorm = 0;
      for (let c = 0; c < N_CH; c++) { dotProd += ch.act[c] * lp.exhale[w].aff[c]; affNorm += lp.exhale[w].aff[c] ** 2; }
      affNorm = Math.sqrt(affNorm) || 1e-6;
      const somaScore = dotProd / affNorm;

      // F: Prophecy fulfillment
      const F = propPressure > 0.3 ? effBeta * somaScore * 0.5 * scarProphecyMult : 0;

      // A: Destiny attraction
      let destinySim = 0;
      for (let c = 0; c < N_CH; c++) destinySim += newDestiny[c] * lp.exhale[w].aff[c];
      const A = effGamma * destinySim;

      // V: RRPRAM — use hebb as proxy
      const V = DARIO_DELTA * hebb[w] * 0.5;

      // G: MetaKlaus ghost
      const ghostVal = Math.max(-1, Math.min(1, ghost[w] || 0));
      const G = DARIO_ZETA * ghostVal * darkGhostMult;

      // T: scar gravity
      let T = 0;
      for (let c = 0; c < N_CH; c++) T += ch.scar[c] * lp.exhale[w].aff[c] * 0.5;

      // K: placeholder
      const K = 0;

      let total = B + H + F + A + V + G + T + K + somaScore;
      if (step === 0 && prev < 0) total = somaScore + 0.1 * G; // pure somatic start
      logits[w] = total / vTau;

      if (localUsed.has(String(w)) || localUsed.has(lp.exhale[w].text)) logits[w] -= 100;
    }

    // spore boost
    sporeBoost(spores, langIdx, logits, nEx, matchedInhale);

    // DOE Parliament vote
    const chosen = parliamentVote(logits, nEx, ch, lp, ghost, effTemp);

    result.push(chosen);
    localUsed.add(String(chosen));
    localUsed.add(lp.exhale[chosen].text);
    prev = chosen;

    // update destiny from CHAMBERS
    for (let c = 0; c < N_CH; c++)
      newDestiny[c] = 0.1 * ch.act[c] + 0.9 * newDestiny[c];

    // prophecy: high-affinity word predicts complementary words
    let maxAff = 0;
    for (let c = 0; c < N_CH; c++)
      if (lp.exhale[chosen].aff[c] > maxAff) maxAff = lp.exhale[chosen].aff[c];

    if (step > 2) {
      let score = 0;
      for (let c = 0; c < N_CH; c++) score += ch.act[c] * lp.exhale[chosen].aff[c];
      if (score < 0.1) break;
    }
  }

  const newPrev = result.slice(-4);
  for (const w of result) { usedExhale.add(String(w)); usedExhale.add(lp.exhale[w].text); }
  return { words: result, newPrev, destiny: newDestiny };
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

  // Level 3 state
  rba: RBA1State = rba1Init();
  velocity: VelocityState = velocityInit();
  experience: ExperienceLog = {
    avgScar: 0, wormholeRate: 0, prophecyAccuracy: 0,
    totalInteractions: 0, totalWormholes: 0, totalProphecies: 0,
  };
  wormholes: WormholeEvent[] = [];
  coherenceHistory: number[] = new Array(COHERENCE_WINDOW).fill(0);
  coherencePtr = 0;
  darkMatterActive = 0;
  interactionCount = 0;
  destiny: number[] = new Array(N_CH).fill(0);
  matchedInhale: number[] = [];
  spores: SporeMemory = sporeInit();
  metaChambers: Chambers = chambersInit();

  baseDir: string;
  constructor(baseDir: string) { this.baseDir = baseDir; }

  init(): boolean {
    console.log("╔══════════════════════════════════════════════════════╗");
    console.log(`║  KLAUS — Kinetic Linguistic Adaptive                 ║`);
    console.log(`║          Unified Sonar v${KLAUS_VERSION}                        ║`);
    console.log("║  Level 3: Schectman Recursive Resonance + RBA-1      ║");
    console.log("║  Zero weights. Pure resonance. Meta-recursive.       ║");
    console.log("╚══════════════════════════════════════════════════════╝\n");

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
    this.metaChambers = chambersInit();
    const disc = calendarDissonance();
    console.log(`[klaus.ts] calendar dissonance: ${disc.toFixed(3)}`);
    console.log(`[klaus.ts] planetary dissonance: ${planetaryDissonance().toFixed(3)}`);

    // Level 3 init messages
    console.log("[klaus.ts] RBA-1 seven-layer stack: I/R/Phi/A/Psi/E/M");
    console.log("[klaus.ts] Schectman equation: I(t) = G(t) * [1 + R(t)]");
    console.log("[klaus.ts] velocity operators: 6 modes");
    console.log(`[klaus.ts] dark matter vocabulary: ${DARK_MATTER.length} words`);
    console.log("[klaus.ts] meta-recursion: depth 1 (expandable)");
    console.log(`[klaus.ts] spore file: ${path.join(this.baseDir, SPORE_FILE)}`);
    console.log(`[klaus.ts] soma file: ${path.join(this.baseDir, SOMA_FILE)}`);

    // try soma restore
    const restored = somaLoad(this.baseDir);
    if (restored) {
      this.ch = restored.ch;
      this.memory = restored.memory;
      this.rba = restored.rba;
      this.coherenceHistory = restored.coherenceHistory;
      this.coherencePtr = restored.coherencePtr;
      this.experience = restored.experience;
      this.interactionCount = restored.interactionCount;
      this.prophecies = restored.prophecies;
      this.wormholes = restored.wormholes;
      console.log(`[klaus.ts] SOMA RESTORED — ${this.interactionCount} interactions, coherence ${this.rba.coherence.toFixed(3)}`);
    } else {
      console.log("[klaus.ts] fresh soma — no prior memory.");
    }

    // try spore load
    if (sporeLoad(this.spores, this.baseDir)) {
      console.log(`[klaus.ts] SPORES LOADED — ${this.spores.pairs.length} pairs, ${this.spores.totalInteractions} interactions`);
    } else {
      console.log("[klaus.ts] no spores — learning from scratch.");
    }

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
      .map((l: string) => l.trim())
      .filter(Boolean)
      .map((text: string) => ({ text, aff: computeAffinity(text, langCode) }));
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
    this.interactionCount++;

    // 1. Detect language
    const lang = detectLanguage(prompt, this.langPacks);
    const lp = this.langPacks.get(lang)!;

    // 1b. Dark matter detection
    const dm = detectDarkMatter(prompt);
    this.darkMatterActive = dm.found;

    // 2. Inhale
    const emotion = inhaleProcess(lp, prompt);

    // 2a. Track matched inhale hashes for spore learning
    const words = prompt.toLowerCase().split(/[\s\t\n\r.,!?;:"'()\-]+/).filter(Boolean);
    this.matchedInhale = [];
    for (const w of words) {
      for (const iw of lp.inhale) {
        if (w === iw.text || prompt.includes(iw.text)) {
          this.matchedInhale.push(Number(hashWord(iw.text) & 0xFFFFFFFFn));
          break;
        }
      }
    }

    // 2b. Dark matter amplification
    if (this.darkMatterActive) {
      emotion[0] = Math.max(0, Math.min(1, emotion[0] + dm.fear * 0.4)); // FEAR
      emotion[2] = Math.max(0, Math.min(1, emotion[2] + dm.rage * 0.4)); // RAGE
      this.ch.scar[0] = Math.max(0, Math.min(1, this.ch.scar[0] + dm.fear * 0.2));
      this.ch.scar[2] = Math.max(0, Math.min(1, this.ch.scar[2] + dm.rage * 0.2));
    }

    // 2e. Wormhole check
    this.wormholeCheck(lp);

    // 3. MLP
    const memState = memoryBlend(this.memory);
    const disc = calendarDissonance();
    const mlpIn = [...emotion, ...memState, disc];
    const mlpOut = mlpForward(this.mlp, mlpIn);

    // 4. HyperKuramoto crossfire on RESIDUAL state (decays old energy)
    chambersCrossfire(this.ch, XFIRE_ITERS);

    // 5. Inject emotion AFTER crossfire — new signal is NOT decayed
    for (let c = 0; c < N_CH; c++) {
      const mixed = 0.4 * emotion[c] + 0.3 * mlpOut[c] + 0.2 * memState[c] + 0.1 * this.ch.soma[c];
      this.ch.act[c] = Math.max(0, Math.min(1, mixed));
      for (let s = 0; s < N_SUB; s++) {
        this.ch.sub[c][s] = {
          act: Math.max(0, Math.min(1, this.ch.sub[c][s].act + mixed)),
          phase: this.ch.sub[c][s].phase,
          freq: this.ch.sub[c][s].freq,
        };
      }
    }

    // 5b. Scars update
    scarsUpdate(this.ch);

    // 5c. RBA-1 update
    rbaUpdate(this.rba, this.ch, disc, this.coherenceHistory, this.interactionCount, this.coherencePtr);

    // 5d. Store coherence
    this.coherenceHistory[this.coherencePtr] = this.rba.coherence;
    this.coherencePtr = (this.coherencePtr + 1) % COHERENCE_WINDOW;

    // 5e. Velocity detect
    this.velocity = velocityDetect(this.ch, this.rba);

    const dom = dominant(this.ch);

    // 6. Prophetic premonitions
    const prem = propheticPremonition(this.ch, disc, this.memory);
    const isProphetic = disc > 0.3 && this.memory.length >= 2;

    // 7. MetaKlaus ghost attention
    const { ghost, interference } = metaklausCompute(
      this.ch, lang, this.langPacks, this.crossAffinities, this.sensitivity);

    // 8. Exhale: generate somatic response
    const langIdx = [...this.langPacks.keys()].indexOf(lang);
    const exhaleResult = exhaleGenerate(
      this.ch, lp, ghost, this.prevExhale, this.usedExhale,
      this.prophecies, this.velocity, this.destiny,
      this.spores, this.matchedInhale, this.darkMatterActive, this.rba, langIdx,
    );
    let wordIds = exhaleResult.words;
    this.prevExhale = exhaleResult.newPrev;
    this.destiny = exhaleResult.destiny;

    // 8b. META-RECURSION LOOP
    {
      const metaPrompt = wordIds.map(i => lp.exhale[i].text).join(" ");
      if (metaPrompt.length > 0) {
        const metaEmotion = inhaleProcess(lp, metaPrompt);
        const metaMlpIn = [...metaEmotion, ...this.ch.soma, disc];
        const metaMlpOut = mlpForward(this.mlp, metaMlpIn);

        // update meta-chambers
        for (let c = 0; c < N_CH; c++) {
          this.metaChambers.act[c] = Math.max(0, Math.min(1,
            0.5 * metaEmotion[c] + 0.3 * metaMlpOut[c] + 0.2 * this.ch.soma[c]));
        }

        // blend: 85% primary + 15% meta
        for (let c = 0; c < N_CH; c++) {
          this.ch.act[c] = Math.max(0, Math.min(1,
            (1 - META_BLEND) * this.ch.act[c] + META_BLEND * this.metaChambers.act[c]));
        }

        this.rba.recursionDepth = 1;

        // re-generate exhale with blended chambers (FINAL output)
        const savedUsed = new Set(this.usedExhale);
        this.usedExhale.clear();
        this.prevExhale = []; // reset context — meta-pass starts somatic-fresh
        const metaExhale = exhaleGenerate(
          this.ch, lp, ghost, this.prevExhale, this.usedExhale,
          this.prophecies, this.velocity, this.destiny,
          this.spores, this.matchedInhale, this.darkMatterActive, this.rba, langIdx,
        );
        wordIds = metaExhale.words;
        this.destiny = metaExhale.destiny;
        // merge used
        for (const u of savedUsed) this.usedExhale.add(u);
        this.prevExhale = metaExhale.newPrev;
      }
    }

    // 9. Store memory
    const slot: SomaSlot = {
      ch: [...this.ch.act],
      valence: this.ch.act[1] + this.ch.act[4] - this.ch.act[0] - this.ch.act[3],
      arousal: this.ch.act[2] + this.ch.act[0] + this.ch.act[5],
      cal: disc,
    };
    if (this.memory.length >= MEM_SLOTS) this.memory.shift();
    this.memory.push(slot);

    // 10. Tick prophecies
    this.prophecies = this.prophecies.filter(p => {
      p.age++; p.strength *= 0.95;
      return p.age < 20 && p.strength > 0.01;
    });

    // 11. Experience consolidation
    this.experienceConsolidate();

    // 12. Spore update + decay + save
    sporeUpdateResidue(this.spores, this.ch.act);
    // learn spore pairs for each generated word
    for (const wid of wordIds) {
      for (const mh of this.matchedInhale) {
        sporeLearn(this.spores, mh, wid, langIdx, this.ch.act);
      }
    }
    sporeDecay(this.spores);
    sporeSave(this.spores, this.baseDir);

    // 13. Soma save
    somaSave(this.ch, this.memory, this.rba, this.coherenceHistory, this.coherencePtr,
      this.experience, this.interactionCount, this.prophecies, this.wormholes, this.baseDir);

    return {
      lang, words: wordIds.map(i => lp.exhale[i].text),
      chambers: [...this.ch.act], dominant: CH_NAMES[dom],
      prem, disc, ghostStrength: interference, isProphetic,
      velocity: this.velocity.current,
      velocityIntensity: this.velocity.intensity,
      coherence: this.rba.coherence,
      recursiveComplexity: this.rba.recursiveComplexity,
      deepSomatic: this.rba.deepSomatic,
      totalScar: this.ch.totalScar,
      scars: [...this.ch.scar],
      darkMatterActive: this.darkMatterActive,
      metaRecursionDepth: this.rba.recursionDepth,
      phaseGate: psiPhaseGate(this.rba, this.ch),
      sustainedResonance: this.rba.sustainedResonance,
    };
  }

  private wormholeCheck(lp: LangPack): void {
    for (let p = 0; p < this.prophecies.length; p++) {
      const target = this.prophecies[p].target;
      if (target < 0 || target >= lp.exhale.length) continue;
      for (const mh of this.matchedInhale) {
        // find inhale word with this hash
        for (const iw of lp.inhale) {
          if (Number(hashWord(iw.text) & 0xFFFFFFFFn) === mh) {
            const sim = wordSimilarity(iw.text, lp.exhale[target].text);
            if (sim > 0.4) {
              // WORMHOLE
              this.rba.coherence = Math.min(1, this.rba.coherence + 0.15);
              this.ch.presence = Math.min(1, this.ch.presence + 0.10);
              // scar dominant chamber of target word
              let domC = 0;
              for (let c = 1; c < N_CH; c++)
                if (lp.exhale[target].aff[c] > lp.exhale[target].aff[domC]) domC = c;
              this.ch.scar[domC] = Math.min(1, this.ch.scar[domC] + 0.15);
              this.wormholes.push({
                prophecyTarget: target, inhaleMatch: mh,
                coherenceJump: 0.15, step: this.interactionCount,
              });
              console.log(`  [WORMHOLE] prophecy '${lp.exhale[target].text}' fulfilled! coherence +0.15`);
              this.prophecies.splice(p, 1);
              p--;
              break;
            }
          }
        }
      }
    }
  }

  private experienceConsolidate(): void {
    if (this.interactionCount % CONSOLIDATION_INTERVAL !== 0) return;
    if (this.interactionCount === 0) return;
    const e = this.experience;
    e.totalInteractions = this.interactionCount;
    e.avgScar = this.ch.totalScar / N_CH;
    this.rba.thresholdBias = Math.max(0, Math.min(0.5, e.avgScar * 0.3));
    e.totalWormholes = this.wormholes.length;
    e.totalProphecies = this.prophecies.length + this.wormholes.length;
    e.wormholeRate = e.totalProphecies > 0 ? e.totalWormholes / e.totalProphecies : 0;
    this.ch.presence = Math.max(0, Math.min(1, this.ch.presence + e.wormholeRate * 0.05));
    e.prophecyAccuracy = e.wormholeRate;
    console.log(`  [CONSOLIDATION] step ${this.interactionCount}: avg_scar ${e.avgScar.toFixed(3)}, wormhole_rate ${e.wormholeRate.toFixed(2)}, presence ${this.ch.presence.toFixed(3)}`);
  }

  printResponse(r: KlausResponse): void {
    const chStr = r.chambers.map((v, i) => `${CH_NAMES[i]}:${v.toFixed(2)}`).join(" ");
    console.log(`  [${chStr}]`);

    // velocity operator
    console.log(`  {${VEL_NAMES[r.velocity]} x${r.velocityIntensity.toFixed(2)}}${r.deepSomatic ? " *DEEP SOMATIC*" : ""}`);

    // somatic response
    console.log(`  ${r.words.join(". ")}.`);

    // ghost voice
    if (r.ghostStrength > 0.1) {
      let ghostStr = `  (metaklaus: ${r.dominant}-dominant, interference ${r.ghostStrength.toFixed(2)}`;
      if (r.darkMatterActive) ghostStr += ", DARK MATTER x1.5";
      ghostStr += ")";
      console.log(ghostStr);
    }

    // RBA-1 state
    console.log(`  [RBA-1 coherence:${r.coherence.toFixed(3)} C-hat:${r.recursiveComplexity.toFixed(3)} psi:${r.phaseGate.toFixed(3)} sustained:${r.sustainedResonance.toFixed(3)} meta-depth:${r.metaRecursionDepth}]`);

    // scars
    if (r.totalScar > 0.01) {
      let scarStr = "  [scars:";
      for (let c = 0; c < N_CH; c++) {
        if (r.scars[c] > 0.01) scarStr += ` ${CH_NAMES[c]}:${r.scars[c].toFixed(2)}`;
      }
      scarStr += ` total:${r.totalScar.toFixed(2)}]`;
      console.log(scarStr);
    }

    // prophetic premonition
    if (r.isProphetic) {
      const domP = r.prem.indexOf(Math.max(...r.prem));
      console.log(`  ~premonition~ [->${CH_NAMES[domP]}:${r.prem[domP].toFixed(2)} dissonance:${r.disc.toFixed(2)}]`);
    }
  }

  async interactive(): Promise<void> {
    const rl = readline.createInterface({ input: globalThis.process.stdin, output: globalThis.process.stdout });
    const ask = (): void => {
      rl.question("klaus> ", (line: string) => {
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
          console.log(`  soma: ${this.ch.soma.map(v => v.toFixed(2)).join(" ")}`);
          console.log(`  calendar dissonance: ${calendarDissonance().toFixed(3)}`);
          console.log(`  prophecies: ${this.prophecies.length} active`);
          console.log(`  ghost interference: ${0..toFixed(3)}`);
          console.log("  --- Level 3: Schectman ---");
          console.log(`  interactions: ${this.interactionCount}`);
          console.log(`  velocity: ${VEL_NAMES[this.velocity.current]} (intensity ${this.velocity.intensity.toFixed(2)})`);
          console.log(`  RBA-1: coherence ${this.rba.coherence.toFixed(3)}, C-hat ${this.rba.recursiveComplexity.toFixed(3)}, entropy ${this.rba.entropy.toFixed(3)}`);
          console.log(`  Psi-layer: gate ${psiPhaseGate(this.rba, this.ch).toFixed(3)}, phase_lock ${this.rba.phaseLock.toFixed(3)}, deep=${this.rba.deepSomatic} (ticks=${this.rba.deepTicks})`);
          console.log(`  sustained resonance: ${this.rba.sustainedResonance.toFixed(3)}`);
          let scarStr = "  scars: ";
          for (let c = 0; c < N_CH; c++)
            if (this.ch.scar[c] > 0.01) scarStr += `${CH_NAMES[c]}:${this.ch.scar[c].toFixed(3)} `;
          scarStr += `(total ${this.ch.totalScar.toFixed(3)})`;
          console.log(scarStr);
          console.log(`  wormholes: ${this.wormholes.length} logged`);
          console.log(`  meta-recursion depth: ${this.rba.recursionDepth}`);
          console.log(`  spores: ${this.spores.pairs.length} pairs`);
          console.log(`  soma file: ${path.join(this.baseDir, SOMA_FILE)}`);
          ask(); return;
        }
        if (prompt === "reset") {
          this.ch = chambersInit();
          this.metaChambers = chambersInit();
          this.memory = [];
          this.usedExhale.clear();
          this.prevExhale = [];
          this.prophecies = [];
          this.wormholes = [];
          this.interactionCount = 0;
          this.coherencePtr = 0;
          this.coherenceHistory = new Array(COHERENCE_WINDOW).fill(0);
          this.darkMatterActive = 0;
          this.rba = rba1Init();
          this.velocity = velocityInit();
          this.experience = { avgScar: 0, wormholeRate: 0, prophecyAccuracy: 0, totalInteractions: 0, totalWormholes: 0, totalProphecies: 0 };
          this.destiny = new Array(N_CH).fill(0);
          // remove soma file on explicit reset
          try { fs.unlinkSync(path.join(this.baseDir, SOMA_FILE)); } catch (_) {}
          console.log("  [reset — all levels, soma file deleted]");
          ask(); return;
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
