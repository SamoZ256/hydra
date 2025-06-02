#pragma once

#include "core/horizon/services/nvdrv/ioctl/fd_base.hpp"
#include "core/hw/tegra_x1/gpu/const.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

enum class AllocSpaceFlags : u32 {
    None = 0,
    FixedOffset = BIT(0),
    Sparse = BIT(1),
};

ENABLE_ENUM_BITMASK_OPERATORS(AllocSpaceFlags)

enum class MapBufferFlags : u32 {
    None = 0,
    FixedOffset = BIT(0),
    IsCacheable = BIT(2),
    Modify = BIT(8),
};

ENABLE_ENUM_BITMASK_OPERATORS(MapBufferFlags)

struct VaRegion {
    gpu_vaddr_t addr;
    u32 page_size;
    u32 reserved;
    u64 pages;
};

struct RemapOp {
    u16 flags;
    u16 kind;
    u32 mem_handle;
    u32 mem_offset_in_pages;
    u32 virt_offset_in_pages;
    u32 num_pages;
};

class NvHostAsGpu : public FdBase {
  public:
    NvResult Ioctl(IoctlContext& context, u32 type, u32 nr) override;

  private:
    // Ioctls
    NvResult BindChannel(u32 fd_id);
    NvResult AllocSpace(u32 pages, u32 page_size,
                        aligned<AllocSpaceFlags, 8> flags,
                        InOut<u64, gpu_vaddr_t> align_and_offset);
    NvResult UnmapBuffer(gpu_vaddr_t addr);
    NvResult MapBufferEX(MapBufferFlags flags, hw::tegra_x1::gpu::NvKind kind,
                         handle_id_t nvmap_handle_id, u32 reserved,
                         u64 buffer_offset, u64 mapping_size,
                         InOutSingle<gpu_vaddr_t> inout_addr);
    NvResult GetVaRegions(gpu_vaddr_t buffer_addr,
                          InOutSingle<u32> inout_buffer_size, u32 reserved,
                          std::array<VaRegion, 2>* out_va_regions);
    NvResult AllocAsEX(u32 big_page_size, i32 as_fd, u32 flags, u32 reserved,
                       u64 va_range_start, u64 va_range_end,
                       u64 va_range_split);
    NvResult Remap(const RemapOp* entries);
};

} // namespace hydra::horizon::services::nvdrv::ioctl
