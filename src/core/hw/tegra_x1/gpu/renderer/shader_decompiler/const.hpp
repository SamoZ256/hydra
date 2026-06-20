#pragma once

#include "core/hw/tegra_x1/gpu/engines/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

using instruction_t = u64;
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

constexpr reg_t RZ = 0xff;
constexpr pred_t PT = 0x7;

struct AMem {
    reg_t reg;
    u64 imm;
    bool is_input;

    AMem(reg_t reg_, u64 imm_, bool is_input_)
        : reg{reg_}, imm{imm_}, is_input{is_input_} {}

    bool operator==(const AMem& other) const {
        return reg == other.reg && imm == other.imm;
    }
};

struct CMem {
    u32 idx;
    reg_t reg;
    u64 imm; // TODO: signed?

    CMem(u32 idx_, reg_t reg_, u64 imm_) : idx{idx_}, reg{reg_}, imm{imm_} {}

    bool operator==(const CMem& other) const {
        return idx == other.idx && reg == other.reg && imm == other.imm;
    }
};

struct PredCond {
    pred_t pred;
    bool not_;
    bool never;

    PredCond(pred_t pred_, bool not_, bool never_)
        : pred{pred_}, not_{not_}, never{never_} {}

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

    bool operator==(const Sv& o) const {
        return semantic == o.semantic && index == o.index;
    }

    bool operator<(const Sv& o) const {
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

enum class TextureType {
    _1D,
    _1DArray,
    _2D,
    _2DArray,
    _3D,
    _3DArray,
    Cube,
    CubeArray,
};

inline bool IsTextureArray(TextureType type) {
    return type == TextureType::_1DArray || type == TextureType::_2DArray ||
           type == TextureType::_3DArray || type == TextureType::CubeArray;
}

enum class TextureSampleFlags {
    None = 0,
    IntCoords = BIT(0),
    DepthCompare = BIT(1),
    Lod = BIT(2),
};
ENABLE_ENUM_BITWISE_OPERATORS(TextureSampleFlags)

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

    PixelImapType GetFirstUsedType() const {
        if (x != PixelImapType::Unused)
            return x;
        if (y != PixelImapType::Unused)
            return y;
        if (z != PixelImapType::Unused)
            return z;
        if (w != PixelImapType::Unused)
            return w;
        return PixelImapType::Unused;
    }
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
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::SvSemantic, Invalid,
    "invalid", Position, "position", UserInOut, "user in out")
