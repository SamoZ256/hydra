#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/comparison_helper.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

union InstHsetp2Base {
    BitField64<pred_t, 0, 3> dst_inv_pred;
    BitField64<pred_t, 3, 3> dst_pred;
    BitField64<bool, 6, 1> ftz;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<pred_t, 39, 3> src_pred;
    BitField64<bool, 42, 1> src_pred_inv;
    BitField64<bool, 43, 1> neg_a;
    BitField64<bool, 44, 1> abs_a;
    BitField64<BoolOp, 45, 2> b_op;
    BitField64<HalfSwizzle, 47, 2> swizzle_a;
};

union InstHsetp2R {
    InstHsetp2Base base;
    BitField64<reg_t, 20, 8> src_b;
    BitField64<HalfSwizzle, 28, 2> swizzle_b;
    BitField64<bool, 30, 1> abs_b;
    BitField64<bool, 31, 1> neg_b;
    BitField64<FloatCmpOp, 35, 4> op;
    BitField64<bool, 49, 1> h_and;
};

void emitHsetp2R(DecoderContext& context, InstHsetp2R inst);

union InstHsetp2C {
    InstHsetp2Base base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
    BitField64<bool, 43, 1> neg_a;
    BitField64<FloatCmpOp, 49, 4> op;
    BitField64<bool, 53, 1> h_and;
    BitField64<bool, 54, 1> abs_b;
    BitField64<bool, 56, 1> neg_b;
};

void emitHsetp2C(DecoderContext& context, InstHsetp2C inst);

union InstHsetp2I {
    InstHsetp2Base base;
    BitField64<u32, 20, 10> h0_imm10;
    BitField64<u32, 30, 9> h1_imm10_0;
    BitField64<bool, 43, 1> neg_a;
    BitField64<FloatCmpOp, 49, 4> op;
    BitField64<bool, 53, 1> h_and;
    BitField64<u32, 56, 1> h1_imm10_9;

    u32 getH1Imm10() const {
        return h1_imm10_0.get() | (h1_imm10_9.get() << 9);
    }
};

void emitHsetp2I(DecoderContext& context, InstHsetp2I inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
