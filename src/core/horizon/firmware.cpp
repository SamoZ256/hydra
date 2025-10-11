#include "core/horizon/firmware.hpp"

#include "core/horizon/filesystem/content_archive.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/filesystem/host_file.hpp"

namespace hydra::horizon {

void try_install_firmware_to_filesystem(filesystem::Filesystem& fs) {
    const auto& firmware_path = CONFIG_INSTANCE.GetFirmwarePath().Get();
    if (!std::filesystem::exists(firmware_path)) {
        LOG_WARN(Horizon, "Firmware path does not exist");
        return;
    }

    std::map<u64, std::string> firmware_titles_map = {
        {0x010000000000080a, "AvatarImage"},
        {0x010000000000080e, "TimeZoneBinary"},
        {0x0100000000000810, "FontNintendoExtension"},
        {0x0100000000000811, "FontStandard"},
        {0x0100000000000812, "FontKorean"},
        {0x0100000000000813, "FontChineseTraditional"},
        {0x0100000000000814, "FontChineseSimple"},
    };

    // Iterate over the directory
    for (const auto& entry :
         std::filesystem::directory_iterator(firmware_path)) {
        auto file = new horizon::filesystem::HostFile(entry.path().string());
        horizon::filesystem::ContentArchive content_archive(file);
        // TODO: find a better way to handle this
        if (content_archive.GetContentType() ==
            horizon::filesystem::ContentArchiveContentType::Meta)
            continue;

        auto res = fs.AddEntry(fmt::format(FS_FIRMWARE_PATH "/{:016x}",
                                           content_archive.GetTitleID()),
                               file, true);
        ASSERT(res == horizon::filesystem::FsResult::Success, Horizon,
               "Failed to add firmware entry: {}", res);
    }

    for (const auto& [title_id, filename] : firmware_titles_map) {
        filesystem::FileBase* file;
        auto res = fs.GetFile(
            fmt::format(FS_FIRMWARE_PATH "/{:016x}", title_id), file);
        ASSERT(res == horizon::filesystem::FsResult::Success, Horizon,
               "Failed to get firmware entry {:016x}: {}", title_id, res);

        res = fs.AddEntry(fmt::format(FS_FIRMWARE_PATH "/{}", filename), file,
                          true);
        ASSERT(res == horizon::filesystem::FsResult::Success, Horizon,
               "Failed to add firmware entry alias \"{}\": {}", filename, res);
    }
}

} // namespace hydra::horizon
