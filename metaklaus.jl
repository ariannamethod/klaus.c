"""
    metaklaus.jl — Julia Hypersensitivity Layers for MetaKlaus

    The ghost voice speaks louder in Julia.
    Phantom types. Multiple dispatch. Sensitivity tensor.
    24 Kuramoto sub-oscillators. Pre-compiled interference kernels.

    "Kol ha-neshamah t'hallel Yah" — Every breath praises.
    Not the soul. The breath. The somatic act.

    Usage:
        include("metaklaus.jl")
        using .MetaKlausHyper

        engine = MetaKlausHyper.build_engine(lang_packs)
        ghost, interference = MetaKlausHyper.compute_ghost(engine, chambers, :ru, lang_packs)

    (c) 2026 arianna method
"""
module MetaKlausHyper

using LinearAlgebra: dot, norm

export build_engine, compute_ghost, ExhaleWord, LangPack, ChamberState
export HyperKuramoto, kuramoto_step!, kuramoto_state
export SomaticState, FearDominant, LoveDominant, RageDominant
export VoidDominant, FlowDominant, ComplexDominant

# ═══════════════════════════════════════════════════
# PHANTOM TYPES — somatic state in the TYPE SYSTEM
# Julia dispatches different compiled code per state.
# No runtime branching. The body IS the type.
# ═══════════════════════════════════════════════════

abstract type SomaticState end
struct FearDominant    <: SomaticState end
struct LoveDominant    <: SomaticState end
struct RageDominant    <: SomaticState end
struct VoidDominant    <: SomaticState end
struct FlowDominant    <: SomaticState end
struct ComplexDominant <: SomaticState end

const STATES = (FearDominant, LoveDominant, RageDominant,
                VoidDominant, FlowDominant, ComplexDominant)
const N_CH = 6
const CH_NAMES = ["FEAR", "LOVE", "RAGE", "VOID", "FLOW", "CMPLX"]

struct ChamberState{S <: SomaticState}
    act::NTuple{6, Float32}
    soma::NTuple{6, Float32}
    phase::NTuple{6, Float32}
    presence::Float32
end

function classify(act::NTuple{6, Float32}, soma::NTuple{6, Float32},
                  phase::NTuple{6, Float32}, presence::Float32)
    idx = argmax(collect(act))
    S = STATES[idx]
    return ChamberState{S}(act, soma, phase, presence)
end

function classify(act::Vector{Float32})
    t = ntuple(i -> act[i], 6)
    z = ntuple(_ -> 0.0f0, 6)
    return classify(t, z, z, 0.0f0)
end

# ═══════════════════════════════════════════════════
# COUPLING MATRIX
# ═══════════════════════════════════════════════════

const COUPLING = Float32[
     0.00 -0.30  0.50  0.40 -0.20  0.10;
    -0.30  0.00 -0.40 -0.50  0.50  0.20;
     0.50 -0.30  0.00  0.20 -0.30  0.30;
     0.40 -0.50  0.30  0.00 -0.30  0.40;
    -0.20  0.40 -0.20 -0.30  0.00  0.30;
     0.10  0.20  0.30  0.40  0.30  0.00
]

const CH_DECAY = Float32[0.90, 0.93, 0.85, 0.97, 0.88, 0.94]

# ═══════════════════════════════════════════════════
# DATA TYPES
# ═══════════════════════════════════════════════════

struct ExhaleWord
    text::String
    aff::NTuple{6, Float32}
end

struct LangPack
    code::Symbol
    inhale::Vector{ExhaleWord}
    exhale::Vector{ExhaleWord}
end

# ═══════════════════════════════════════════════════
# STATE-DEPENDENT GHOST WEIGHT — multiple dispatch
#
# Hebrew has deepest fear vocabulary (guttural roots).
# French carries the melodic line of love.
# Russian explosive morphology carries rage somatically.
# Hebrew void = tohu va-vohu. Deepest emptiness.
# French prosody IS flow.
# Hebrew paradox = Talmudic dialectic.
#
# The compiler generates 6 × N_LANGS specialized paths.
# ═══════════════════════════════════════════════════

