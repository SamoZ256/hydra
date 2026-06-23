#pragma once

#include "core/horizon/services/nvdrv/ioctl/const.hpp"

namespace hydra::horizon::kernel {
class Event;
}

namespace hydra::horizon::services::nvdrv::ioctl {

class FdBase {
  public:
    virtual ~FdBase() noexcept = default;

    virtual NvResult Ioctl(IoctlContext& context, u32 type, u32 nr) = 0;
    virtual NvResult Ioctl2([[maybe_unused]] IoctlContext& context, u32 type,
                            u32 nr) {
        LOG_WARN(Services,
                 "Ioctl2 not supported by this FD (type: {:x}, NR: {:x})", type,
                 nr);
        return NvResult::NotSupported;
    }
    virtual NvResult Ioctl3([[maybe_unused]] IoctlContext& context, u32 type,
                            u32 nr) {
        LOG_WARN(Services,
                 "Ioctl2 not supported by this FD (type: {:x}, NR: {:x})", type,
                 nr);
        return NvResult::NotSupported;
    }
    virtual NvResult QueryEvent(u32 event_id_u32, kernel::Event*& out_event) {
        (void)out_event;
        LOG_WARN(Services, "Unknown event id {}", event_id_u32);
        return NvResult::NotSupported;
    }
};

} // namespace hydra::horizon::services::nvdrv::ioctl
