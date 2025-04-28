#pragma once

#include "core/hw/tegra_x1/gpu/const.hpp"

namespace Hydra::HW::TegraX1::GPU {

void encode_generic_16bx2(usize stride, usize height, usize block_height_log2,
                          u8* in_data, u8* out_data);

void decode_generic_16bx2(usize stride, usize height, usize block_height_log2,
                          u8* in_data, u8* out_data);

} // namespace Hydra::HW::TegraX1::GPU
