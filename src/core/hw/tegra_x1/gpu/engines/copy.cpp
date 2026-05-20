#include "core/hw/tegra_x1/gpu/engines/copy.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/memory_util.hpp"
#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"

namespace hydra::hw::tegra_x1::gpu::engines {

DEFINE_METHOD_TABLE(Copy, 0xc0, 1, LaunchDMA, LaunchDMAData)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

void Copy::LaunchDMA(const u32 index, const LaunchDMAData data) {
    // TODO: implement component remapping
    // HACK
    u32 src_stride = regs.src.stride;
    u32 dst_stride = regs.dst.stride;
    if (data.remap_enable) {
        const auto& c = regs.remap_components;
        ASSERT_DEBUG(
            c.dst_x == 0 && c.dst_y == 1 && c.dst_z == 2 && c.dst_w == 3,
            Engines, "Component remapping not implemented ({}, {}, {}, {})",
            c.dst_x, c.dst_y, c.dst_z, c.dst_w);

        const u32 component_size = c.component_size_minus_one + 1;
        {
            const u32 component_count = c.src_component_count_minus_one + 1;
            u32 bytes_per_block = component_count * component_size;
            src_stride *= bytes_per_block;
        }
        {
            const u32 component_count = c.dst_component_count_minus_one + 1;
            u32 bytes_per_block = component_count * component_size;
            dst_stride *= bytes_per_block;
        }
    }

    const auto src_ptr = tls_crnt_gmmu->UnmapAddr(regs.offset_in);
    const auto dst_ptr = tls_crnt_gmmu->UnmapAddr(regs.offset_out);
    if (data.src_memory_layout == MemoryLayout::Pitch) {
        if (data.dst_memory_layout == MemoryLayout::Pitch) {
            for (u32 i = 0; i < regs.line_count; i++)
                memcpy(reinterpret_cast<void*>(dst_ptr + regs.stride_out * i),
                       reinterpret_cast<void*>(src_ptr + regs.stride_in * i),
                       regs.stride_in);

            // Invalidate
            gpu.GetRenderer().GetBufferCache().InvalidateMemory(
                Range<uptr>::FromSize(dst_ptr,
                                      regs.line_count * regs.stride_out));
        } else {
            // TODO: origin, line size, src stride
            // TODO: block size log2 can also be negative?
            ConvertLinearToBlockLinear(
                dst_stride, regs.dst.height, regs.dst.depth,
                static_cast<u32>(
                    get_block_size_log2(regs.dst.block_size.height)),
                static_cast<u32>(
                    get_block_size_log2(regs.dst.block_size.depth)),
                reinterpret_cast<u8*>(src_ptr), reinterpret_cast<u8*>(dst_ptr));

            // TODO: invalidate memory
        }
    } else {
        if (data.dst_memory_layout == MemoryLayout::Pitch) {
            // TODO: origin, line size, dst stride
            // TODO: block size log2 can also be negative?
            ConvertBlockLinearToLinear(
                src_stride, regs.src.height, regs.src.depth,
                static_cast<u32>(
                    get_block_size_log2(regs.src.block_size.height)),
                static_cast<u32>(
                    get_block_size_log2(regs.src.block_size.depth)),
                reinterpret_cast<u8*>(src_ptr), reinterpret_cast<u8*>(dst_ptr));

            // TODO: invalidate memory
        } else {
            LOG_NOT_IMPLEMENTED(Engines, "BlockLinear to BlockLinear");
        }
    }

    // Invalidate
    gpu.GetRenderer().GetTextureCache().InvalidateMemory(
        Range<uptr>(dst_ptr, regs.stride_in * regs.line_count));
}

#pragma GCC diagnostic pop

} // namespace hydra::hw::tegra_x1::gpu::engines
