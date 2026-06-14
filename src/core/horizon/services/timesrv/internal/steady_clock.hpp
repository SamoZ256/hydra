#pragma once

namespace hydra::hw {
class WallClock;
}

namespace hydra::horizon::services::timesrv::internal {

class SteadyClock {
  public:
    SteadyClock(hw::WallClock& wall_clock_);

    u64 UpdateOffset();
    u64 GetTimePoint() const;

  private:
    hw::WallClock& wall_clock;
    u64 offset_ns;

  public:
    GETTER(offset_ns, GetOffsetNs);
};

} // namespace hydra::horizon::services::timesrv::internal
