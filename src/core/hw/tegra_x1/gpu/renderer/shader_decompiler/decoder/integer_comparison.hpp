#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/comparison_helper.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

enum class IntCmpOp {
    F = 0,
    Lt = 1,
    Eq = 2,
    Le = 3,
    Gt = 4,
    Ne = 5,
    Ge = 6,
    T = 7,
};

union InstIsetBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<pred_t, 39, 3> src_pred;
    BitField64<bool, 42, 1> src_pred_inv;
    BitField64<bool, 43, 1> x;
    BitField64<bool, 44, 1> b_float;
    BitField64<BoolOp, 45, 2> b_op;
    BitField64<bool, 47, 1> write_cc;
    BitField64<bool, 48, 1> is_signed;
    BitField64<IntCmpOp, 49, 3> op;
};

union InstIsetR {
    InstIsetBase base;
    BitField64<reg_t, 20, 8> src_b;
};

void EmitIsetR(DecoderContext& context, InstIsetR inst);

union InstIsetC {
    InstIsetBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
};

void EmitIsetC(DecoderContext& context, InstIsetC inst);

union InstIsetI {
    InstIsetBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
};

void EmitIsetI(DecoderContext& context, InstIsetI inst);

union InstIsetpBase {
    BitField64<pred_t, 0, 3> dst_inv_pred;
    BitField64<pred_t, 3, 3> dst_pred;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<pred_t, 39, 3> src_pred;
    BitField64<bool, 42, 1> src_pred_inv;
    BitField64<bool, 43, 1> x;
    BitField64<BoolOp, 45, 2> b_op;
    BitField64<bool, 48, 1> is_signed;
    BitField64<IntCmpOp, 49, 3> op;
};

union InstIsetpR {
    InstIsetpBase base;
    BitField64<reg_t, 20, 8> src_b;
};

void EmitIsetpR(DecoderContext& context, InstIsetpR inst);

union InstIsetpC {
    InstIsetpBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
};

void EmitIsetpC(DecoderContext& context, InstIsetpC inst);

union InstIsetpI {
    InstIsetpBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
};

void EmitIsetpI(DecoderContext& context, InstIsetpI inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
