#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

union InstFaddBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<RoundMode, 39, 2> round_mode;
    BitField64<bool, 44, 1> ftz;
    BitField64<bool, 45, 1> neg_b;
    BitField64<bool, 46, 1> abs_a;
    BitField64<bool, 47, 1> write_cc;
    BitField64<bool, 48, 1> neg_a;
    BitField64<bool, 49, 1> abs_b;
    BitField64<bool, 50, 1> sat;
};

union InstFaddR {
    InstFaddBase base;
    BitField64<reg_t, 20, 8> src_b;
};

void EmitFaddR(DecoderContext& context, InstFaddR inst);

union InstFaddC {
    InstFaddBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
};

void EmitFaddC(DecoderContext& context, InstFaddC inst);

union InstFaddI {
    InstFaddBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
};

void EmitFaddI(DecoderContext& context, InstFaddI inst);

union InstFadd32I {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 32> imm;
    BitField64<bool, 52, 1> write_cc;
    BitField64<bool, 53, 1> neg_b;
    BitField64<bool, 54, 1> abs_a;
    BitField64<bool, 55, 1> ftz;
    BitField64<bool, 56, 1> neg_a;
    BitField64<bool, 57, 1> abs_b;
};

void EmitFadd32I(DecoderContext& context, InstFadd32I inst);

enum class MultiplyScale : u64 {
    None = 0,
    D2 = 1,
    D4 = 2,
    D8 = 3,
    M8 = 4,
    M4 = 5,
    M2 = 6,
};

union InstFmulBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<RoundMode, 39, 2> round_mode;
    BitField64<MultiplyScale, 41, 3> scale;
    BitField64<u32, 44, 2> fmz; // TODO
    BitField64<bool, 47, 1> write_cc;
    BitField64<bool, 48, 1> neg_b;
    BitField64<bool, 50, 1> sat;
};

union InstFmulR {
    InstFmulBase base;
    BitField64<reg_t, 20, 8> src_b;
};

void EmitFmulR(DecoderContext& context, InstFmulR inst);

union InstFmulC {
    InstFmulBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
};

void EmitFmulC(DecoderContext& context, InstFmulC inst);

union InstFmulI {
    InstFmulBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
};

void EmitFmulI(DecoderContext& context, InstFmulI inst);

union InstFmul32I {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 32> imm;
    BitField64<bool, 52, 1> write_cc;
    BitField64<u32, 53, 2> fmz; // TODO
    BitField64<bool, 55, 1> sat;
};

void EmitFmul32I(DecoderContext& context, InstFmul32I inst);

union InstFfmaBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<bool, 47, 1> write_cc;
    BitField64<bool, 48, 1> neg_b;
    BitField64<bool, 49, 1> neg_c;
    BitField64<bool, 50, 1> sat;
    BitField64<RoundMode, 51, 2> round_mode;
    BitField64<u32, 53, 2> fmz; // TODO
};

union InstFfmaR {
    InstFfmaBase base;
    BitField64<reg_t, 20, 8> src_b;
    BitField64<reg_t, 39, 8> src_c;
};

void EmitFfmaR(DecoderContext& context, InstFfmaR inst);

union InstFfmaRC {
    InstFfmaBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
    BitField64<reg_t, 39, 8> src_b;
};

void EmitFfmaRC(DecoderContext& context, InstFfmaRC inst);

union InstFfmaC {
    InstFfmaBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
    BitField64<reg_t, 39, 8> src_c;
};

void EmitFfmaC(DecoderContext& context, InstFfmaC inst);

union InstFfmaI {
    InstFfmaBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
    BitField64<reg_t, 39, 8> src_c;
};

void EmitFfmaI(DecoderContext& context, InstFfmaI inst);

union InstFfma32I {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 32> imm;
    BitField64<bool, 52, 1> write_cc;
    BitField64<u32, 53, 2> fmz; // TODO
    BitField64<bool, 55, 1> sat;
    BitField64<bool, 56, 1> neg_a;
    BitField64<bool, 57, 1> neg_c;
};

void EmitFfma32I(DecoderContext& context, InstFfma32I inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
