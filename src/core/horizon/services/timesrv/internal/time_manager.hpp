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
    explicit TimeManager(System& system_);

  private:
    System& system;

    SteadyClock steady_clock;
    SystemClock system_clock;
    TimeZoneManager time_zone_manager;

    kernel::SharedMemory* shared_memory;

    // Helpers

    // Update
    void updateSteadyClockContext();
    void updateSystemClockContext();

    // Write
    void writeSteadyClockContext(const SteadyClockContext& context);
    void writeLocalSystemClockContext(const SystemClockContext& context);
    void writeNetworkSystemClockContext(const SystemClockContext& context);
    void writeSystemClockContext(const SystemClockContext& context);
    void writeAutomaticCorrectionEnabled(bool enabled);
    void writeContinuousAdjustmentTimePoint(
        const ContinuousAdjustmentTimePoint& time_point);

    // From Ryujinx
    template <typename T>
    void writeObjectToSharedMemory(u32 offset, u32 padding, T value) {
        u32 new_index =
            *reinterpret_cast<u32*>(shared_memory->getPtr() + offset) + 1;

        u32 object_offset = offset + 4 + padding +
                            static_cast<u32>((new_index & 0x1) * sizeof(T));

        *reinterpret_cast<T*>(shared_memory->getPtr() + object_offset) = value;

        // TODO: memory barrier?

        *reinterpret_cast<u32*>(shared_memory->getPtr() + offset) = new_index;
    }

  public:
    REF_GETTER(steady_clock, getSteadyClock);
    REF_GETTER(system_clock, getSystemClock);
    REF_GETTER(time_zone_manager, getTimeZoneManager);
    GETTER(shared_memory, getSharedMemory);
};

} // namespace hydra::horizon::services::timesrv::internal
