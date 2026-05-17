#pragma once

#include "core/horizon/kernel/shared_memory.hpp"

namespace hydra {
class System;
}

namespace hydra::horizon::services::timesrv::internal {

struct SteadyClockTimePoint {
    u64 time_point;
    u128 clock_source_id;
};

struct SteadyClockContext {
    u64 internal_offset;
    u128 clock_source_id;
};

struct SystemClockContext {
    i64 offset;
    SteadyClockTimePoint steady_time_point;
};

struct ContinuousAdjustmentTimePoint {
    u64 clock_offset;
    i64 multiplier;
    i64 divisor_log2;
    SystemClockContext context;
};

class TimeManager {
  public:
    TimeManager(System& system);

  private:
    kernel::SharedMemory* shared_memory;

    // From Ryujinx
    template <typename T>
    void WriteObjectToSharedMemory(u32 offset, u32 padding, T value) {
        u32 new_index =
            *reinterpret_cast<u32*>(shared_memory->GetPtr() + offset) + 1;

        u32 object_offset =
            offset + 4 + padding + u32((new_index & 0x1) * sizeof(T));

        *reinterpret_cast<T*>(shared_memory->GetPtr() + object_offset) = value;

        // TODO: memory barrier?

        *reinterpret_cast<u32*>(shared_memory->GetPtr() + offset) = new_index;
    }

  public:
    GETTER(shared_memory, GetSharedMemory);
};

} // namespace hydra::horizon::services::timesrv::internal
