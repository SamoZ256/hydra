#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

union InstMovBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 39, 4> mask;
};

DEFINE_INST_SRC1_VARIANTS(Mov);

union InstMov32I {
    BitField64<reg_t, 0, 8> dst;
    BitField64<u32, 12, 4> mask;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 32> imm;
};

void emitMov32I(DecoderContext& context, InstMov32I inst);

union InstSelBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 39, 4> mask;
    BitField64<pred_t, 39, 3> src_pred;
    BitField64<bool, 42, 1> src_pred_inv;
};

DEFINE_INST_SRC2_VARIANTS(Sel);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
