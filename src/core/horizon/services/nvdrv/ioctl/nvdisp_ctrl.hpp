#pragma once

#include "core/horizon/services/nvdrv/ioctl/fd_base.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

class NvDispCtrl : public FdBase {
  public:
    NvResult Ioctl([[maybe_unused]] IoctlContext& context, u32 type,
                   u32 nr) override;

  private:
    // Ioctls
    NvResult NumOutputs(u32* out_num_outputs);
    NvResult
    GetDisplayProperties(InOutSingle<std::array<u8, 20>> inout_unknown_x0);
};

} // namespace hydra::horizon::services::nvdrv::ioctl
