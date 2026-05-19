#include "core/hw/tegra_x1/gpu/memory_util.hpp"

namespace hydra::hw::tegra_x1::gpu {

namespace {

struct MemoryLayout {
    u32 block_height_gobs;
    u32 block_height;

    u32 horizontal_gobs;
    u32 horizontal_blocks;
    u32 vertical_gobs;
    u32 vertical_blocks;
};

MemoryLayout GetMemoryLayout(u32 stride, u32 rows, u32 block_height_log2) {
    MemoryLayout layout;
    layout.block_height_gobs = 1 << block_height_log2;
    layout.block_height = layout.block_height_gobs * GOB_HEIGHT;

    layout.horizontal_gobs = align(stride, GOB_WIDTH) >> GOB_WIDTH_LOG2;
    layout.horizontal_blocks =
        layout.horizontal_gobs; // GOBs = blocks (horizontally)
    layout.vertical_gobs = align(rows, layout.block_height) >> GOB_HEIGHT_LOG2;
    layout.vertical_blocks = layout.vertical_gobs >> block_height_log2;

    return layout;
}

uint2 UnswizzleGobCoords(u32 index) {
    const u32 local_y = ((index >> 1) & 0x06) | (index & 0x01);
    const u32 local_x = ((index << 3) & 0x10) | ((index << 1) & 0x20);
    return uint2({local_x, local_y});
}

} // namespace

void ConvertBlockLinearToLinear(u32 stride, u32 rows, u32 block_height_log2,
                                const u8* in_data, WriteGobFn write_fn) {
    const auto layout = GetMemoryLayout(stride, rows, block_height_log2);
    u32 gob_index = 0;
    for (u32 block_y = 0; block_y < layout.vertical_blocks; block_y++) {
        for (u32 block_x = 0; block_x < layout.horizontal_blocks; block_x++) {
            for (u32 gob_y = 0; gob_y < layout.block_height_gobs; gob_y++) {
                const u32 y =
                    block_y * layout.block_height + gob_y * GOB_HEIGHT;
                if (y >= rows) {
                    // Skip this GOB if we're past the valid height
                    gob_index++;
                    continue;
                }

                u8 out_gob[GOB_SIZE];
                for (u32 i = 0; i < GOB_SIZE / sizeof(u128); i++) {
                    const auto local = UnswizzleGobCoords(i);
                    *reinterpret_cast<u128*>(out_gob + local.y() * GOB_WIDTH +
                                             local.x()) =
                        reinterpret_cast<const u128*>(in_data +
                                                      gob_index * GOB_SIZE)[i];
                }

                write_fn(out_gob, block_x,
                         (block_y << block_height_log2) + gob_y, 0,
                         layout.horizontal_gobs, layout.vertical_gobs);

                gob_index++;
            }
        }
    }
}

void ConvertBlockLinearToLinear(u32 stride, u32 rows, u32 block_height_log2,
                                const u8* in_data, u8* out_data) {
    ConvertBlockLinearToLinear(
        stride, rows, block_height_log2, in_data,
        [=](const u8* in_gob, u32 gob_x, u32 gob_y, u32 gob_z,
            u32 horizontal_gobs, u32 vertical_gobs) {
            (void)gob_z;
            (void)vertical_gobs;
            for (u32 local_y = 0; local_y < GOB_HEIGHT; local_y++) {
                const u32 row = gob_y * GOB_HEIGHT + local_y;
                std::memcpy(out_data +
                                (row * horizontal_gobs + gob_x) * GOB_WIDTH,
                            in_gob + local_y * GOB_WIDTH, GOB_WIDTH);
            }
        });
}

void ConvertLinearToBlockLinear(u32 stride, u32 rows, u32 block_height_log2,
                                ReadGobFn read_fn, u8* out_data) {
    const auto layout = GetMemoryLayout(stride, rows, block_height_log2);
    for (u32 block_y = 0; block_y < layout.vertical_blocks; block_y++) {
        for (u32 block_x = 0; block_x < layout.horizontal_blocks; block_x++) {
            for (u32 gob_y = 0; gob_y < layout.block_height_gobs; gob_y++) {
                const u32 x = block_x * GOB_WIDTH;
                const u32 y =
                    block_y * layout.block_height + gob_y * GOB_HEIGHT;
                if (y >= rows) {
                    // Skip this GOB if we're past the valid height
                    continue;
                }

                u8 gob[GOB_SIZE];
                read_fn(block_x, (block_y << block_height_log2) + gob_y, 0,
                        layout.horizontal_gobs, layout.vertical_gobs, gob);

                u8* out_gob = out_data + y * stride + x;
                for (u32 i = 0; i < GOB_SIZE / sizeof(u128); i++) {
                    const auto local = UnswizzleGobCoords(i);
                    *reinterpret_cast<u128*>(out_gob + local.y() * stride +
                                             local.x()) =
                        reinterpret_cast<const u128*>(gob)[i];
                }
            }
        }
    }
}

void ConvertLinearToBlockLinear(u32 stride, u32 rows, u32 block_height_log2,
                                const u8* in_data, u8* out_data) {
    ConvertLinearToBlockLinear(
        stride, rows, block_height_log2,
        [=](u32 gob_x, u32 gob_y, u32 gob_z, u32 horizontal_gobs,
            u32 vertical_gobs, u8* out_gob) {
            (void)gob_z;
            (void)vertical_gobs;
            for (u32 local_y = 0; local_y < GOB_HEIGHT; local_y++) {
                const u32 row = gob_y * GOB_HEIGHT + local_y;
                std::memcpy(out_gob + local_y * GOB_WIDTH,
                            in_data +
                                (row * horizontal_gobs + gob_x) * GOB_WIDTH,
                            GOB_WIDTH);
            }
        },
        out_data);
}

} // namespace hydra::hw::tegra_x1::gpu
