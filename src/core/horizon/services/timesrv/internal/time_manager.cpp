#include "core/horizon/services/timesrv/internal/time_manager.hpp"

#include "core/system.hpp"

namespace hydra::horizon::services::timesrv::internal {

namespace {

constexpr usize SHARED_MEMORY_SIZE = 0x1000;

constexpr u32 STEADY_CLOCK_CONTEXT_OFFSET = 0x00;
constexpr u32 LOCAL_SYSTEM_CLOCK_CONTEXT_OFFSET = 0x38;
constexpr u32 NETWORK_SYSTEM_CLOCK_CONTEXT_OFFSET = 0x80;
constexpr u32 AUTOMATIC_CORRECTION_ENABLED_OFFSET = 0xc8;
constexpr u32 CONTINUOUS_ADJUSTMENT_TIME_POINT_OFFSET = 0xd0;

} // namespace

TimeManager::TimeManager(System& system)
    : shared_memory{
          new kernel::SharedMemory(system.GetCpu(), SHARED_MEMORY_SIZE)} {
    std::memset(reinterpret_cast<void*>(shared_memory->GetPtr()), 0,
                SHARED_MEMORY_SIZE);

    (void)STEADY_CLOCK_CONTEXT_OFFSET;
    (void)LOCAL_SYSTEM_CLOCK_CONTEXT_OFFSET;
    (void)NETWORK_SYSTEM_CLOCK_CONTEXT_OFFSET;
    (void)AUTOMATIC_CORRECTION_ENABLED_OFFSET;
    (void)CONTINUOUS_ADJUSTMENT_TIME_POINT_OFFSET;

    // HACK: initialize shared memory
    /*
    WriteObjectToSharedMemory(STEADY_CLOCK_CONTEXT_OFFSET, 0x4,
                              SteadyClockContext{
                                  .internal_offset = 0,
                                  .clock_source_id = 0x1,
                              });
    WriteObjectToSharedMemory(LOCAL_SYSTEM_CLOCK_CONTEXT_OFFSET, 0x4,
                              SystemClockContext{
                                  .offset = 0,
                                  .steady_time_point =
                                      {
                                          .time_point = 0,
                                          .clock_source_id = 0x1,
                                      },
                              });
    WriteObjectToSharedMemory(NETWORK_SYSTEM_CLOCK_CONTEXT_OFFSET, 0x4,
                              SystemClockContext{
                                  .offset = 0,
                                  .steady_time_point =
                                      {
                                          .time_point = 0,
                                          .clock_source_id = 0x1,
                                      },
                              });
    WriteObjectToSharedMemory(AUTOMATIC_CORRECTION_ENABLED_OFFSET, 0x0, false);
    WriteObjectToSharedMemory(
        CONTINUOUS_ADJUSTMENT_TIME_POINT_OFFSET, 0x4,
        ContinuousAdjustmentTimePoint{.clock_offset = 0,
                                      .multiplier = 1,
                                      .divisor_log2 = 0,
                                      .context = {
                                          .offset = 0,
                                          .steady_time_point =
                                              {
                                                  .time_point = 0,
                                                  .clock_source_id = 0x1,
                                              },
                                      }});
                                      */
}

} // namespace hydra::horizon::services::timesrv::internal
