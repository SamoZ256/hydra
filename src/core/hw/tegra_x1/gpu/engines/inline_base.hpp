#pragma once

#include "core/hw/tegra_x1/gpu/engines/engine_base.hpp"

#define INLINE_ENGINE_TABLE                                                    \
    0x6c, 1, launchDma, u32, 0x6d, 1, loadInlineData, u32
#define DEFINE_INLINE_ENGINE_METHODS                                           \
    void launchDma(const u32 index, const u32 data) {                          \
        launchDmaImpl(gpu, regs.regs_inline, index, data);                     \
    }                                                                          \
    void loadInlineData(const u32 index, const u32 data) {                     \
        loadInlineDataImpl(gpu, regs.regs_inline, index, data);                \
    }

namespace hydra::hw::tegra_x1::gpu {
class Gpu;
}

namespace hydra::hw::tegra_x1::gpu::engines {

struct RegsInline {
    u32 padding_0x0[0x60];

    // 0x60
    u32 line_length_in;
    u32 line_count;
    Iova offset_out;
    u32 pitch_out;
    u32 dst_block_size;
    u32 dst_width;
    u32 dst_height;
    u32 dst_depth;
    u32 dst_layer;
    u32 dst_origin_bytes_x;
    u32 dst_origin_samples_y;
    u32 not_a_reg_0xc;
    u32 not_a_reg_0xd;
};

class InlineBase {
  protected:
    // Commands
    void launchDmaImpl(Gpu& gpu, RegsInline& regs, const u32 index,
                       const u32 data);
    void loadInlineDataImpl(Gpu& gpu, RegsInline& regs, const u32 index,
                            const u32 data);

  private:
    std::vector<u32> inline_data;
};

} // namespace hydra::hw::tegra_x1::gpu::engines
