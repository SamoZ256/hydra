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

TimeManager::TimeManager(System& system_)
    : system{system_}, steady_clock(system.getWallClock()),
      system_clock(steady_clock),
      time_zone_manager(system.getOs().getFilesystem()),
      shared_memory{
          new kernel::SharedMemory(system.getCpu(), SHARED_MEMORY_SIZE)} {
    // Clock
    updateSteadyClockContext();
    updateSystemClockContext();

    // TODO: implement
    writeAutomaticCorrectionEnabled(false);
    writeContinuousAdjustmentTimePoint(
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

    // Time zone
    time_zone_manager.loadMyRule();
}

void TimeManager::updateSteadyClockContext() {
    writeSteadyClockContext({
        .internal_offset = steady_clock.getOffsetNs(),
        .clock_source_id = CLOCK_SOURCE_ID,
    });
}

void TimeManager::updateSystemClockContext() {
    // TODO: handle local and network separately?
    writeSystemClockContext({
        .offset = static_cast<i64>(system_clock.getOffsetS()),
        .steady_time_point =
            {
                .time_point = system_clock.getSteadyTimePoint(),
                .clock_source_id = CLOCK_SOURCE_ID,
            },
    });
}

void TimeManager::writeSteadyClockContext(const SteadyClockContext& context) {
    writeObjectToSharedMemory(STEADY_CLOCK_CONTEXT_OFFSET, 0x4, context);
}

void TimeManager::writeLocalSystemClockContext(
    const SystemClockContext& context) {
    writeObjectToSharedMemory(LOCAL_SYSTEM_CLOCK_CONTEXT_OFFSET, 0x4, context);
}

void TimeManager::writeNetworkSystemClockContext(
    const SystemClockContext& context) {
    writeObjectToSharedMemory(NETWORK_SYSTEM_CLOCK_CONTEXT_OFFSET, 0x4,
                              context);
}

void TimeManager::writeSystemClockContext(const SystemClockContext& context) {
    writeLocalSystemClockContext(context);
    writeNetworkSystemClockContext(context);
}

void TimeManager::writeAutomaticCorrectionEnabled(bool enabled) {
    writeObjectToSharedMemory(AUTOMATIC_CORRECTION_ENABLED_OFFSET, 0x0,
                              enabled);
}

void TimeManager::writeContinuousAdjustmentTimePoint(
    const ContinuousAdjustmentTimePoint& time_point) {
    writeObjectToSharedMemory(CONTINUOUS_ADJUSTMENT_TIME_POINT_OFFSET, 0x4,
                              time_point);
}

} // namespace hydra::horizon::services::timesrv::internal
