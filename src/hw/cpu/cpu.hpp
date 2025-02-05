#pragma once

#include "common.hpp"

namespace Hydra::HW::CPU {

class CPUBase {
  public:
    virtual u64 GetRegX(u8 reg) const = 0;
    virtual void SetRegX(u8 reg, u64 value) = 0;
};

} // namespace Hydra::HW::CPU
