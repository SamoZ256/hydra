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
    NvResult Create(System* system, u32 size, handle_id_t* out_handle_id);
    NvResult FromId(u32 id, handle_id_t* out_handle_id);
    NvResult Alloc(System* system, handle_id_t handle_id, u32 heap_mask,
                   u32 flags, InOutSingle<u32> inout_alignment,
                   aligned<u8, 8> kind, gpu_vaddr_t addr);
    NvResult Free(System* system, aligned<handle_id_t, 8> handle_id,
                  gpu_vaddr_t* out_addr, u64* out_size, u32* out_flags);
    NvResult Param(System* system, handle_id_t handle_id, NvMapParamType type,
                   u32* out_value);
    NvResult GetId(u32* out_id, handle_id_t handle_id);
};

} // namespace hydra::horizon::services::nvdrv::ioctl
