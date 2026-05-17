#pragma once

#include "core/horizon/services/nvdrv/ioctl/fd_base.hpp"
#include "core/hw/tegra_x1/gpu/const.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

struct MapCmdBufferHandle {
    handle_id_t in_map_handle_id;
    u32 out_phys_addr;
};

struct UnmapCmdBufferHandle {
    handle_id_t in_map_handle_id;
    u32 _reserved_x4;
};

class ChannelBase : public FdBase {
  public:
    NvResult Ioctl([[maybe_unused]] IoctlContext& context, u32 type,
                   u32 nr) override;

  protected:
    u64 user_data;

    // Ioctls
    // TODO: args
    NvResult Submit();
    NvResult GetSyncpoint(u32 module_id, u32* out_value);
    NvResult GetWaitBase(u32 module_id, u32* out_value);
    NvResult SetSubmitTimeout(u32 timeout);
    NvResult MapCmdBuffer(u32 num_handles, [[maybe_unused]] u32 _reserved_x4,
                          aligned<bool, 4> is_compressed,
                          const MapCmdBufferHandle* handles);
    NvResult UnmapCmdBuffer(u32 num_handles, [[maybe_unused]] u32 _reserved_x4,
                            aligned<bool, 4> is_compressed,
                            const UnmapCmdBufferHandle* handles);
    NvResult SetUserData(u64 data);
    NvResult GetUserData(u64* out_data);
    NvResult SetNvMapFd(u32 fd_id);
    NvResult SetTimeout(u32 timeout);
    virtual NvResult
    SubmitGpfifo([[maybe_unused]] System* system,
                 [[maybe_unused]] kernel::Process* process, u64 gpfifo,
                 u32 num_entries,
                 InOut<hw::tegra_x1::gpu::GpfifoFlags, u32>
                     inout_flags_and_detailed_error,
                 InOutSingle<hw::tegra_x1::gpu::Fence> inout_fence,
                 const hw::tegra_x1::gpu::GpfifoEntry* entries) {
        std::span<const hw::tegra_x1::gpu::GpfifoEntry> entries_span(
            entries, num_entries);
        LOG_FATAL(Services,
                  "SubmitGpfifo not available (gpfifo: {}, "
                  "flags: {}, fence: {}, entries: [{}])",
                  gpfifo, inout_flags_and_detailed_error, inout_fence,
                  fmt::join(entries_span, ", "));
    }
    virtual NvResult AllocObjCtx(u32 class_num, u32 flags, u64* out_obj_id) {
        (void)out_obj_id;
        LOG_FATAL(Services,
                  "AllocObjCtx not available (class_num: {}, flags: {:x})",
                  class_num, flags);
    }
    virtual NvResult ZCullBind(gpu_vaddr_t addr, u32 mode,
                               [[maybe_unused]] u32 reserved) {
        LOG_FATAL(Services, "ZCullBind not available (addr: {:x}, mode: {})",
                  addr, mode);
    }
    virtual NvResult SetErrorNotifier(u64 offset, u64 size, u32 mem,
                                      [[maybe_unused]] u32 reserved) {
        LOG_FATAL(Services,
                  "SetErrorNotifier not available (offset: {:x}, size: {:x}, "
                  "mem: {})",
                  offset, size, mem);
    }
    NvResult SetPriority(u32 priority);
    virtual NvResult GetErrorNotification(u64* out_timestamp, u32* out_info32,
                                          u16* out_info16, u64* out_status) {
        (void)out_timestamp;
        (void)out_info32;
        (void)out_info16;
        (void)out_status;
        LOG_FATAL(Services, "GetErrorNotification not available");
    }
    virtual NvResult
    AllocGpfifoEX(u32 num_entries, u32 num_jobs, u32 flags,
                  hw::tegra_x1::gpu::Fence* out_fence,
                  [[maybe_unused]] std::array<u32, 3> reserved) {
        (void)out_fence;
        LOG_FATAL(Services,
                  "AllocGpfifoEX not available (num_entries: {}, "
                  "num_jobs: {}, flags: {:x})",
                  num_entries, num_jobs, flags);
    }
};

} // namespace hydra::horizon::services::nvdrv::ioctl

ENABLE_STRUCT_FORMATTING(
    hydra::horizon::services::nvdrv::ioctl::MapCmdBufferHandle,
    in_map_handle_id, "", "in map handle")

ENABLE_STRUCT_FORMATTING(
    hydra::horizon::services::nvdrv::ioctl::UnmapCmdBufferHandle,
    in_map_handle_id, "", "in map handle")
