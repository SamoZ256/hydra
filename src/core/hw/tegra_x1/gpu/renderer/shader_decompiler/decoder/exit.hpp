#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

union InstKil {
    BitField64<u32, 0, 5> ccc; // TODO
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
};

void EmitKil(DecoderContext& context, InstKil inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
