#include "core/horizon/services/nvdrv/ioctl/channel_base.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

DEFINE_IOCTL_TABLE(
    ChannelBase,
    DEFINE_IOCTL_TABLE_ENTRY(ChannelBase, 0x47, 0x14, SetUserData, 0x15,
                             GetUserData)
        DEFINE_IOCTL_TABLE_ENTRY(ChannelBase, 0x48, 0x01, SetNvMapFd, 0x03,
                                 SetTimeout, 0x08, SubmitGpfifo, 0x09,
                                 AllocObjCtx, 0x0b, ZCullBind, 0x0c,
                                 SetErrorNotifier, 0x0d, SetPriority, 0x17,
                                 GetErrorNotification, 0x18, AllocGpfifoEX,
                                 0x1a, AllocGpfifoEX, 0x1b, SubmitGpfifo))

NvResult ChannelBase::SetUserData(u64 data) {
    user_data = data;
    return NvResult::Success;
}

NvResult ChannelBase::GetUserData(u64* out_data) {
    *out_data = user_data;
    return NvResult::Success;
}

NvResult ChannelBase::SetNvMapFd(u32 fd_id) {
    LOG_DEBUG(Services, "FD: {}", fd_id);

    LOG_FUNC_STUBBED(Services);

    return NvResult::Success;
}

NvResult ChannelBase::SetTimeout(u32 timeout) {
    LOG_DEBUG(Services, "Timeout: {}", timeout);

    LOG_FUNC_STUBBED(Services);

    return NvResult::Success;
}

NvResult ChannelBase::SetPriority(u32 priority) {
    LOG_DEBUG(Services, "Priority: {}", priority);

    LOG_FUNC_STUBBED(Services);
    return NvResult::Success;
}

} // namespace hydra::horizon::services::nvdrv::ioctl