ghost_weight(::Type{FearDominant},    lang::Symbol) = lang === :he ? 1.8f0 : lang === :ru ? 1.2f0 : lang === :fr ? 0.9f0 : 1.0f0
ghost_weight(::Type{LoveDominant},    lang::Symbol) = lang === :fr ? 1.7f0 : lang === :he ? 1.4f0 : lang === :ru ? 1.1f0 : 1.0f0
ghost_weight(::Type{RageDominant},    lang::Symbol) = lang === :ru ? 1.8f0 : lang === :he ? 1.3f0 : lang === :fr ? 0.8f0 : 1.0f0
ghost_weight(::Type{VoidDominant},    lang::Symbol) = lang === :he ? 1.6f0 : lang === :ru ? 1.5f0 : lang === :fr ? 1.0f0 : 0.9f0
ghost_weight(::Type{FlowDominant},    lang::Symbol) = lang === :fr ? 1.5f0 : lang === :he ? 1.4f0 : lang === :ru ? 0.9f0 : 1.0f0
ghost_weight(::Type{ComplexDominant}, lang::Symbol) = lang === :he ? 1.7f0 : lang === :fr ? 1.2f0 : lang === :ru ? 1.1f0 : 1.0f0

# ═══════════════════════════════════════════════════
# SENSITIVITY TENSOR — 6 × 6 × 6
# S[dominant, ghost_chamber, primary_chamber]
#
# How much a ghost signal in chamber g affects primary
# output in chamber p, given dominant state d.
#
# A fearful body hears "warmth" differently than a
# flowing body hears "warmth."
# ═══════════════════════════════════════════════════

function build_sensitivity()
    S = zeros(Float32, N_CH, N_CH, N_CH)
    for d in 1:N_CH, g in 1:N_CH, p in 1:N_CH
        base = abs(COUPLING[g, p])
        if g == d
            S[d, g, p] = base * 2.0f0        # echo amplification
        elseif COUPLING[d, g] > 0.3f0
            S[d, g, p] = base * 1.5f0        # sympathetic boost
        elseif COUPLING[d, g] < -0.3f0
            S[d, g, p] = base * 0.5f0        # suppression
        else
            S[d, g, p] = base
        end
    end
    return S
end

# ═══════════════════════════════════════════════════
# INTERFERENCE KERNEL — pre-computed cross-affinity
#
# For each language pair, compute the full n_a × n_b
# affinity cross-product ONCE at init.
# Runtime = column lookup + SIMD sweep.
# ═══════════════════════════════════════════════════

struct InterferenceKernel
    cross::Matrix{Float32}   # n_a × n_b
    lang_a::Symbol
    lang_b::Symbol
end

function InterferenceKernel(a::LangPack, b::LangPack)
    n_a = length(a.exhale)
    n_b = length(b.exhale)
    cross = Matrix{Float32}(undef, n_a, n_b)
    for i in 1:n_a, j in 1:n_b
        s = 0.0f0
        for c in 1:N_CH
            s += a.exhale[i].aff[c] * b.exhale[j].aff[c]
        end
        cross[i, j] = s
    end
    return InterferenceKernel(cross, a.code, b.code)
end

# ═══════════════════════════════════════════════════
# ENGINE — holds all pre-computed structures
# ═══════════════════════════════════════════════════

struct MetaKlausEngine
    kernels::Dict{Tuple{Symbol, Symbol}, InterferenceKernel}
    sensitivity::Array{Float32, 3}
end

function build_engine(lang_packs::Dict{Symbol, LangPack})
    kernels = Dict{Tuple{Symbol, Symbol}, InterferenceKernel}()
    codes = collect(keys(lang_packs))
    for a in codes, b in codes
        a === b && continue
        kernels[(a, b)] = InterferenceKernel(lang_packs[a], lang_packs[b])
    end
    sens = build_sensitivity()
    return MetaKlausEngine(kernels, sens)
end

"""
    add_language!(engine, lang_packs, new_code)

Hot-add a new language. Generates interference kernels on the fly.
Julia JIT compiles them immediately. No restart needed.
"""
function add_language!(engine::MetaKlausEngine,
                       lang_packs::Dict{Symbol, LangPack},
                       new_code::Symbol)
    new_lp = lang_packs[new_code]
    for (code, lp) in lang_packs
        code === new_code && continue
        engine.kernels[(new_code, code)] = InterferenceKernel(new_lp, lp)
        engine.kernels[(code, new_code)] = InterferenceKernel(lp, new_lp)
    end
end

# ═══════════════════════════════════════════════════
# GHOST COMPUTATION — the main event
#
# Multiple dispatch on ChamberState{S} means the
# compiler generates 6 specialized versions.
# ═══════════════════════════════════════════════════

