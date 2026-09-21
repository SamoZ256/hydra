#include "core/horizon/services/nvdrv/ioctl/channel_base.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

DEFINE_IOCTL_TABLE(
    ChannelBase,
    DEFINE_IOCTL_TABLE_ENTRY(ChannelBase, 0x00, 0x01, submit, 0x02,
                             getSyncpoint, 0x03, getWaitBase, 0x07,
                             setSubmitTimeout, 0x09, mapCmdBuffer, 0x0a,
                             unmapCmdBuffer)
        DEFINE_IOCTL_TABLE_ENTRY(ChannelBase, 0x47, 0x14, setUserData, 0x15,
                                 getUserData)
            DEFINE_IOCTL_TABLE_ENTRY(ChannelBase, 0x48, 0x01, setNvMapFd, 0x03,
                                     setTimeout, 0x08, submitGpfifo, 0x09,
                                     allocObjCtx, 0x0b, zCullBind, 0x0c,
                                     setErrorNotifier, 0x0d, setPriority, 0x17,
                                     getErrorNotification, 0x18, allocGpfifoEx,
                                     0x1a, allocGpfifoEx, 0x1b, submitGpfifo))

NvResult ChannelBase::submit() {
    LOG_FUNC_STUBBED(Services);
    return NvResult::Success;
}

NvResult ChannelBase::getSyncpoint(u32 module_id, u32* out_value) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "module ID: 0x{:x}", module_id);

    // HACK
    *out_value = 0;
    return NvResult::Success;
}

NvResult ChannelBase::getWaitBase(u32 module_id, u32* out_value) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "module ID: 0x{:x}", module_id);

    // HACK
    *out_value = 0;
    return NvResult::Success;
}

NvResult ChannelBase::setSubmitTimeout(u32 timeout) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "timeout: {}", timeout);
    return NvResult::Success;
}

NvResult ChannelBase::mapCmdBuffer(u32 num_handles,
                                   [[maybe_unused]] u32 _reserved_x4,
                                   Aligned<bool, 4> is_compressed,
                                   const MapCmdBufferHandle* handles) {
    std::span<const MapCmdBufferHandle> handle_span(handles, num_handles);
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "is compressed: {}, handles: [{}]",
                               is_compressed, fmt::join(handle_span, ", "));
    return NvResult::Success;
}

NvResult ChannelBase::unmapCmdBuffer(u32 num_handles,
                                     [[maybe_unused]] u32 _reserved_x4,
                                     Aligned<bool, 4> is_compressed,
                                     const UnmapCmdBufferHandle* handles) {
    std::span<const UnmapCmdBufferHandle> handle_span(handles, num_handles);
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "is compressed: {}, handles: [{}]",
                               is_compressed, fmt::join(handle_span, ", "));
    return NvResult::Success;
}

NvResult ChannelBase::setUserData(u64 data) {
    user_data = data;
    return NvResult::Success;
}

NvResult ChannelBase::getUserData(u64* out_data) const {
    *out_data = user_data;
    return NvResult::Success;
}

NvResult ChannelBase::setNvMapFd(u32 fd_id) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "FD: {}", fd_id);
    return NvResult::Success;
}

NvResult ChannelBase::setTimeout(u32 timeout) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "timeout: {}", timeout);
    return NvResult::Success;
}

NvResult ChannelBase::setPriority(u32 priority) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "priority: {}", priority);
    return NvResult::Success;
}

} // namespace hydra::horizon::services::nvdrv::ioctl
