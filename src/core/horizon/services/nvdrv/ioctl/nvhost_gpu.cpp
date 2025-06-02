#include "core/horizon/services/nvdrv/ioctl/nvhost_gpu.hpp"

#include "core/hw/tegra_x1/gpu/const.hpp"
#include "core/hw/tegra_x1/gpu/gpu.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

namespace {

enum class EventId {
    SmExceptionBptIntReport = 1,
    SmExceptionBptPauseReport,
    ErrorNotifierEvent,
};

}

NvResult NvHostGpu::QueryEvent(u32 event_id_u32, handle_id_t& out_handle_id) {
    auto event_id = static_cast<EventId>(event_id_u32);
    switch (event_id) {
    case EventId::SmExceptionBptIntReport:
        out_handle_id = sm_exception_bpt_int_report_event.id;
        break;
    case EventId::SmExceptionBptPauseReport:
        out_handle_id = sm_exception_bpt_pause_report_event.id;
        break;
    case EventId::ErrorNotifierEvent:
        out_handle_id = error_notifier_event.id;
        break;
    default:
        out_handle_id = INVALID_HANDLE_ID;
        return NvResult::NotSupported;
    }

    return NvResult::Success;
}

NvResult NvHostGpu::SubmitGpfifo(
    u64 gpfifo, u32 num_entries,
    InOut<hw::tegra_x1::gpu::GpfifoFlags, u32> inout_flags_and_detailed_error,
    InOutSingle<hw::tegra_x1::gpu::Fence> inout_fence,
    const hw::tegra_x1::gpu::GpfifoEntry* entries) {
    GPU_INSTANCE.GetPfifo().SubmitEntries(
        std::vector<hw::tegra_x1::gpu::GpfifoEntry>(entries,
                                                    entries + num_entries),
        inout_flags_and_detailed_error);

    // HACK
    inout_flags_and_detailed_error = 0;
    inout_fence = hw::tegra_x1::gpu::Fence{};
    return NvResult::Success;
}

NvResult NvHostGpu::AllocObjCtx(u32 class_num, u32 flags, u64* out_obj_id) {
    LOG_FUNC_STUBBED(Services);
    return NvResult::Success;
}

NvResult NvHostGpu::ZCullBind(gpu_vaddr_t addr, u32 mode, u32 reserved) {
    LOG_FUNC_STUBBED(Services);
    return NvResult::Success;
}

NvResult NvHostGpu::SetErrorNotifier(u64 offset, u64 size, u32 mem,
                                     u32 reserved) {
    LOG_FUNC_STUBBED(Services);
    return NvResult::Success;
}

NvResult NvHostGpu::GetErrorNotification(u64* out_timestamp, u32* out_info32,
                                         u16* out_info16, u64* out_status) {
    LOG_FUNC_STUBBED(Services);

    *out_timestamp = 0;           // TODO
    *out_info32 = 0;              // TODO
    *out_info16 = 0;              // TODO
    *out_status = /*0xffff*/ 0x0; // TODO: correct?

    return NvResult::Success;
}

NvResult NvHostGpu::AllocGpfifoEX(u32 num_entries, u32 num_jobs, u32 flags,
                                  hw::tegra_x1::gpu::Fence* out_fence,
                                  std::array<u32, 3> reserved) {
    LOG_FUNC_STUBBED(Services);
    return NvResult::Success;
}

} // namespace hydra::horizon::services::nvdrv::ioctl