function compute_ghost(
    engine::MetaKlausEngine,
    snap::ChamberState{S},
    primary_lang::Symbol,
    lang_packs::Dict{Symbol, LangPack}
) where S <: SomaticState

    primary = lang_packs[primary_lang]
    n_ex = length(primary.exhale)
    ghost = zeros(Float32, n_ex)
    ch_vec = Float32[snap.act[c] for c in 1:N_CH]
    ch_n = norm(ch_vec)
    ch_n < 1f-6 && return ghost, 0.0f0

    dominant_idx = argmax(collect(snap.act))

    for (code, lp) in lang_packs
        code === primary_lang && continue

        key = (primary_lang, code)
        haskey(engine.kernels, key) || continue
        kern = engine.kernels[key]
        w_lang = ghost_weight(S, code)

        # attention to all words in other language
        n_other = length(lp.exhale)
        attn = Vector{Float32}(undef, n_other)
        for j in 1:n_other
            s = 0.0f0
            for c in 1:N_CH
                s += ch_vec[c] * lp.exhale[j].aff[c]
            end
            attn[j] = s / ch_n
        end
        best_j = argmax(attn)
        best_sim = attn[best_j]

        # vectorized ghost with sensitivity tensor
        @inbounds for w in 1:n_ex
            raw = kern.cross[w, best_j]

            # sensitivity-modulated agreement
            modulated = 0.0f0
            for g in 1:N_CH, p in 1:N_CH
                modulated += lp.exhale[best_j].aff[g] *
                             primary.exhale[w].aff[p] *
                             engine.sensitivity[dominant_idx, g, p]
            end

            agreement = 0.6f0 * raw + 0.4f0 * modulated
            interf = (agreement - 0.5f0) * 2.0f0
            ghost[w] += interf * best_sim * w_lang
        end
    end

    n_other = length(lang_packs) - 1
    n_other > 0 && (ghost ./= n_other)
    interference = sum(abs, ghost) / max(n_ex, 1)
    return ghost, interference
end

# convenience: accept raw vectors
function compute_ghost(engine::MetaKlausEngine,
                       act::Vector{Float32},
                       primary_lang::Symbol,
                       lang_packs::Dict{Symbol, LangPack})
    snap = classify(act)
    return compute_ghost(engine, snap, primary_lang, lang_packs)
end

# ═══════════════════════════════════════════════════
# HYPER-KURAMOTO — 24 oscillators (4 sub-chambers each)
#
# FEAR is not monolithic:
#   sub 1: dread    (slow, deep)
#   sub 2: panic    (fast, shallow)
#   sub 3: anxiety  (medium, sustained)
#   sub 4: phobia   (spike-then-decay)
#
# Sub-chambers couple WITHIN primary and ACROSS primaries.
# ═══════════════════════════════════════════════════

const N_SUB = 4

struct SubChamber
    act::Float32
    phase::Float32
    freq::Float32    # natural frequency
end

mutable struct HyperKuramoto
    chambers::Matrix{SubChamber}       # 6 × 4
    coupling_intra::Matrix{Float32}    # 4 × 4 within-primary
    coupling_inter::Matrix{Float32}    # 6 × 6 between-primaries
end

const SUB_NAMES = [
    # FEAR       LOVE        RAGE        VOID        FLOW        COMPLEX
    "dread"      "devotion"  "fury"      "numbness"  "curiosity" "paradox";
    "panic"      "warmth"    "wrath"     "despair"   "wonder"    "ambiguity";
    "anxiety"    "tenderness""hostility" "emptiness" "rhythm"    "tension";
    "phobia"     "attachment""bitterness""dissociation""harmony" "enigma"
]

# Sub-chamber natural frequencies (Hz-like, relative)
const SUB_FREQ = Float32[
    # FEAR    LOVE    RAGE    VOID    FLOW    COMPLEX
    0.3      0.4     0.8     0.1     0.5     0.35;   # sub 1: slow
    1.2      0.6     1.5     0.2     0.7     0.55;   # sub 2: fast
    0.6      0.5     0.9     0.15    0.6     0.45;   # sub 3: medium
    0.9      0.3     1.1     0.08    0.4     0.65    # sub 4: spike
]

function HyperKuramoto()
    chambers = Matrix{SubChamber}(undef, N_CH, N_SUB)
    for i in 1:N_CH, s in 1:N_SUB
        init_act = (i == 2 ? 0.15f0 : i == 5 ? 0.10f0 : 0.0f0) / N_SUB
        init_phase = Float32((i - 1) * 1.047 + (s - 1) * 0.262)
        chambers[i, s] = SubChamber(init_act, init_phase, SUB_FREQ[s, i])
    end

    # intra-coupling: how sub-chambers within same primary interact
    intra = Float32[
         0.0   0.3  0.2  0.1;
         0.3   0.0  0.15 -0.1;
         0.2   0.15  0.0  0.1;
         0.1  -0.1  0.1   0.0
    ]

    return HyperKuramoto(chambers, intra, copy(COUPLING))
