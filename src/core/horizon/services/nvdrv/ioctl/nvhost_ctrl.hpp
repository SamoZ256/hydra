#pragma once

#include "core/horizon/services/nvdrv/ioctl/fd_base.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

class NvHostCtrl : public FdBase {
  public:
    NvResult Ioctl(IoctlContext& context, u32 type, u32 nr) override;

  private:
    // Ioctls
    NvResult GetConfig(std::array<char, 0x41> name, std::array<char, 0x41> key,
                       std::array<u8, 0x101>* out_value);
    NvResult SyncptWaitEvent(u32 id, u32 tresh, i32* out_timeout,
                             u32* out_value);
};

} // namespace hydra::horizon::services::nvdrv::ioctl
