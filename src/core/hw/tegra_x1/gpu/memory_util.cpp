#include "core/hw/tegra_x1/gpu/memory_util.hpp"

namespace hydra::hw::tegra_x1::gpu {

namespace {

template <bool encode>
void process_generic_16bx2(u32 stride, u32 rows, u32 block_height_log2,
                           u8* encoded, u8* decoded) {
    const auto block_height_gobs = 1u << block_height_log2;
    const auto block_height_px = 8u << block_height_log2;

    const auto horizontal_blocks = stride >> 6;
    const auto vertical_blocks =
        (rows + block_height_px - 1) >> (3 + block_height_log2);

    // Clear the output buffer first
    // TODO: is this necessary?
    // memset(out_data, 0, stride * height);

    constexpr usize BLOCK_SIZE = 32;

    for (u32 block_y = 0; block_y < vertical_blocks; block_y++) {
        for (u32 block_x = 0; block_x < horizontal_blocks; block_x++) {
            for (u32 gob_y = 0; gob_y < block_height_gobs; gob_y++) {
                const u32 x = block_x * 64;
                const u32 y = block_y * block_height_px + gob_y * 8;
                if (y < rows) {
                    u8* decoded_gob = (u8*)decoded + y * stride + x;
                    // Reverse the 16Bx2 swizzling for each GOB
                    for (u32 i = 0; i < BLOCK_SIZE; i++) {
                        const u32 local_y = ((i >> 1) & 0x06) | (i & 0x01);
                        const u32 local_x =
                            ((i << 3) & 0x10) | ((i << 1) & 0x20);

                        auto decoded_data = reinterpret_cast<u128*>(
                            decoded_gob + local_y * stride + local_x);
                        auto encoded_data = reinterpret_cast<u128*>(encoded);
                        if constexpr (encode)
                            *encoded_data = *decoded_data;
                        else
                            *decoded_data = *encoded_data;

                        encoded += sizeof(u128);
                    }
                } else {
                    // Skip this GOB if we're past the valid height
                    encoded += sizeof(u128) * BLOCK_SIZE;
                }
            }
        }
    }
}

} // namespace

void encode_generic_16bx2(u32 stride, u32 rows, u32 block_height_log2,
                          u8* in_data, u8* out_data) {
    process_generic_16bx2<true>(stride, rows, block_height_log2, out_data,
                                in_data);
}

void decode_generic_16bx2(u32 stride, u32 rows, u32 block_height_log2,
                          u8* in_data, u8* out_data) {
    process_generic_16bx2<false>(stride, rows, block_height_log2, in_data,
                                 out_data);
}

} // namespace hydra::hw::tegra_x1::gpu
