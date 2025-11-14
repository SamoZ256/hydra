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
                      "Couldn't extract " #e " (" #name                        \
                      ") operand from instruction 0x{:016x}",                  \
                      inst);                                                   \
            return e::Invalid;                                                 \
        }                                                                      \
    }

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

DEFINE_OPERAND_TABLE(0400_0, BitwiseOp, 0x0060000000000000, 0x0000000000000000,
                     And, 0x0020000000000000, Or, 0x0040000000000000, Xor,
                     0x0060000000000000, PassB)

DEFINE_OPERAND_TABLE(5c40_0, BitwiseOp, 0x0000060000000000, 0x0000000000000000,
                     And, 0x0000020000000000, Or, 0x0000040000000000, Xor,
                     0x0000060000000000, PassB)

DEFINE_OPERAND_TABLE(5c40_1, PredOp, 0x0000300000000000, 0x0000000000000000,
                     False, 0x0000100000000000, True, 0x0000200000000000, Zero,
                     0x0000300000000000, NotZero)

DEFINE_OPERAND_TABLE(e000_0, IpaOp, 0x00c0000000000000, 0x0000000000000000,
                     Pass, 0x0040000000000000, Multiply, 0x0080000000000000,
                     Constant, 0x00c0000000000000, SC)

DEFINE_OPERAND_TABLE(eed0sz, LoadStoreMode, 0x0007000000000000,
                     0x0000000000000000, U8, 0x0001000000000000, S8,
                     0x0002000000000000, U16, 0x0003000000000000, S16,
                     0x0004000000000000, B32, 0x0005000000000000, B64,
                     0x0006000000000000, B128, 0x0007000000000000, UB128)

DEFINE_OPERAND_TABLE(eff0_0, LoadStoreMode, 0x0001800000000000,
                     0x0000000000000000, B32, 0x0000800000000000, B64,
                     0x0001000000000000, B96, 0x0001800000000000, B128)

DEFINE_OPERAND_TABLE(5080_0, MathFunc, 0x0000000000f00000, 0x0000000000000000,
                     Cos, 0x0000000000100000, Sin, 0x0000000000200000, Ex2,
                     0x0000000000300000, Lg2, 0x0000000000400000, Rcp,
                     0x0000000000500000, Rsq, 0x0000000000600000, Rcp64h,
                     0x0000000000700000, Rsq64h, 0x0000000000800000, Sqrt)

DEFINE_OPERAND_TABLE(5090_0, BitwiseOp, 0x0000000003000000, 0x0000000000000000,
                     And, 0x0000000001000000, Or, 0x0000000002000000, Xor)

DEFINE_OPERAND_TABLE(5bb0_0, ComparisonOp, 0x000f000000000000,
                     0x0000000000000000, F, 0x0001000000000000, Less,
                     0x0002000000000000, Equal, 0x0003000000000000, LessEqual,
                     0x0004000000000000, Greater, 0x0005000000000000, NotEqual,
                     0x0006000000000000, GreaterEqual, 0x0007000000000000, Num,
                     0x0008000000000000, Nan, 0x0009000000000000, LessU,
                     0x000a000000000000, EqualU, 0x000b000000000000, LessEqualU,
                     0x000c000000000000, GreaterU, 0x000d000000000000,
                     NotEqualU, 0x000e000000000000, GreaterEqualU,
                     0x000f000000000000, T)

DEFINE_OPERAND_TABLE(5bb0_1, BitwiseOp, 0x0000600000000000, 0x0000000000000000,
                     And, 0x0000200000000000, Or, 0x0000400000000000, Xor)

DEFINE_OPERAND_TABLE(5ce0_0, DataType, 0x0000000000001300, 0x0000000000000000,
                     U8, 0x0000000000000100, U16, 0x0000000000000200, U32,
                     0x0000000000001000, I8, 0x0000000000001100, I16,
                     0x0000000000001200, I32)

