#pragma once

namespace hydra::hw {
class WallClock;
}

namespace hydra::horizon::services::timesrv::internal {

class SteadyClock {
  public:
    explicit SteadyClock(hw::WallClock& wall_clock_);

    u64 updateOffset();
    u64 getTimePoint() const;

  private:
    hw::WallClock& wall_clock;
    u64 offset_ns;

  public:
    GETTER(offset_ns, getOffsetNs);
};

} // namespace hydra::horizon::services::timesrv::internal
