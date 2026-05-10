#pragma once

namespace hydra::hw {

constexpr u32 GUEST_CNTFRQ = 19'200'000;
constexpr u64 GPU_TICK_FREQ = 614'400'000;

class WallClock {
  public:
    static WallClock& GetInstance();

    WallClock();
    ~WallClock();

    u64 GetCntpct() const;
    u64 GetGpuTick() const;

  private:
    u128 guest_factor;
    u128 gpu_tick_factor;
};

} // namespace hydra::hw