DEFINE_OPERAND_TABLE(5ce0_1, DataType, 0x0000000000002c00, 0x0000000000000000,
                     U8, 0x0000000000000400, U16, 0x0000000000000800, U32,
                     0x0000000000002000, I8, 0x0000000000002400, I16,
                     0x0000000000002800, I32)

// TODO: 64-bit types
DEFINE_OPERAND_TABLE(5cb0_2, DataType, 0x0000000000001300, 0x0000000000000000,
                     U8, 0x0000000000000100, U16, 0x0000000000000200, U32,
                     0x0000000000001000, I8, 0x0000000000001100, I16,
                     0x0000000000001200, I32)

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

DEFINE_OPERAND_TABLE(5b60_0, ComparisonOp, 0x000e000000000000,
                     0x0000000000000000, F, 0x0002000000000000, Less,
                     0x0004000000000000, Equal, 0x0006000000000000, LessEqual,
                     0x0008000000000000, Greater, 0x000a000000000000, NotEqual,
                     0x000c000000000000, GreaterEqual, 0x000e000000000000, T)

DEFINE_OPERAND_TABLE(5c30_0, DataType, 0x0001000000000000, 0x0000000000000000,
                     U32, 0x0001000000000000, I32)

DEFINE_OPERAND_TABLE(ef10_0, ShuffleMode, 0x00000000c0000000,
                     0x0000000000000000, Index, 0x0000000040000000, Up,
                     0x0000000080000000, Down, 0x00000000c0000000, Bfly)

DEFINE_OPERAND_TABLE(d200_2_ff, ComponentMask, 0x001c000ff0000000,
                     0x0000000ff0000000, R, 0x0004000ff0000000, G,
                     0x0008000ff0000000, B, 0x000c000ff0000000, A,
                     0x0010000ff0000000, RG, 0x0014000ff0000000, RA,
                     0x0018000ff0000000, GA, 0x001c000ff0000000, BA)

DEFINE_OPERAND_TABLE(d200_2_00, ComponentMask, 0x001c000000000000,
                     0x0000000000000000, RGB, 0x0004000000000000, RGA,
                     0x0008000000000000, RBA, 0x000c000000000000, GBA,
                     0x0010000000000000, RGBA)

DEFINE_OPERAND_TABLE(5cb0_1, IntegerRoundMode, 0x0000018000000000,
                     0x0000000000000000, Pass, 0x0000008000000000, Floor,
                     0x0000010000000000, Ceil, 0x0000018000000000, Trunc)

// TODO: is there Pass twice?
DEFINE_OPERAND_TABLE(5ca8_0, IntegerRoundMode, 0x0000058000000000,
                     0x0000000000000000, Pass, 0x0000018000000000, Pass,
                     0x0000040000000000, Round, 0x0000048000000000, Floor,
                     0x0000050000000000, Ceil, 0x0000058000000000, Trunc)

DEFINE_OPERAND_TABLE(ef90_0, AddressMode, 0x0000300000000000,
                     0x0000100000000000, Il, 0x0000200000000000, Is,
                     0x0000300000000000, Isl)

DEFINE_OPERAND_TABLE(ef90sz, LoadStoreMode, 0x0007000000000000,
                     0x0000000000000000, U8, 0x0001000000000000, S8,
                     0x0002000000000000, U16, 0x0003000000000000, S16,
                     0x0004000000000000, B32, 0x0005000000000000, B64)

DEFINE_OPERAND_TABLE(df50_0, TextureQuery, 0x000000000fc00000,
                     0x0000000000400000, Dimensions, 0x0000000000800000,
                     TextureType, 0x0000000001400000, SamplePos,
                     0x0000000004000000, SamplerFilter, 0x0000000004800000,
                     SamplerLod, 0x0000000005000000, SamplerWrap,
                     0x0000000005800000, SamplerBorderColor)

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
