#pragma once

#include "core/horizon/services/nvdrv/ioctl/fd_base.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

class NvSchedCtrl : public FdBase {
  public:
    NvResult ioctl([[maybe_unused]] IoctlContext& context, u32 type,
                   u32 nr) override;

  private:
    // Ioctls
};

} // namespace hydra::horizon::services::nvdrv::ioctl
