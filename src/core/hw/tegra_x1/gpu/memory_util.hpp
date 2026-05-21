#pragma once

#include "core/hw/tegra_x1/gpu/const.hpp"

// TODO: support depth

namespace hydra::hw::tegra_x1::gpu {

constexpr u32 GOB_WIDTH_LOG2 = 6;
constexpr u32 GOB_HEIGHT_LOG2 = 3;
constexpr u32 GOB_WIDTH = 1 << GOB_WIDTH_LOG2;
constexpr u32 GOB_HEIGHT = 1 << GOB_HEIGHT_LOG2;

constexpr usize GOB_SIZE = 512;

typedef std::function<void(u32 gob_x, u32 gob_y, u32 gob_z, u8* out_gob)>
    ReadGobFn;
typedef std::function<void(const u8* in_gob, u32 gob_x, u32 gob_y, u32 gob_z)>
    WriteGobFn;

void ConvertBlockLinearToLinear(u32 stride, u32 rows, u32 depth,
                                u32 block_height_gobs_log2,
                                u32 block_depth_gobs_log2, const u8* in_data,
                                WriteGobFn write_fn);
void ConvertBlockLinearToLinear(u32 src_stride, u32 dst_stride,
                                u32 dst_slice_stride, u32 rows, u32 depth,
                                u32 block_height_gobs_log2,
                                u32 block_depth_gobs_log2, const u8* in_data,
                                u8* out_data);

void ConvertLinearToBlockLinear(u32 stride, u32 rows, u32 depth,
                                u32 block_height_gobs_log2,
                                u32 block_depth_gobs_log2, ReadGobFn read_fn,
                                u8* out_data);
void ConvertLinearToBlockLinear(u32 src_stride, u32 src_slice_stride,
                                u32 dst_stride, u32 rows, u32 depth,
                                u32 block_height_gobs_log2,
                                u32 block_depth_gobs_log2, const u8* in_data,
                                u8* out_data);

} // namespace hydra::hw::tegra_x1::gpu
