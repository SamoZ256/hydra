#include "core/horizon/services/nvdrv/ioctl/channel_base.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

DEFINE_IOCTL_TABLE(
    ChannelBase,
    DEFINE_IOCTL_TABLE_ENTRY(ChannelBase, 0x00, 0x01, Submit, 0x02,
                             GetSyncpoint, 0x03, GetWaitBase, 0x09,
                             MapCmdBuffer, 0x0a, UnmapCmdBuffer)
        DEFINE_IOCTL_TABLE_ENTRY(ChannelBase, 0x47, 0x14, SetUserData, 0x15,
                                 GetUserData)
            DEFINE_IOCTL_TABLE_ENTRY(ChannelBase, 0x48, 0x01, SetNvMapFd, 0x03,
                                     SetTimeout, 0x08, SubmitGpfifo, 0x09,
                                     AllocObjCtx, 0x0b, ZCullBind, 0x0c,
                                     SetErrorNotifier, 0x0d, SetPriority, 0x17,
                                     GetErrorNotification, 0x18, AllocGpfifoEX,
                                     0x1a, AllocGpfifoEX, 0x1b, SubmitGpfifo))

NvResult ChannelBase::Submit() {
    LOG_FUNC_NOT_IMPLEMENTED(Services);
    return NvResult::Success;
}

NvResult ChannelBase::GetSyncpoint(u32 module_id, u32* out_value) {
    LOG_FUNC_STUBBED(Services);

    LOG_DEBUG(Services, "Module ID: 0x{:x}", module_id);

    // HACK
    *out_value = 0;
    return NvResult::Success;
}

NvResult ChannelBase::GetWaitBase(u32 module_id, u32* out_value) {
    LOG_FUNC_STUBBED(Services);

    LOG_DEBUG(Services, "Module ID: 0x{:x}", module_id);

    // HACK
    *out_value = 0;
    return NvResult::Success;
}

NvResult ChannelBase::MapCmdBuffer(u32 num_handles, u32 _reserved_x4,
                                   bool is_compressed,
                                   std::array<u8, 3> _padding_x9,
                                   const MapCmdBufferHandle* handles) {
    LOG_FUNC_STUBBED(Services);
    return NvResult::Success;
}

NvResult ChannelBase::UnmapCmdBuffer(u32 num_handles, u32 _reserved_x4,
                                     bool is_compressed,
                                     std::array<u8, 3> _padding_x9,
                                     const UnmapCmdBufferHandle* handles) {
    LOG_FUNC_STUBBED(Services);
    return NvResult::Success;
}

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