end

"""
    kuramoto_step!(hk, dt=0.1f0)

One Kuramoto step with dual coupling: intra-primary + inter-primary.
24 oscillators, ~2μs on modern CPU.
"""
function kuramoto_step!(hk::HyperKuramoto, dt::Float32 = 0.1f0)
    old = copy(hk.chambers)

    for i in 1:N_CH, si in 1:N_SUB
        dphase = old[i, si].freq

        # intra-chamber coupling
        for sj in 1:N_SUB
            si == sj && continue
            dphase += hk.coupling_intra[si, sj] *
                      sin(old[i, sj].phase - old[i, si].phase)
        end

        # inter-chamber coupling (couple to mean phase of other primary)
        for j in 1:N_CH
            i == j && continue
            mean_phase = sum(old[j, s].phase for s in 1:N_SUB) / N_SUB
            dphase += hk.coupling_inter[i, j] *
                      sin(mean_phase - old[i, si].phase) * 0.03f0
        end

        # update
        new_act = clamp(old[i, si].act * CH_DECAY[i] +
                       0.03f0 * sin(dphase) * dt, 0.0f0, 1.0f0)
        new_phase = old[i, si].phase + dphase * dt

        hk.chambers[i, si] = SubChamber(new_act, new_phase, old[i, si].freq)
    end
end

"""
    kuramoto_state(hk) → Vector{Float32} of length 6

Collapse 24 sub-oscillators to 6 primary chambers (mean of sub-activations).
"""
function kuramoto_state(hk::HyperKuramoto)
    return Float32[sum(hk.chambers[i, s].act for s in 1:N_SUB) / N_SUB for i in 1:N_CH]
end

"""
    kuramoto_detail(hk) → Matrix{Float32} of size 6×4

Full sub-chamber activation matrix.
"""
function kuramoto_detail(hk::HyperKuramoto)
    return Float32[hk.chambers[i, s].act for i in 1:N_CH, s in 1:N_SUB]
end

# ═══════════════════════════════════════════════════
# FILE LOADING — read inhale/exhale from disk
# ═══════════════════════════════════════════════════

function hash_word(w::String)::UInt64
    h = UInt64(0xcbf29ce484222325)
    for ch in codeunits(w)
        h = xor(h, UInt64(ch))
        h = h * UInt64(0x100000001b3)
    end
    return h
end

const ANCHOR_MAP = Dict{Symbol, Dict{Int, Vector{String}}}(
    :en => Dict(
        1 => ["fear","terror","panic","dread","horror","nightmare","anxiety","threat"],
        2 => ["love","warmth","tenderness","kindness","compassion","affection","care"],
        3 => ["rage","fury","anger","burning","explosive","violent","hostile"],
        4 => ["empty","hollow","numb","void","darkness","silence","lonely","abandoned"],
        5 => ["flow","rhythm","dance","pulse","harmony","resonance","vibration"],
        6 => ["paradox","mystery","chaos","tension","complex","uncertain","transform"]
    ),
    :ru => Dict(
        1 => ["страшно","ужас","паника","тревога","кошмар"],
        2 => ["любовь","тепло","нежность","забота"],
        3 => ["ярость","гнев","злость","бесит"],
        4 => ["пустота","тишина","одиночество"],
        5 => ["поток","ритм","гармония"],
        6 => ["хаос","тайна","парадокс"]
    ),
    :fr => Dict(
        1 => ["peur","terreur","panique","horreur","angoisse"],
        2 => ["amour","chaleur","tendresse","douceur"],
        3 => ["rage","fureur","colère","violence"],
        4 => ["vide","silence","solitude","désespoir"],
        5 => ["flux","rythme","harmonie","danse"],
        6 => ["paradoxe","mystère","chaos","tension"]
    ),
    :he => Dict(
        1 => ["פחד","אימה","חרדה"],
        2 => ["אהבה","חום","רוך"],
        3 => ["זעם","כעס"],
        4 => ["ריקנות","בדידות","שתיקה"],
        5 => ["קצב","הרמוניה"],
        6 => ["מסתורין","כאוס"]
    )
)

