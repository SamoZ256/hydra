#include "core/horizon/services/timesrv/internal/time_manager.hpp"

#include "date/tz.h"

#include "core/horizon/filesystem/content_archive.hpp"
#include "core/horizon/filesystem/file.hpp"
#include "core/horizon/filesystem/romfs/romfs.hpp"
#include "core/horizon/services/timesrv/internal/tzif.hpp"
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
    : system{system_}, steady_clock(system.GetWallClock()),
      system_clock(steady_clock), shared_memory{new kernel::SharedMemory(
                                      system.GetCpu(), SHARED_MEMORY_SIZE)} {
    // Clock
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

    // Time zone
    LoadTimeZoneRule(GetDeviceLocationName(), my_time_zone_rule);
}

std::string_view TimeManager::GetDeviceLocationName() {
    // TODO: make this configurable
    const auto* tz = date::current_zone();
    return tz->name();
}

void TimeManager::LoadTimeZoneRule(std::string_view location_name,
                                   TimeZoneRule& out_rule) {
    LOG_DEBUG(Services, "Location name: {}", location_name);

    // NCA
    filesystem::IFile* time_zone_archive_file;
    auto res = system.GetOS().GetFilesystem().GetFile(
        FS_FIRMWARE_PATH "/TimeZoneBinary", time_zone_archive_file);
    if (res != filesystem::FsResult::Success) {
        // TODO: return error?
        LOG_ERROR(Services, "Failed to get time zone binary archive: {}", res);
        return;
    }

    filesystem::ContentArchive time_zone_archive(time_zone_archive_file);

    // Data
    filesystem::IFile* data_file;
    res = time_zone_archive.GetFile("data", data_file);
    if (res != filesystem::FsResult::Success) {
        // TODO: return error?
        LOG_ERROR(Services, "Failed to get time zone data: {}", res);
        return;
    }

    filesystem::romfs::RomFS romfs(data_file);

    // Info file
    filesystem::IFile* info_file;
    res = romfs.GetFile(fmt::format("zoneinfo/{}", location_name), info_file);
    if (res != filesystem::FsResult::Success) {
        // TODO: return error?
        LOG_ERROR(Services, "Failed to get time zone info: {}", res);
        return;
    }

    const auto stream = info_file->Open(filesystem::FileOpenFlags::Read);
    internal::ParseTimeZoneBinary(stream, out_rule);
    delete stream;
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
