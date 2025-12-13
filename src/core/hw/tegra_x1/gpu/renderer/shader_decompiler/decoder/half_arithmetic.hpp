#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

// TODO: check all these instructions

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

enum class HalfOutputFormat {
    F16 = 0,
    F32 = 1,
    MrgH0 = 2,
    MrgH1 = 3,
};

enum class HalfSwizzle {
    F16 = 0,
    F32 = 1,
    H0H0 = 2,
    H1H1 = 3,
};

union InstHadd2Base {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<bool, 39, 1> ftz;
    BitField64<bool, 44, 1> abs_a;
    BitField64<HalfSwizzle, 47, 2> swizzle_a;
    BitField64<HalfOutputFormat, 49, 2> out_fmt;
};

union InstHadd2R {
    InstHadd2Base base;
    BitField64<reg_t, 20, 8> src_b;
    BitField64<HalfSwizzle, 28, 2> swizzle_b;
    BitField64<bool, 30, 1> abs_b;
    BitField64<bool, 31, 1> neg_b;
    BitField64<bool, 32, 1> sat;
    BitField64<bool, 38, 1> neg_a;
};

void EmitHadd2R(DecoderContext& context, InstHadd2R inst);

union InstHadd2C {
    InstHadd2Base base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
    BitField64<bool, 43, 1> neg_a;
    BitField64<bool, 52, 1> sat;
    BitField64<bool, 54, 1> abs_b;
    BitField64<bool, 56, 1> neg_b;
};

void EmitHadd2C(DecoderContext& context, InstHadd2C inst);

union InstHadd2I {
    InstHadd2Base base;
    BitField64<u32, 20, 10> h0_imm10;
    BitField64<u32, 30, 9> h1_imm10_0;
    BitField64<bool, 43, 1> neg_a;
    BitField64<u32, 56, 1> h1_imm10_9;
    BitField64<bool, 52, 1> sat;

    u32 GetH1Imm10() const {
        return h1_imm10_0.Get() | (h1_imm10_9.Get() << 9);
    }
};

void EmitHadd2I(DecoderContext& context, InstHadd2I inst);

union InstHadd2_32I {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 32> imm;
    BitField64<bool, 52, 1> sat;
    BitField64<HalfSwizzle, 53, 2> swizzle_a;
    BitField64<bool, 55, 1> ftz;
    BitField64<bool, 56, 1> neg_a;
};

void EmitHadd2_32I(DecoderContext& context, InstHadd2_32I inst);

union InstHmul2Base {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 39, 2> fmz; // TODO
    BitField64<bool, 44, 1> abs_a;
    BitField64<HalfSwizzle, 47, 2> swizzle_a;
    BitField64<HalfOutputFormat, 49, 2> out_fmt;
};

union InstHmul2R {
    InstHmul2Base base;
    BitField64<reg_t, 20, 8> src_b;
    BitField64<HalfSwizzle, 28, 2> swizzle_b;
    BitField64<bool, 30, 1> abs_b;
    BitField64<bool, 31, 1> neg_b;
    BitField64<bool, 32, 1> sat;
};

void EmitHmul2R(DecoderContext& context, InstHmul2R inst);

union InstHmul2C {
    InstHmul2Base base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
    BitField64<bool, 43, 1> neg_a;
    BitField64<bool, 52, 1> sat;
    BitField64<bool, 54, 1> abs_b;
};

void EmitHmul2C(DecoderContext& context, InstHmul2C inst);

union InstHmul2I {
    InstHmul2Base base;
    BitField64<u32, 20, 10> h0_imm10;
    BitField64<u32, 30, 9> h1_imm10_0;
    BitField64<bool, 43, 1> neg_a;
    BitField64<u32, 56, 1> h1_imm10_9;
    BitField64<bool, 52, 1> sat;

    u32 GetH1Imm10() const {
        return h1_imm10_0.Get() | (h1_imm10_9.Get() << 9);
    }
};

void EmitHmul2I(DecoderContext& context, InstHmul2I inst);

union InstHmul2_32I {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 32> imm;
    BitField64<bool, 52, 1> sat;
    BitField64<HalfSwizzle, 53, 2> swizzle_a;
    BitField64<u32, 55, 2> fmz; // TODO
};

void EmitHmul2_32I(DecoderContext& context, InstHmul2_32I inst);

union InstHfma2Base {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<HalfSwizzle, 47, 2> swizzle_a;
    BitField64<HalfOutputFormat, 49, 2> out_fmt;
};

union InstHfma2R {
    InstHfma2Base base;
    BitField64<reg_t, 20, 8> src_b;
    BitField64<HalfSwizzle, 28, 2> swizzle_b;
    BitField64<bool, 30, 1> neg_c;
    BitField64<bool, 31, 1> neg_b;
    BitField64<bool, 32, 1> sat;
    BitField64<HalfSwizzle, 35, 2> swizzle_c;
    BitField64<u32, 37, 2> fmz; // TODO
    BitField64<reg_t, 39, 8> src_c;
};

void EmitHfma2R(DecoderContext& context, InstHfma2R inst);

union InstHfma2RC {
    InstHfma2Base base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
    BitField64<reg_t, 39, 8> src_b;
    BitField64<bool, 51, 1> neg_c;
    BitField64<bool, 52, 1> sat;
    BitField64<HalfSwizzle, 53, 2> swizzle_b;
    BitField64<bool, 56, 1> neg_b;
    BitField64<u32, 57, 2> fmz; // TODO
};

void EmitHfma2RC(DecoderContext& context, InstHfma2RC inst);

union InstHfma2C {
    InstHfma2Base base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
    BitField64<reg_t, 39, 8> src_c;
    BitField64<bool, 51, 1> neg_c;
    BitField64<bool, 52, 1> sat;
    BitField64<HalfSwizzle, 53, 2> swizzle_c;
    BitField64<bool, 56, 1> neg_b;
    BitField64<u32, 57, 2> fmz; // TODO
};

void EmitHfma2C(DecoderContext& context, InstHfma2C inst);

union InstHfma2I {
    InstHfma2Base base;
    BitField64<u32, 20, 10> h0_imm10;
    BitField64<u32, 30, 9> h1_imm10_0;
    BitField64<reg_t, 39, 8> src_c;
    BitField64<bool, 51, 1> neg_c;
    BitField64<bool, 52, 1> sat;
    BitField64<HalfSwizzle, 53, 2> swizzle_c;
    BitField64<u32, 56, 1> h1_imm10_9;
    BitField64<u32, 57, 2> fmz; // TODO

    u32 GetH1Imm10() const {
        return h1_imm10_0.Get() | (h1_imm10_9.Get() << 9);
    }
};

void EmitHfma2I(DecoderContext& context, InstHfma2I inst);

union InstHfma2_32I {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 32> imm;
    BitField64<bool, 52, 1> neg_c;
    BitField64<HalfSwizzle, 53, 2> swizzle_a;
    BitField64<u32, 55, 2> fmz; // TODO
};

void EmitHfma2_32I(DecoderContext& context, InstHfma2_32I inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
