#pragma once

#include "core/horizon/services/nvdrv/ioctl/fd_base.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

enum class NvMapParamType : u32 {
    Size = 1,
    Alignment = 2,
    Base = 3, // Returns error
    Heap = 4, // Always 0x40000000
    Kind = 5,
    Compr = 6, // Unused
};

class NvMap : public FdBase {
  public:
    NvResult Ioctl([[maybe_unused]] IoctlContext& context, u32 type,
                   u32 nr) override;

  private:
    // Ioctls
    NvResult Create(System* system, u32 size, Handle* out_handle);
    NvResult FromId(u32 id, Handle* out_handle);
    NvResult Alloc(System* system, Handle handle, u32 heap_mask, u32 flags,
                   InOutSingle<u32> inout_alignment, Aligned<u8, 8> kind,
                   gpu_vaddr_t addr);
    NvResult Free(System* system, Aligned<Handle, 8> handle,
                  gpu_vaddr_t* out_addr, u64* out_size, u32* out_flags);
    NvResult Param(System* system, Handle handle, NvMapParamType type,
                   u32* out_value);
    NvResult GetId(u32* out_id, Handle handle);
};

} // namespace hydra::horizon::services::nvdrv::ioctl
