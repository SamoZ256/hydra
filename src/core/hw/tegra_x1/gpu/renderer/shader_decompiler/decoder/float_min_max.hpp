#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

union InstFmnmxBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<pred_t, 39, 3> src_pred;
    BitField64<bool, 42, 1> src_pred_inv;
    BitField64<bool, 44, 1> ftz;
    BitField64<bool, 45, 1> neg_b;
    BitField64<bool, 46, 1> abs_a;
    BitField64<bool, 47, 1> write_cc;
    BitField64<bool, 48, 1> neg_a;
    BitField64<bool, 49, 1> abs_b;
};

union InstFmnmxR {
    InstFmnmxBase base;
    BitField64<reg_t, 20, 8> src_b;
};

void EmitFmnmxR(DecoderContext& context, InstFmnmxR inst);

union InstFmnmxC {
    InstFmnmxBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
};

void EmitFmnmxC(DecoderContext& context, InstFmnmxC inst);

union InstFmnmxI {
    InstFmnmxBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
};

void EmitFmnmxI(DecoderContext& context, InstFmnmxI inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
