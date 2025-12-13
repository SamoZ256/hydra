#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/comparison_helper.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

enum class FloatCmpOp {
    F = 0,
    Lt = 1,
    Eq = 2,
    Le = 3,
    Gt = 4,
    Ne = 5,
    Ge = 6,
    Num = 7,
    Nan = 8,
    Ltu = 9,
    Equ = 10,
    Leu = 11,
    Gtu = 12,
    Neu = 13,
    Geu = 14,
    T = 15,
};

union InstFsetBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<pred_t, 39, 3> src_pred;
    BitField64<bool, 42, 1> src_pred_inv;
    BitField64<bool, 43, 1> neg_a;
    BitField64<bool, 44, 1> abs_b;
    BitField64<BoolOp, 45, 2> b_op;
    BitField64<bool, 47, 1> write_cc;
    BitField64<FloatCmpOp, 48, 4> op;
    BitField64<bool, 52, 1> b_float;
    BitField64<bool, 53, 1> neg_b;
    BitField64<bool, 54, 1> abs_a;
    BitField64<bool, 55, 1> ftz;
};

union InstFsetR {
    InstFsetBase base;
    BitField64<reg_t, 20, 8> src_b;
};

void EmitFsetR(DecoderContext& context, InstFsetR inst);

union InstFsetC {
    InstFsetBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
};

void EmitFsetC(DecoderContext& context, InstFsetC inst);

union InstFsetI {
    InstFsetBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
};

void EmitFsetI(DecoderContext& context, InstFsetI inst);

union InstFsetpBase {
    BitField64<pred_t, 0, 3> dst_inv_pred;
    BitField64<pred_t, 3, 3> dst_pred;
    BitField64<bool, 6, 1> neg_b;
    BitField64<bool, 7, 1> abs_a;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<pred_t, 39, 3> src_pred;
    BitField64<bool, 42, 1> src_pred_inv;
    BitField64<bool, 43, 1> neg_a;
    BitField64<bool, 44, 1> abs_b;
    BitField64<BoolOp, 45, 2> b_op;
    BitField64<bool, 47, 1> ftz;
    BitField64<FloatCmpOp, 48, 4> op;
};

union InstFsetpR {
    InstFsetpBase base;
    BitField64<reg_t, 20, 8> src_b;
};

void EmitFsetpR(DecoderContext& context, InstFsetpR inst);

union InstFsetpC {
    InstFsetpBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
};

void EmitFsetpC(DecoderContext& context, InstFsetpC inst);

union InstFsetpI {
    InstFsetpBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
};

void EmitFsetpI(DecoderContext& context, InstFsetpI inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
