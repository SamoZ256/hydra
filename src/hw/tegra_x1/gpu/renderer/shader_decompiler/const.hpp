#pragma once

#include "hw/tegra_x1/gpu/engines/const.hpp"
#include "hw/tegra_x1/gpu/renderer/const.hpp"

#define INST0(value, mask) if ((inst & mask##ull) == value##ull)
#define INST(value, mask) else INST0(value, mask)

namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler {

typedef u8 reg_t;

constexpr reg_t RZ = 255;

struct AMem {
    reg_t reg;
    u64 imm;
};

struct CMem {
    u32 idx;
    reg_t reg;
    u64 imm;
};

enum class OperandType {
    Register,
    Immediate,
    AttributeMemory,
    ConstMemory,
};

struct Operand {
    OperandType type;
    union {
        reg_t reg;
        u32 imm;
        AMem amem;
        CMem cmem;
    };

    static Operand Register(reg_t reg_) {
        return Operand{.type = OperandType::Register, .reg = reg_};
    }

    static Operand Immediate(u32 imm_) {
        return Operand{.type = OperandType::Immediate, .imm = imm_};
    }

    static Operand AttributeMemory(const AMem& amem) {
        return Operand{.type = OperandType::AttributeMemory, .amem = amem};
    }

    static Operand ConstMemory(const CMem& cmem) {
        return Operand{.type = OperandType::ConstMemory, .cmem = cmem};
    }
};

enum class DataType {
    None,
    Int,
    UInt,
    Float,
};

inline DataType to_data_type(Engines::VertexAttribType vertex_attrib_type) {
    switch (vertex_attrib_type) {
    case Engines::VertexAttribType::Snorm:
        return DataType::Float;
    case Engines::VertexAttribType::Unorm:
        return DataType::Float;
    case Engines::VertexAttribType::Sint:
        return DataType::Int;
    case Engines::VertexAttribType::Uint:
        return DataType::UInt;
    case Engines::VertexAttribType::Sscaled:
        return DataType::Int; // TODO: use float if the Rendered backend
                              // supports scaled attributes
    case Engines::VertexAttribType::Uscaled:
        return DataType::UInt; // TODO: use float if the Rendered backend
                               // supports scaled attributes
    case Engines::VertexAttribType::Float:
        return DataType::Float;
    default:
        LOG_WARNING(ShaderDecompiler, "Unknown vertex attrib type {}",
                    vertex_attrib_type);
        return DataType::None;
    }
}

inline DataType to_data_type(TextureFormat format) {
    // TODO: implement
    return DataType::Float;
}

enum class SVSemantic {
    Invalid,
    Position,
    UserInOut,
    // TODO: more
};

struct SV {
    SVSemantic semantic;
    u8 index;
    u8 component_index;
    // TODO: more?

    SV(SVSemantic semantic_, u8 index_ = invalid<u8>(),
       u8 component_index_ = invalid<u8>())
        : semantic{semantic_}, index{index_},
          component_index{component_index_} {}
};

enum class LoadStoreMode {
    Invalid,
    B32,
    B64,
    B96,
    B128,
};

inline u32 get_load_store_count(LoadStoreMode mode) {
    switch (mode) {
    case LoadStoreMode::B32:
        return 1;
    case LoadStoreMode::B64:
        return 2;
    case LoadStoreMode::B96:
        return 3;
    case LoadStoreMode::B128:
        return 4;
    default:
        LOG_ERROR(ShaderDecompiler, "Unknown load store mode {}", mode);
        return 0;
    }
}

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

inline const SV get_sv_from_addr(u64 addr) {
    ASSERT_ALIGNMENT_DEBUG(addr, 4, ShaderDecompiler, "Address");

    struct SVBase {
        SVSemantic semantic;
        u64 base_addr;
    };

    static constexpr SVBase bases[] = {
        {SVSemantic::UserInOut, 0x80},
        {SVSemantic::Position, 0x70},
    };

    for (const auto& base : bases) {
        if (addr >= base.base_addr) {
            return SV(base.semantic,
                      static_cast<u8>((addr - base.base_addr) >> 4),
                      static_cast<u8>((addr >> 2) & 0x3));
        }
    }

    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "SV address 0x{:02x}", addr);

    return SVSemantic::Invalid;
}

} // namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler

ENABLE_ENUM_FORMATTING(
    Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler::OperandType, Register,
    "register", Immediate, "immediate", AttributeMemory, "attribute memory",
    ConstMemory, "const memory")

ENABLE_ENUM_FORMATTING(
    Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler::DataType, Int, "int",
    UInt, "uint", Float, "float")

ENABLE_ENUM_FORMATTING(
    Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler::SVSemantic, Invalid,
    "invalid", Position, "position", UserInOut, "user in out")

ENABLE_ENUM_FORMATTING(
    Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler::LoadStoreMode, Invalid,
    "invalid", B32, "b32", B64, "b64", B96, "b96", B128, "b128")

ENABLE_ENUM_FORMATTING(
    Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler::MathFunc, Cos, "cos",
    Sin, "sin", Ex2, "ex2", Lg2, "lg2", Rcp, "rcp", Rsq, "rsq", Rcp64h,
    "rcp64h", Rsq64h, "rsq64h", Sqrt, "sqrt")
