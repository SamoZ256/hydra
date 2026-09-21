#pragma once

namespace hydra::horizon::services::timesrv::internal {

class SteadyClock;

class SystemClock {
  public:
    explicit SystemClock(SteadyClock& steady_clock_);

    u64 updateOffset();
    u64 getTimePoint() const;

  private:
    SteadyClock& steady_clock;
    u64 offset_s;
    u64 steady_time_point;

  public:
    GETTER(offset_s, getOffsetS);
    GETTER(steady_time_point, getSteadyTimePoint);
};

} // namespace hydra::horizon::services::timesrv::internal
