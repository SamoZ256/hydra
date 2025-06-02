#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/const.hpp"

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

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

DEFINE_OPERAND_TABLE(eff0_0, LoadStoreMode, 0x0001800000000000,
                     0x0000000000000000, B32, 0x0000800000000000, B64,
                     0x0001000000000000, B96, 0x0001800000000000, B128)

DEFINE_OPERAND_TABLE(5080_0, MathFunc, 0x0000000000f00000, 0x0000000000000000,
                     Cos, 0x0000000000100000, Sin, 0x0000000000200000, Ex2,
                     0x0000000000300000, Lg2, 0x0000000000400000, Rcp,
                     0x0000000000500000, Rsq, 0x0000000000600000, Rcp64h,
                     0x0000000000700000, Rsq64h, 0x0000000000800000, Sqrt)

DEFINE_OPERAND_TABLE(5090_0, BinaryOperator, 0x0000000003000000,
                     0x0000000000000000, And, 0x0000000001000000, Or,
                     0x0000000002000000, Xor)

DEFINE_OPERAND_TABLE(5bb0_0, ComparisonOperator, 0x000f000000000000,
                     0x0000000000000000, F, 0x0001000000000000, Less,
                     0x0002000000000000, Equal, 0x0003000000000000, LessEqual,
                     0x0004000000000000, Greater, 0x0005000000000000, NotEqual,
                     0x0006000000000000, GreaterEqual, 0x0007000000000000, Num,
                     0x0008000000000000, Nan, 0x0009000000000000, LessU,
                     0x000a000000000000, EqualU, 0x000b000000000000, LessEqualU,
                     0x000c000000000000, GreaterU, 0x000d000000000000,
                     NotEqualU, 0x000e000000000000, GreaterEqualU,
                     0x000f000000000000, T)

DEFINE_OPERAND_TABLE(5bb0_1, BinaryOperator, 0x0000600000000000,
                     0x0000000000000000, And, 0x0000200000000000, Or,
                     0x0000400000000000, Xor)

DEFINE_OPERAND_TABLE(5ce0_0, DataType, 0x0000000000001300, 0x0000000000000000,
                     U8, 0x0000000000000100, U16, 0x0000000000000200, U32,
                     0x0000000000001000, I8, 0x0000000000001100, I16,
                     0x0000000000001200, I32)

DEFINE_OPERAND_TABLE(5ce0_1, DataType, 0x0000000000002c00, 0x0000000000000000,
                     U8, 0x0000000000000400, U16, 0x0000000000000800, U32,
                     0x0000000000002000, I8, 0x0000000000002400, I16,
                     0x0000000000002800, I32)

// TODO: 64-bit types
DEFINE_OPERAND_TABLE(5cb0_0, DataType, 0x0000000000000c00, 0x0000000000000400,
                     F16, 0x0000000000000800, F32)

// TODO: 64-bit types
DEFINE_OPERAND_TABLE(5cb8_0, DataType, 0x0000000000000300, 0x0000000000000100,
                     F16, 0x0000000000000200, F32)

// TODO: 64-bit types
DEFINE_OPERAND_TABLE(5cb8_1, DataType, 0x0000000000002c00, 0x0000000000000000,
                     U8, 0x0000000000000400, U16, 0x0000000000000800, U32,
                     0x0000000000002000, I8, 0x0000000000002400, I16,
                     0x0000000000002800, I32)

DEFINE_OPERAND_TABLE(5b60_0, ComparisonOperator, 0x000e000000000000,
                     0x0000000000000000, F, 0x0002000000000000, Less,
                     0x0004000000000000, Equal, 0x000600000000000, LessEqual,
                     0x0008000000000000, Greater, 0x000a000000000000, NotEqual,
                     0x000c000000000000, GreaterEqual, 0x000e000000000000, T)

DEFINE_OPERAND_TABLE(5c30_0, DataType, 0x0001000000000000, 0x0000000000000000,
                     U32, 0x0001000000000000, I32)

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp
