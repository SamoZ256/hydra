#pragma once

#include "hw/tegra_x1/gpu/renderer/shader_decompiler/const.hpp"

#define OPERAND_TABLE_ENTRY(e, value, operand)                                 \
    case value:                                                                \
        return e::operand;

#define DEFINE_OPERAND_TABLE(name, e, mask, ...)                               \
    inline e get_operand_##name(u64 inst) {                                    \
        switch (inst & mask) {                                                 \
            FOR_EACH_1_2(OPERAND_TABLE_ENTRY, e, __VA_ARGS__)                  \
        default:                                                               \
            LOG_ERROR(ShaderDecompiler,                                        \
                      "Couldn't extract opernad from instruction 0x{:016x}",   \
                      inst);                                                   \
            return e::Invalid;                                                 \
        }                                                                      \
    }

namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler {

DEFINE_OPERAND_TABLE(eff0_0, LoadStoreMode, 0x0001800000000000,
                     0x0000000000000000, B32, 0x0000800000000000, B64,
                     0x0001000000000000, B96, 0x0001800000000000, B128)

DEFINE_OPERAND_TABLE(5080_0, MathFunc, 0x0000000000f00000, 0x0000000000000000,
                     Cos, 0x0000000000100000, Sin, 0x0000000000200000, Ex2,
                     0x0000000000300000, Lg2, 0x0000000000400000, Rcp,
                     0x0000000000500000, Rsq, 0x0000000000600000, Rcp64h,
                     0x0000000000700000, Rsq64h, 0x0000000000800000, Sqrt)

} // namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler
