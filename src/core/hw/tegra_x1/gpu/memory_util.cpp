#include "core/hw/tegra_x1/gpu/memory_util.hpp"

namespace hydra::hw::tegra_x1::gpu {

namespace {

// block_height_log2 is in GOBs
template <bool to_block_linear>
void Convert(u32 stride, u32 rows, u32 block_height_log2, u8* block_linear,
             u8* linear) {
    constexpr u32 GOB_WIDTH_LOG2 = 6;
    constexpr u32 GOB_HEIGHT_LOG2 = 3;
    constexpr u32 GOB_WIDTH = 1 << GOB_WIDTH_LOG2;
    constexpr u32 GOB_HEIGHT = 1 << GOB_HEIGHT_LOG2;

    const u32 block_height_gobs = 1 << block_height_log2;
    const u32 block_height_px = GOB_HEIGHT << block_height_log2;

    const u32 horizontal_blocks =
        stride >> GOB_WIDTH_LOG2; // GOBs = blocks (horizontally)
    const u32 vertical_blocks =
        align(rows, block_height_px) >> (GOB_HEIGHT_LOG2 + block_height_log2);

    // Clear the output buffer first
    // TODO: is this necessary?
    // memset(out_data, 0, stride * height);

    constexpr usize GOB_SIZE = 512;

    for (u32 block_y = 0; block_y < vertical_blocks; block_y++) {
        for (u32 block_x = 0; block_x < horizontal_blocks; block_x++) {
            for (u32 gob_y = 0; gob_y < block_height_gobs; gob_y++) {
                const u32 x = block_x * GOB_WIDTH;
                const u32 y = block_y * block_height_px + gob_y * GOB_HEIGHT;
                if (y >= rows) {
                    // Skip this GOB if we're past the valid height
                    block_linear += GOB_SIZE;
                    continue;
                }

                u8* decoded_gob = linear + y * stride + x;

                // Reverse the 16Bx2 swizzling for each GOB
                for (u32 i = 0; i < GOB_SIZE / sizeof(u128); i++) {
                    const u32 local_y = ((i >> 1) & 0x06) | (i & 0x01);
                    const u32 local_x = ((i << 3) & 0x10) | ((i << 1) & 0x20);

                    auto linear_data = reinterpret_cast<u128*>(
                        decoded_gob + local_y * stride + local_x);
                    auto block_linear_data =
                        reinterpret_cast<u128*>(block_linear);
                    if constexpr (to_block_linear)
                        *block_linear_data = *linear_data;
                    else
                        *linear_data = *block_linear_data;

                    block_linear += sizeof(u128);
                }
            }
        }
    }
}

} // namespace

void ConvertBlockLinearToLinear(u32 stride, u32 rows, u32 block_height_log2,
                                const u8* in_data, u8* out_data) {
    Convert<false>(stride, rows, block_height_log2, const_cast<u8*>(in_data),
                   out_data);
}

void ConvertLinearToBlockLinear(u32 stride, u32 rows, u32 block_height_log2,
                                const u8* in_data, u8* out_data) {
    Convert<true>(stride, rows, block_height_log2, out_data,
                  const_cast<u8*>(in_data));
}

} // namespace hydra::hw::tegra_x1::gpu
