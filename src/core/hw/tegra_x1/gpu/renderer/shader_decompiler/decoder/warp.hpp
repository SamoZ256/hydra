#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

enum class ShuffleMode {
    Idx = 0,
    Up = 1,
    Down = 2,
    Bfly = 3,
};

union InstShfl {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 5> src_b_imm5;
    BitField64<reg_t, 20, 8> src_b_r;
    BitField64<bool, 28, 1> b_is_imm;
    BitField64<bool, 29, 1> c_is_imm;
    BitField64<ShuffleMode, 30, 2> mode;
    BitField64<u32, 34, 13> src_c_imm13;
    BitField64<reg_t, 39, 8> src_c_r;
    BitField64<pred_t, 48, 3> dst_pred;
};

void emitShfl(DecoderContext& context, InstShfl inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder::ShuffleMode,
    Idx, "indexed", Up, "up", Down, "down", Bfly, "butterfly")
