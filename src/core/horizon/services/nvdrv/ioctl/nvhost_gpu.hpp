#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/nvdrv/ioctl/channel_base.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

class NvHostGpu : public ChannelBase {
  public:
    // TODO: autoclear events?
    NvHostGpu()
        : sm_exception_bpt_int_report_event(new kernel::Event()),
          sm_exception_bpt_pause_report_event(new kernel::Event()),
          error_notifier_event(new kernel::Event()) {}

    NvResult QueryEvent(u32 event_id_u32, handle_id_t& out_handle_id) override;

  private:
    kernel::HandleWithId<kernel::Event> sm_exception_bpt_int_report_event;
    kernel::HandleWithId<kernel::Event> sm_exception_bpt_pause_report_event;
    kernel::HandleWithId<kernel::Event> error_notifier_event;

    // Ioctls
    NvResult
    SubmitGpfifo(u64 gpfifo, u32 num_entries,
                 InOut<hw::tegra_x1::gpu::GpfifoFlags, u32>
                     inout_flags_and_detailed_error,
                 InOutSingle<hw::tegra_x1::gpu::Fence> inout_fence,
                 const hw::tegra_x1::gpu::GpfifoEntry* entries) override;
    NvResult AllocObjCtx(u32 class_num, u32 flags, u64* out_obj_id) override;
    NvResult ZCullBind(gpu_vaddr_t addr, u32 mode, u32 reserved) override;
    NvResult SetErrorNotifier(u64 offset, u64 size, u32 mem,
                              u32 reserved) override;
    NvResult GetErrorNotification(u64* out_timestamp, u32* out_info32,
                                  u16* out_info16, u64* out_status) override;
    NvResult AllocGpfifoEX(u32 num_entries, u32 num_jobs, u32 flags,
                           hw::tegra_x1::gpu::Fence* out_fence,
                           std::array<u32, 3> reserved) override;
};

} // namespace hydra::horizon::services::nvdrv::ioctl
