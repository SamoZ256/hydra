#pragma once

#include "core/hw/tegra_x1/gpu/engines/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

typedef u64 instruction_t;
STRONG_NUMBER_TYPEDEF(reg_t, u8);
STRONG_NUMBER_TYPEDEF(pred_t, u8);
STRONG_NUMBER_TYPEDEF(label_t, u32);

struct local_t {
    label_t label;
    u32 id;

    bool operator==(const local_t& other) const {
        return label == other.label && id == other.id;
    }
};

constexpr reg_t RZ = 255;
constexpr pred_t PT = 7;

// TODO: make an abstract Type class
enum class DataType {
    Invalid,

    U8,
    U16,
    U32,
    I8,
    I16,
    I32,
    F16,
    F32,
    F16X2,
};

DataType to_data_type(engines::VertexAttribType vertex_attrib_type);

inline DataType to_data_type(TextureFormat format) {
    // TODO: implement
    return DataType::F32;
}

struct AMem {
    reg_t reg;
    u64 imm;
    bool is_input;

    bool operator==(const AMem& other) const {
        return reg == other.reg && imm == other.imm;
    }
};

struct CMem {
    u32 idx;
    reg_t reg;
    u64 imm;

    bool operator==(const CMem& other) const {
        return idx == other.idx && reg == other.reg && imm == other.imm;
    }
};

struct PredCond {
    pred_t pred;
    bool not_;
    bool never;

    bool operator==(const PredCond& other) const {
        return pred == other.pred && not_ == other.not_ && never == other.never;
    }
};

enum class SvSemantic {
    Invalid,
    Position,
    UserInOut,
    InstanceID,
    VertexID,
    // TODO: more
};

constexpr u32 SV_POSITION_BASE = 0x70;
constexpr u32 SV_USER_IN_OUT_BASE = 0x80;
constexpr u32 SV_INSTANCE_ID_BASE = 0x2f8;
constexpr u32 SV_VERTEX_ID_BASE = 0x2fc;

struct Sv {
    SvSemantic semantic;
    u8 index;
    // TODO: more?

    Sv(SvSemantic semantic_, u8 index_ = invalid<u8>())
        : semantic{semantic_}, index{index_} {}

    bool const operator==(const Sv& o) const {
        return semantic == o.semantic && index == o.index;
    }

    bool const operator<(const Sv& o) const {
        return semantic < o.semantic ||
               (semantic == o.semantic && index < o.index);
    }
};

struct SvAccess {
    Sv sv;
    u8 component_index;

    SvAccess(const Sv& sv_, u8 component_index_)
        : sv{sv_}, component_index{component_index_} {}
};

const SvAccess get_sv_access_from_addr(u64 addr);

enum class LoadStoreMode {
    Invalid,

    U8,
    S8,
    U16,
    S16,
    B32,
    B64,
    B96,
    B128,
    UB128,
};

u32 get_load_store_count(LoadStoreMode mode);

enum class ShuffleMode {
    Invalid,

    Index,
    Up,
    Down,
    Bfly,
};

enum class MathFunc {
    Invalid,

    Cos,
    Sin,
    Ex2,
    Lg2,
    Rcp,
    Rsq,
    Rcp64h,
    Rsq64h,
    Sqrt,
};

enum class ComparisonOp {
    Invalid,

    F,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Num,
    Nan,
    LessU,
    EqualU,
    LessEqualU,
    GreaterU,
    NotEqualU,
    GreaterEqualU,
    T,
};

enum class BitwiseOp {
    Invalid,

    And,
    Or,
    Xor,
    PassB,
};

enum class PredOp {
    Invalid,

    False,
    True,
    Zero,
    NotZero,
};

enum class ComponentMask {
    Invalid,

    R,
    G,
    B,
    A,
    RG,
    RA,
    GA,
    BA,
    RGB,
    RGA,
    RBA,
    GBA,
    RGBA,
};

enum class IntegerRoundMode {
    Invalid,

    Pass,
    Round,
    Floor,
    Ceil,
    Trunc,
};

enum class AddressMode {
    Invalid,

    Il,
    Is,
    Isl,
};

enum class TextureComponent {
    Invalid,

    R,
    G,
    B,
    A,
};

enum class TextureQuery {
    Invalid,

    Dimensions,
    TextureType,
    SamplePos,
    SamplerFilter,
    SamplerLod,
    SamplerWrap,
    SamplerBorderColor,
};

enum class IpaOp {
    Invalid,

    Pass,
    Multiply,
    Constant,
    SC,
};

enum class PixelImapType : u8 {
    Unused = 0,
    Constant = 1,
    Perspective = 2,
    ScreenLinear = 3,
};

struct PixelImap {
    PixelImapType x;
    PixelImapType y;
    PixelImapType z;
    PixelImapType w;
};

constexpr u32 PIXEL_IMAP_COUNT = 32;

