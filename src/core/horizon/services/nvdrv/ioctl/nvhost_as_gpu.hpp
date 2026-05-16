#pragma once

#include "core/horizon/services/nvdrv/ioctl/fd_base.hpp"
#include "core/hw/tegra_x1/gpu/const.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

enum class AllocSpaceFlags : u32 {
    None = 0,
    FixedOffset = BIT(0),
    Sparse = BIT(1),
};

ENABLE_ENUM_BITWISE_OPERATORS(AllocSpaceFlags)

enum class MapBufferFlags : u32 {
    None = 0,
    FixedOffset = BIT(0),
    IsCacheable = BIT(2),
    Modify = BIT(8),
};

ENABLE_ENUM_BITWISE_OPERATORS(MapBufferFlags)

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
    NvResult Ioctl([[maybe_unused]] IoctlContext& context, u32 type,
                   u32 nr) override;
    NvResult Ioctl3([[maybe_unused]] IoctlContext& context, u32 type,
                    u32 nr) override;

  private:
    // Ioctls
    NvResult BindChannel(u32 fd_id);
    NvResult AllocSpace(kernel::Process* process, u32 pages, u32 page_size,
                        aligned<AllocSpaceFlags, 8> flags,
                        InOut<u64, gpu_vaddr_t> align_and_offset);
    NvResult FreeSpace(vaddr_t offset, u32 pages, u32 page_size);
    NvResult UnmapBuffer(gpu_vaddr_t addr);
    NvResult MapBufferEX(System* system, kernel::Process* process,
                         MapBufferFlags flags, hw::tegra_x1::gpu::NvKind kind,
                         handle_id_t nvmap_handle_id,
                         [[maybe_unused]] u32 reserved, u64 buffer_offset,
                         u64 mapping_size, InOutSingle<gpu_vaddr_t> inout_addr);
    NvResult GetVaRegions(gpu_vaddr_t buffer_addr,
                          InOutSingle<u32> inout_buffer_size,
                          [[maybe_unused]] u32 reserved,
                          std::array<VaRegion, 2>* out_va_regions);
    NvResult AllocAsEX(kernel::Process* process, u32 big_page_size, i32 as_fd,
                       u32 flags, [[maybe_unused]] u32 reserved,
                       u64 va_range_start, u64 va_range_end,
                       u64 va_range_split);
    NvResult Remap(const RemapOp* entries);

    NvResult GetVaRegions3(gpu_vaddr_t buffer_addr,
                           InOutSingle<u32> inout_buffer_size,
                           [[maybe_unused]] u32 reserved) {
        // TODO: does this just throw the out_va_regions away?
        std::array<VaRegion, 2> out_va_regions;
        return GetVaRegions(buffer_addr, inout_buffer_size, reserved,
                            &out_va_regions);
    }
};

} // namespace hydra::horizon::services::nvdrv::ioctl

ENABLE_STRUCT_FORMATTING(hydra::horizon::services::nvdrv::ioctl::RemapOp, flags,
                         ":#x", "flags", kind, "", "kind", mem_handle, "",
                         "memory handle", mem_offset_in_pages, ":#x",
                         "memory offset (in pages)", virt_offset_in_pages,
                         ":#x", "virtual offset (in pages)", num_pages, ":#x",
                         "number of pages")
