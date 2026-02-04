#include "core/horizon/services/nvdrv/ioctl/nvhost_gpu.hpp"

#include "core/horizon/kernel/process.hpp"
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

DEFINE_IOCTL2_TABLE(NvHostGpu, DEFINE_IOCTL2_TABLE_ENTRY(NvHostGpu, 0x48, 0x1b,
                                                         SubmitGpfifo))

NvResult NvHostGpu::QueryEvent(u32 event_id_u32, kernel::Event*& out_event) {
    auto event_id = static_cast<EventId>(event_id_u32);
    switch (event_id) {
    case EventId::SmExceptionBptIntReport:
        out_event = sm_exception_bpt_int_report_event;
        break;
    case EventId::SmExceptionBptPauseReport:
        out_event = sm_exception_bpt_pause_report_event;
        break;
    case EventId::ErrorNotifierEvent:
        out_event = error_notifier_event;
        break;
    default:
        return NvResult::NotSupported;
    }

    return NvResult::Success;
}

// TODO: gpfifo
NvResult NvHostGpu::SubmitGpfifo(
    kernel::Process* process, u64 gpfifo, u32 num_entries,
    InOut<hw::tegra_x1::gpu::GpfifoFlags, u32> inout_flags_and_detailed_error,
    InOutSingle<hw::tegra_x1::gpu::Fence> inout_fence,
    const hw::tegra_x1::gpu::GpfifoEntry* entries) {
    (void)gpfifo;

    GPU_INSTANCE.GetPfifo().SubmitEntries(
        *process->GetGMmu(),
        std::span<const hw::tegra_x1::gpu::GpfifoEntry>(entries, num_entries),
        inout_flags_and_detailed_error);

    // HACK
    inout_flags_and_detailed_error = 0;
    inout_fence = hw::tegra_x1::gpu::Fence{};
    return NvResult::Success;
}

NvResult NvHostGpu::AllocObjCtx(u32 class_num, u32 flags, u64* out_obj_id) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "class number: {}, flags: {:#x}",
                               class_num, flags);

    // HACK
    *out_obj_id = 0;
    return NvResult::Success;
}

NvResult NvHostGpu::ZCullBind(gpu_vaddr_t addr, u32 mode,
                              [[maybe_unused]] u32 reserved) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "address: {:#x}, mode: {}", addr,
                               mode);
    return NvResult::Success;
}

NvResult NvHostGpu::SetErrorNotifier(u64 offset, u64 size, u32 mem,
                                     [[maybe_unused]] u32 reserved) {
    LOG_FUNC_WITH_ARGS_STUBBED(
        Services, "offset: {:#x}, size: {:#x}, memory: {}", offset, size, mem);
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

NvResult
NvHostGpu::AllocGpfifoEX(u32 num_entries, u32 num_jobs, u32 flags,
                         hw::tegra_x1::gpu::Fence* out_fence,
                         [[maybe_unused]] std::array<u32, 3> reserved) {
    LOG_FUNC_WITH_ARGS_STUBBED(
        Services, "number of entries: {}, num_jobs: {}, flags: {:#x}",
        num_entries, num_jobs, flags);

    // HACK
    *out_fence = {};
    return NvResult::Success;
}

} // namespace hydra::horizon::services::nvdrv::ioctl
