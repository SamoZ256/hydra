#pragma once

namespace hydra::hw {

constexpr u32 GUEST_CNTFRQ = 19'200'000;
constexpr u64 GPU_TICK_FREQ = 614'400'000;

class WallClock {
  public:
    WallClock();

    u64 getTimeNs() const;
    u64 getCntpct() const;
    u64 getGpuTick() const;

  private:
    u128 ns_factor;
    u128 guest_factor;
    u128 gpu_tick_factor;
};

} // namespace hydra::hw
