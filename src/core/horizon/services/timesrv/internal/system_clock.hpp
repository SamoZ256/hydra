#pragma once

namespace hydra::horizon::services::timesrv::internal {

class SteadyClock;

class SystemClock {
  public:
    SystemClock(SteadyClock& steady_clock_);

    u64 UpdateOffset();
    u64 GetTimePoint() const;

  private:
    SteadyClock& steady_clock;
    u64 offset_s;
    u64 steady_time_point;

  public:
    GETTER(offset_s, GetOffsetS);
    GETTER(steady_time_point, GetSteadyTimePoint);
};

} // namespace hydra::horizon::services::timesrv::internal
