#pragma once

#include "common/common.hpp"

namespace Hydra::HW::TegraX1::CPU {

class ThreadBase {
  public:
    virtual ~ThreadBase() {}

    virtual void Configure(uptr kernel_mem_base,
                           uptr tls_mem_base /*,
      uptr rom_mem_base*/, uptr stack_mem_end) = 0;

    virtual void Run() = 0;

    virtual u64 GetRegX(u8 reg) const = 0;
    virtual void SetRegX(u8 reg, u64 value) = 0;
    virtual void SetRegPC(u64 value) = 0;
};

} // namespace Hydra::HW::TegraX1::CPU
