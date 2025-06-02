#pragma once

#include "core/horizon/services/nvdrv/ioctl/fd_base.hpp"
#include "core/hw/tegra_x1/gpu/const.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

class ChannelBase : public FdBase {
  public:
    NvResult Ioctl(IoctlContext& context, u32 type, u32 nr) override;

  protected:
    u64 user_data;

    // Ioctls
    NvResult SetUserData(u64 data);
    NvResult GetUserData(u64* out_data);
    NvResult SetNvMapFd(u32 fd_id);
    NvResult SetTimeout(u32 timeout);
    virtual NvResult
    SubmitGpfifo(u64 gpfifo, u32 num_entries,
                 InOut<hw::tegra_x1::gpu::GpfifoFlags, u32>
                     inout_flags_and_detailed_error,
                 InOutSingle<hw::tegra_x1::gpu::Fence> inout_fence,
                 const hw::tegra_x1::gpu::GpfifoEntry* entries) = 0;
    virtual NvResult AllocObjCtx(u32 class_num, u32 flags, u64* out_obj_id) = 0;
    virtual NvResult ZCullBind(gpu_vaddr_t addr, u32 mode, u32 reserved) = 0;
    virtual NvResult SetErrorNotifier(u64 offset, u64 size, u32 mem,
                                      u32 reserved) = 0;
    NvResult SetPriority(u32 priority);
    virtual NvResult GetErrorNotification(u64* out_timestamp, u32* out_info32,
                                          u16* out_info16, u64* out_status) = 0;
    virtual NvResult AllocGpfifoEX(u32 num_entries, u32 num_jobs, u32 flags,
                                   hw::tegra_x1::gpu::Fence* out_fence,
                                   std::array<u32, 3> reserved) = 0;
};

} // namespace hydra::horizon::services::nvdrv::ioctl
