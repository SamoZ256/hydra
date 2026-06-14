#pragma once

#include "core/horizon/kernel/shared_memory.hpp"
#include "core/horizon/services/timesrv/internal/steady_clock.hpp"
#include "core/horizon/services/timesrv/internal/system_clock.hpp"
#include "core/horizon/services/timesrv/internal/time_zone_manager.hpp"

namespace hydra {
class System;
}

namespace hydra::horizon::services::timesrv::internal {

class TimeManager {
  public:
    TimeManager(System& system_);

  private:
    System& system;

    SteadyClock steady_clock;
    SystemClock system_clock;
    TimeZoneManager time_zone_manager;

    kernel::SharedMemory* shared_memory;

    // Helpers

    // Update
    void UpdateSteadyClockContext();
    void UpdateSystemClockContext();

    // Write
    void WriteSteadyClockContext(const SteadyClockContext& context);
    void WriteLocalSystemClockContext(const SystemClockContext& context);
    void WriteNetworkSystemClockContext(const SystemClockContext& context);
    void WriteSystemClockContext(const SystemClockContext& context);
    void WriteAutomaticCorrectionEnabled(bool enabled);
    void WriteContinuousAdjustmentTimePoint(
        const ContinuousAdjustmentTimePoint& time_point);

    // From Ryujinx
    template <typename T>
    void WriteObjectToSharedMemory(u32 offset, u32 padding, T value) {
        u32 new_index =
            *reinterpret_cast<u32*>(shared_memory->GetPtr() + offset) + 1;

        u32 object_offset = offset + 4 + padding +
                            static_cast<u32>((new_index & 0x1) * sizeof(T));

        *reinterpret_cast<T*>(shared_memory->GetPtr() + object_offset) = value;

        // TODO: memory barrier?

        *reinterpret_cast<u32*>(shared_memory->GetPtr() + offset) = new_index;
    }

  public:
    REF_GETTER(steady_clock, GetSteadyClock);
    REF_GETTER(system_clock, GetSystemClock);
    REF_GETTER(time_zone_manager, GetTimeZoneManager);
    GETTER(shared_memory, GetSharedMemory);
};

} // namespace hydra::horizon::services::timesrv::internal
