#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/comparison_helper.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

union InstPset {
    BitField64<reg_t, 0, 8> dst;
    BitField64<pred_t, 12, 3> src_a_pred;
    BitField64<bool, 15, 1> src_a_pred_inv;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<BoolOp, 24, 2> b_op_ab;
    BitField64<pred_t, 29, 3> src_b_pred;
    BitField64<bool, 32, 1> src_b_pred_inv;
    BitField64<pred_t, 39, 3> src_c_pred;
    BitField64<bool, 42, 1> src_c_pred_inv;
    BitField64<bool, 44, 1> b_float;
    BitField64<BoolOp, 45, 2> b_op_c;
    BitField64<bool, 47, 1> write_cc;
};

void emitPset(DecoderContext& context, InstPset inst);

union InstPsetp {
    BitField64<pred_t, 0, 3> dst_inv_pred;
    BitField64<pred_t, 3, 3> dst_pred;
    BitField64<pred_t, 12, 3> src_a_pred;
    BitField64<bool, 15, 1> src_a_pred_inv;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<BoolOp, 24, 2> b_op_ab;
    BitField64<pred_t, 29, 3> src_b_pred;
    BitField64<bool, 32, 1> src_b_pred_inv;
    BitField64<pred_t, 39, 3> src_c_pred;
    BitField64<bool, 42, 1> src_c_pred_inv;
    BitField64<BoolOp, 45, 2> b_op_c;
};

void emitPsetp(DecoderContext& context, InstPsetp inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
