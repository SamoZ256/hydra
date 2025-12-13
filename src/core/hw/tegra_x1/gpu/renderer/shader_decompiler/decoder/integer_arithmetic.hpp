#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

union InstIaddBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<bool, 43, 1> x;
    BitField64<bool, 47, 1> write_cc;
    BitField64<bool, 48, 1> neg_b;
    BitField64<bool, 49, 1> neg_a;
    BitField64<bool, 50, 1> sat;
};

union InstIaddR {
    InstIaddBase base;
    BitField64<reg_t, 20, 8> src_b;
};

void EmitIaddR(DecoderContext& context, InstIaddR inst);

union InstIaddC {
    InstIaddBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
};

void EmitIaddC(DecoderContext& context, InstIaddC inst);

union InstIaddI {
    InstIaddBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
};

void EmitIaddI(DecoderContext& context, InstIaddI inst);

union InstIadd32I {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 32> imm;
    BitField64<bool, 52, 1> write_cc;
    BitField64<bool, 53, 1> x;
    BitField64<bool, 54, 1> sat;
    BitField64<bool, 55, 1> po_0;
    BitField64<bool, 56, 1> neg_a;
};

void EmitIadd32I(DecoderContext& context, InstIadd32I inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
