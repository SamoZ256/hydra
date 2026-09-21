#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

enum class LogicOp {
    And = 0,
    Or = 1,
    Xor = 2,
    PassB = 3,
};

enum class PredicateOp {
    F = 0,
    T = 1,
    Z = 2,
    Nz = 3,
};

union InstLopBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<bool, 39, 1> inv_a;
    BitField64<bool, 40, 1> inv_b;
    BitField64<LogicOp, 41, 2> op;
    BitField64<bool, 43, 1> x;
    BitField64<PredicateOp, 44, 2> pred_op;
    BitField64<bool, 47, 1> write_cc;
    BitField64<pred_t, 48, 3> dst_pred;
};

DEFINE_INST_SRC2_VARIANTS(Lop);

union InstLop32I {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 32> imm;
    BitField64<bool, 52, 1> write_cc;
    BitField64<LogicOp, 53, 2> op;
    BitField64<bool, 55, 1> inv_a;
    BitField64<bool, 56, 1> inv_b;
    BitField64<bool, 57, 1> x;
};

void emitLop32I(DecoderContext& context, InstLop32I inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
