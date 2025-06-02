#pragma once

#include "core/horizon/services/nvdrv/ioctl/const.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

class FdBase {
  public:
    virtual ~FdBase() {}

    virtual NvResult Ioctl(IoctlContext& context, u32 type, u32 nr) = 0;
    virtual NvResult QueryEvent(u32 event_id_u32, handle_id_t& out_handle_id) {
        LOG_WARN(Services, "Unknown event id {}", event_id_u32);
        return NvResult::NotSupported;
    }
};

} // namespace hydra::horizon::services::nvdrv::ioctl
