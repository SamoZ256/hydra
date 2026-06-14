#include "core/horizon/services/timesrv/internal/time_zone_manager.hpp"

#include "date/tz.h"

#include "core/horizon/filesystem/content_archive.hpp"
#include "core/horizon/filesystem/file.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/filesystem/romfs/romfs.hpp"
#include "core/horizon/services/timesrv/internal/tzif.hpp"

namespace hydra::horizon::services::timesrv::internal {

TimeZoneManager::TimeZoneManager(filesystem::Filesystem& filesystem_)
    : filesystem{filesystem_} {
    // Get available locations

    // NCA
    filesystem::IFile* time_zone_archive_file;
    auto res = filesystem.GetFile(FS_FIRMWARE_PATH "/TimeZoneBinary",
                                  time_zone_archive_file);
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

    // List file
    filesystem::IFile* list_file;
    res = romfs.GetFile("binaryList.txt", list_file);

    const auto stream = list_file->Open(filesystem::FileOpenFlags::Read);
    char buffer[256];
    u32 str_size = 0;
    while (stream->GetRemainingSize()) {
        const auto c = stream->Read<char>();
        switch (c) {
        case '\n': {
            std::string_view str(buffer, str_size);
            if (!str.empty()) {
                locations.insert(std::string(str));
                str_size = 0;
            }
            break;
        }
        case '\r':
            // Skip
            break;
        default:
            buffer[str_size++] = c;
            break;
        }
    }
    delete stream;

    // Load my rule
    LoadTimeZoneRule(GetDeviceLocationName(), my_time_zone_rule);
}

std::string_view TimeZoneManager::GetDeviceLocationName() {
    std::string_view name;
    if (true) { // TODO: if auto location
        const auto* tz = date::current_zone();
        name = tz->name();
    } else {
        // TODO
    }

    // Verify that the location is available
    if (!locations.contains(name)) {
        LOG_WARN(Services,
                 "Unsupported device location \"{}\", falling back to UTC",
                 name);
        name = "UTC";
    }

    return name;
}

void TimeZoneManager::LoadTimeZoneRule(std::string_view location_name,
                                       TimeZoneRule& out_rule) {
    LOG_DEBUG(Services, "Location name: {}", location_name);

    // NCA
    filesystem::IFile* time_zone_archive_file;
    auto res = filesystem.GetFile(FS_FIRMWARE_PATH "/TimeZoneBinary",
                                  time_zone_archive_file);
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

} // namespace hydra::horizon::services::timesrv::internal
