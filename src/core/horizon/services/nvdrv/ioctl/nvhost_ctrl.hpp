#pragma once

#include "core/horizon/services/nvdrv/ioctl/fd_base.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

struct NvHostEvent {
    kernel::Event* event{nullptr};
    // TODO: more
};

class NvHostCtrl : public FdBase {
  public:
    NvResult ioctl([[maybe_unused]] IoctlContext& context, u32 type,
                   u32 nr) override;
    NvResult queryEvent(u32 event_id_u32, kernel::Event*& out_event) override;

  private:
    constexpr static usize EVENT_COUNT = 64;

    NvHostEvent events[EVENT_COUNT] = {{}};

    // Ioctls
    NvResult getConfig(std::array<char, 0x41> name, std::array<char, 0x41> key,
                       std::array<u8, 0x101>* out_value);
    NvResult syncptWaitEvent(u32 id, u32 tresh, i32 timeout, u32* out_value);
    NvResult syncptWaitEventEx(u32 id, u32 tresh, i32 timeout,
                               InOutSingle<u32> out_value);
    NvResult syncptAllocEvent(u32 slot);
    NvResult syncptFreeEvent(u32 slot);
};

} // namespace hydra::horizon::services::nvdrv::ioctl