struct DecompilerContext {
    ShaderType type;
    union {
        struct {
            PixelImap pixel_imaps[PIXEL_IMAP_COUNT];
        } frag;
    };
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp

template <>
struct fmt::formatter<hydra::hw::tegra_x1::gpu::renderer::shader_decomp::reg_t>
    : formatter<string_view> {
    template <typename FormatContext>
    auto
    format(const hydra::hw::tegra_x1::gpu::renderer::shader_decomp::reg_t reg,
           FormatContext& ctx) const {
        if (reg == hydra::hw::tegra_x1::gpu::renderer::shader_decomp::RZ)
            return formatter<string_view>::format("0", ctx);
        return formatter<string_view>::format(
            fmt::format("r{}", hydra::u8(reg)), ctx);
    }
};

template <>
struct fmt::formatter<hydra::hw::tegra_x1::gpu::renderer::shader_decomp::pred_t>
    : formatter<string_view> {
    template <typename FormatContext>
    auto
    format(const hydra::hw::tegra_x1::gpu::renderer::shader_decomp::pred_t pred,
           FormatContext& ctx) const {
        if (pred == hydra::hw::tegra_x1::gpu::renderer::shader_decomp::PT)
            return formatter<string_view>::format("true", ctx);
        return formatter<string_view>::format(
            fmt::format("p{}", hydra::u8(pred)), ctx);
    }
};

template <>
struct fmt::formatter<
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::label_t>
    : formatter<string_view> {
    template <typename FormatContext>
    auto format(
        const hydra::hw::tegra_x1::gpu::renderer::shader_decomp::label_t label,
        FormatContext& ctx) const {
        return formatter<string_view>::format(
            fmt::format("label0x{:x}", hydra::u32(label)), ctx);
    }
};

template <>
struct fmt::formatter<
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::local_t>
    : formatter<string_view> {
    template <typename FormatContext>
    auto format(
        const hydra::hw::tegra_x1::gpu::renderer::shader_decomp::local_t local,
        FormatContext& ctx) const {
        return formatter<string_view>::format(
            fmt::format("%0x{:x}_{}", hydra::u32(local.label), local.id), ctx);
    }
};

template <>
struct fmt::formatter<hydra::hw::tegra_x1::gpu::renderer::shader_decomp::AMem>
    : formatter<string_view> {
    template <typename FormatContext>
    auto
    format(const hydra::hw::tegra_x1::gpu::renderer::shader_decomp::AMem& amem,
           FormatContext& ctx) const {
        return formatter<string_view>::format(
            fmt::format("a[{} + 0x{:x}]", amem.reg, amem.imm), ctx);
    }
};

template <>
struct fmt::formatter<hydra::hw::tegra_x1::gpu::renderer::shader_decomp::CMem>
    : formatter<string_view> {
    template <typename FormatContext>
    auto
    format(const hydra::hw::tegra_x1::gpu::renderer::shader_decomp::CMem& cmem,
           FormatContext& ctx) const {
        return formatter<string_view>::format(
            fmt::format("c{}[{} + 0x{:x}]", cmem.idx, cmem.reg, cmem.imm), ctx);
    }
};

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::DataType, Invalid,
    "invalid", U8, "u8", U16, "u16", U32, "u32", I8, "i8", I16, "i16", I32,
    "i32", F16, "f16", F32, "f32")

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::SvSemantic, Invalid,
    "invalid", Position, "position", UserInOut, "user in out")

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::LoadStoreMode, Invalid,
    "invalid", U8, "u8", S8, "S8", U16, "u16", S16, "s16", B32, "b32", B64,
    "b64", B96, "b96", B128, "b128")

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ShuffleMode, Invalid,
    "invalid", Index, "index", Up, "up", Down, "down", Bfly, "BFLY")

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::MathFunc, Invalid,
    "invalid", Cos, "cos", Sin, "sin", Ex2, "ex2", Lg2, "lg2", Rcp, "rcp", Rsq,
    "rsq", Rcp64h, "rcp64h", Rsq64h, "rsq64h", Sqrt, "sqrt")

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ComparisonOp, Invalid,
    "invalid", F, "f", Less, "less", Equal, "equal", Greater, "greater",
    LessEqual, "less equal", NotEqual, "not equal", GreaterEqual,
    "greater equal", Num, "num", Nan, "nan", LessU, "less U", EqualU, "equal U",
    LessEqualU, "less equal U", GreaterU, "greater U", NotEqualU, "not equal U",
    GreaterEqualU, "greater equal U", T, "t")

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::BitwiseOp, Invalid,
    "invalid", And, "and", Or, "or", Xor, "xor", PassB, "pass B")

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::PredOp, Invalid,
    "invalid", False, "false", True, "true", Zero, "zero", NotZero, "not zero")

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ComponentMask, Invalid,
    "invalid", R, "r", G, "g", B, "b", A, "a", RG, "rg", RA, "ra", GA, "ga", BA,
    "ba", RGB, "rgb", RGA, "rga", RBA, "rba", GBA, "gba", RGBA, "rgba")

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::IntegerRoundMode,
    Invalid, "invalid", Pass, "pass", Round, "round", Floor, "floor", Ceil,
    "ceil", Trunc, "trunc")

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::AddressMode, Invalid,
    "invalid", Il, "il", Is, "is", Isl, "isl")

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::TextureComponent,
    Invalid, "invalid", R, "r", G, "g", B, "b", A, "a")

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::TextureQuery, Invalid,
    "invalid", Dimensions, "dimensions", TextureType, "texture type", SamplePos,
    "sample pos", SamplerFilter, "sampler filter", SamplerLod, "sampler LOD",
    SamplerWrap, "sampler wrap", SamplerBorderColor, "sampler border color")

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::renderer::shader_decomp::IpaOp,
                       Invalid, "invalid", Pass, "pass", Multiply, "multiply",
                       Constant, "constant", SC, "SC")
