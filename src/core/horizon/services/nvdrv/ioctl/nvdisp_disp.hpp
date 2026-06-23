#pragma once

#include "core/horizon/services/nvdrv/ioctl/fd_base.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

class NvDispDisp : public FdBase {
  public:
    NvDispDisp(u32 display_index_) : display_index{display_index_} {}

    NvResult Ioctl([[maybe_unused]] IoctlContext& context, u32 type,
                   u32 nr) override;

  private:
    u32 display_index;

    // Ioctls
    NvResult GetWindow(u32 unknown_x0);
    NvResult SetCmu(InOutSingle<std::array<u8, 2458>> inout_unknown_x0);
};

} // namespace hydra::horizon::services::nvdrv::ioctl
