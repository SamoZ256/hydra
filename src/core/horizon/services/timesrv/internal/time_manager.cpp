#include "core/horizon/services/timesrv/internal/time_manager.hpp"

#include "core/system.hpp"

// TODO: update contexts after waking from sleep

namespace hydra::horizon::services::timesrv::internal {

namespace {

constexpr u64 SHARED_MEMORY_SIZE = 0x1000;

constexpr u32 STEADY_CLOCK_CONTEXT_OFFSET = 0x00;
constexpr u32 LOCAL_SYSTEM_CLOCK_CONTEXT_OFFSET = 0x38;
constexpr u32 NETWORK_SYSTEM_CLOCK_CONTEXT_OFFSET = 0x80;
constexpr u32 AUTOMATIC_CORRECTION_ENABLED_OFFSET = 0xc8;
constexpr u32 CONTINUOUS_ADJUSTMENT_TIME_POINT_OFFSET = 0xd0;

} // namespace

TimeManager::TimeManager(System& system)
    : steady_clock(system.GetWallClock()),
      system_clock(steady_clock), shared_memory{new kernel::SharedMemory(
                                      system.GetCpu(), SHARED_MEMORY_SIZE)} {
    UpdateSteadyClockContext();
    UpdateSystemClockContext();

    // TODO: implement
    WriteAutomaticCorrectionEnabled(false);
    WriteContinuousAdjustmentTimePoint(
        {.clock_offset = 0,
         .multiplier = 1,
         .divisor_log2 = 0,
         .context = {
             .offset = 0,
             .steady_time_point =
                 {
                     .time_point = 0,
                     .clock_source_id = CLOCK_SOURCE_ID,
                 },
         }});
}

void TimeManager::UpdateSteadyClockContext() {
    WriteSteadyClockContext({
        .internal_offset = steady_clock.GetOffsetNs(),
        .clock_source_id = CLOCK_SOURCE_ID,
    });
}

void TimeManager::UpdateSystemClockContext() {
    // TODO: handle local and network separately?
    WriteSystemClockContext({
        .offset = static_cast<i64>(system_clock.GetOffsetS()),
        .steady_time_point =
            {
                .time_point = system_clock.GetSteadyTimePoint(),
                .clock_source_id = CLOCK_SOURCE_ID,
            },
    });
}

void TimeManager::WriteSteadyClockContext(const SteadyClockContext& context) {
    WriteObjectToSharedMemory(STEADY_CLOCK_CONTEXT_OFFSET, 0x4, context);
}

void TimeManager::WriteLocalSystemClockContext(
    const SystemClockContext& context) {
    WriteObjectToSharedMemory(LOCAL_SYSTEM_CLOCK_CONTEXT_OFFSET, 0x4, context);
}

void TimeManager::WriteNetworkSystemClockContext(
    const SystemClockContext& context) {
    WriteObjectToSharedMemory(NETWORK_SYSTEM_CLOCK_CONTEXT_OFFSET, 0x4,
                              context);
}

void TimeManager::WriteSystemClockContext(const SystemClockContext& context) {
    WriteLocalSystemClockContext(context);
    WriteNetworkSystemClockContext(context);
}

void TimeManager::WriteAutomaticCorrectionEnabled(bool enabled) {
    WriteObjectToSharedMemory(AUTOMATIC_CORRECTION_ENABLED_OFFSET, 0x0,
                              enabled);
}

void TimeManager::WriteContinuousAdjustmentTimePoint(
    const ContinuousAdjustmentTimePoint& time_point) {
    WriteObjectToSharedMemory(CONTINUOUS_ADJUSTMENT_TIME_POINT_OFFSET, 0x4,
                              time_point);
}

} // namespace hydra::horizon::services::timesrv::internal