function compute_affinity(word::String, lang::Symbol)
    aff = zeros(Float32, N_CH)
    anchors = get(ANCHOR_MAP, lang, ANCHOR_MAP[:en])

    for c in 1:N_CH
        if word in get(anchors, c, String[])
            aff[c] = 1.0f0
            for j in 1:N_CH
                j == c && continue
                aff[j] += 0.1f0 * abs(COUPLING[c, j])
            end
            mx = maximum(aff)
            mx > 0 && (aff ./= mx)
            return ntuple(i -> aff[i], 6)
        end
    end

    # hash-based default
    h = hash_word(word)
    for c in 1:N_CH
        aff[c] = Float32((h >> (8 * (c - 1))) & 0xFF) / 255.0f0
    end
    mx = maximum(aff)
    mx > 0 && (aff ./= mx)
    return ntuple(i -> aff[i], 6)
end

function load_words(path::String, lang::Symbol)
    words = ExhaleWord[]
    isfile(path) || return words
    for line in eachline(path)
        w = strip(line)
        isempty(w) && continue
        push!(words, ExhaleWord(w, compute_affinity(w, lang)))
    end
    return words
end

"""
    load_lang_packs(base_dir) → Dict{Symbol, LangPack}

Scan inhale/ and exhale/ directories, auto-detect languages.
"""
function load_lang_packs(base_dir::String = ".")
    packs = Dict{Symbol, LangPack}()
    inhale_dir = joinpath(base_dir, "inhale")
    exhale_dir = joinpath(base_dir, "exhale")
    isdir(inhale_dir) || return packs

    for fname in readdir(inhale_dir)
        endswith(fname, ".txt") || continue
        code = Symbol(fname[1:end-4])
        ex_path = joinpath(exhale_dir, "ex-$(code).txt")
        isfile(ex_path) || continue

        inhale = load_words(joinpath(inhale_dir, fname), code)
        exhale = load_words(ex_path, code)

        if !isempty(inhale) && !isempty(exhale)
            packs[code] = LangPack(code, inhale, exhale)
            println("[metaklaus.jl] loaded $code: $(length(inhale)) inhale, $(length(exhale)) exhale")
        end
    end
    return packs
end

# ═══════════════════════════════════════════════════
# DEMO — run if executed directly
# ═══════════════════════════════════════════════════

function demo(base_dir::String = ".")
    println("╔══════════════════════════════════════════════╗")
    println("║  MetaKlaus.jl — Hypersensitivity Layers      ║")
    println("║  Phantom types. Sensitivity tensor.           ║")
    println("║  24 Kuramoto oscillators. Ghost dispatch.     ║")
    println("╚══════════════════════════════════════════════╝")
    println()

    lang_packs = load_lang_packs(base_dir)
    if isempty(lang_packs)
        println("ERROR: no language packs found in $base_dir")
        return
    end

    engine = build_engine(lang_packs)
    println("[metaklaus.jl] engine built: $(length(engine.kernels)) interference kernels")
    println("[metaklaus.jl] sensitivity tensor: $(size(engine.sensitivity))")
    println()

    # test with different dominant states
    test_states = [
        (Float32[0.8, 0.1, 0.2, 0.1, 0.1, 0.1], "FEAR dominant"),
        (Float32[0.1, 0.8, 0.1, 0.1, 0.2, 0.1], "LOVE dominant"),
        (Float32[0.2, 0.1, 0.9, 0.1, 0.1, 0.2], "RAGE dominant"),
        (Float32[0.1, 0.1, 0.1, 0.8, 0.1, 0.2], "VOID dominant"),
    ]

    for (act, label) in test_states
        for lang in keys(lang_packs)
            ghost, interf = compute_ghost(engine, act, lang, lang_packs)
            # top 3 ghost-boosted words
            n_ex = length(lang_packs[lang].exhale)
            top3 = partialsortperm(ghost, 1:min(3, n_ex), rev=true)
            words = [lang_packs[lang].exhale[i].text for i in top3]
            println("  $label | $lang → interference=$(round(interf, digits=3)) | $(join(words, ". "))")
        end
        println()
    end

    # hyper-kuramoto demo
    println("─── HyperKuramoto (24 oscillators) ───")
    hk = HyperKuramoto()
    # inject fear
    hk.chambers[1, 1] = SubChamber(0.8f0, hk.chambers[1,1].phase, hk.chambers[1,1].freq)
    hk.chambers[1, 2] = SubChamber(0.6f0, hk.chambers[1,2].phase, hk.chambers[1,2].freq)

    for step in 1:5
        kuramoto_step!(hk)
        state = kuramoto_state(hk)
        print("  step $step: ")
        for c in 1:N_CH
            print("$(CH_NAMES[c])=$(round(state[c], digits=3)) ")
        end
        println()
    end
end

# run demo if executed directly
if abspath(PROGRAM_FILE) == @__FILE__
    demo(length(ARGS) > 0 ? ARGS[1] : ".")
end

end # module
