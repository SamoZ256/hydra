#include "core/horizon/loader/loader.hpp"

#include <stb_image.h>

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/disk_file.hpp"
#include "core/horizon/filesystem/romfs/romfs.hpp"
#include "core/horizon/loader/homebrew_loader.hpp"
#include "core/horizon/loader/nca_loader.hpp"
#include "core/horizon/loader/nro_loader.hpp"
#include "core/horizon/loader/nso_loader.hpp"
#include "core/horizon/loader/nx_loader.hpp"
#include "core/horizon/loader/plugins/manager.hpp"

namespace hydra::horizon::loader {

namespace {

uchar4* LoadImage(filesystem::IFile* file, u32& out_width, u32& out_height) {
    auto stream = file->Open(filesystem::FileOpenFlags::Read);

    std::vector<u8> raw_data(stream->getSize());
    stream->readToSpan(std::span(raw_data));

    delete stream;

    i32 w;
    i32 h;
    i32 comp;
    auto data = reinterpret_cast<uchar4*>(stbi_load_from_memory(
        raw_data.data(), static_cast<i32>(raw_data.size()), &w, &h, &comp,
        STBI_rgb_alpha));
    if (data == nullptr) {
        LOG_ERROR(Loader, "Failed to load image");
        return nullptr;
    }

    out_width = static_cast<u32>(w);
    out_height = static_cast<u32>(h);
    return data;
}

uchar4* LoadGIF(filesystem::IFile* file,
                std::vector<std::chrono::milliseconds>& out_delays,
                u32& out_width, u32& out_height, u32& out_frame_count) {
    auto stream = file->Open(filesystem::FileOpenFlags::Read);

    std::vector<u8> raw_data(stream->getSize());
    stream->readToSpan(std::span(raw_data));

    delete stream;

    i32 w;
    i32 h;
    i32 f;
    i32 comp;
    i32* delays_ms;
    auto data = reinterpret_cast<uchar4*>(stbi_load_gif_from_memory(
        raw_data.data(), static_cast<i32>(raw_data.size()), &delays_ms, &w, &h,
        &f, &comp, STBI_rgb_alpha));
    if (data == nullptr) {
        LOG_ERROR(Loader, "Failed to load GIF");
        return nullptr;
    }

    out_width = static_cast<u32>(w);
    out_height = static_cast<u32>(h);
    out_frame_count = static_cast<u32>(f);

    out_delays.reserve(static_cast<usize>(out_frame_count));
    for (u32 i = 0; i < out_frame_count; i++)
        out_delays.emplace_back(delays_ms[i]);
    free(delays_ms);

    return data;
}

} // namespace

std::optional<ILoader*>
ILoader::CreateFromPath(std::string_view path,
                        std::optional<plugins::Manager*> plugin_manager_opt) {
    while (path.back() == '/') {
        path.remove_suffix(1);
    }

    // Check if the path exists
    if (!std::filesystem::exists(path))
        return std::nullopt;

    // Create loader
    const auto ext = std::string_view(path).substr(path.find_last_of('.'));
    if (ext == ".nx") {
        if (!std::filesystem::is_directory(path))
            return std::nullopt;
        const auto dir = new horizon::filesystem::Directory(path);
        return new horizon::loader::NxLoader(*dir);
    } else {
        if (!std::filesystem::is_regular_file(path))
            return std::nullopt;
        const auto file = new horizon::filesystem::DiskFile(path);
        if (ext == ".nro") {
            // Assumes that all NROs are Homebrew
            return new horizon::loader::HomebrewLoader(file);
        } else if (ext == ".nso") {
            return new horizon::loader::NsoLoader(file);
        } else if (ext == ".nca") {
            return new horizon::loader::NcaLoader(file);
        } else {
            // Check if we need to create a temporary plugin manager
            std::unique_ptr<plugins::Manager> tmp_plugin_manager;
            if (!plugin_manager_opt)
                tmp_plugin_manager = std::make_unique<plugins::Manager>();
            auto& plugin_manager =
                (plugin_manager_opt ? *plugin_manager_opt.value()
                                    : *tmp_plugin_manager);

            // First, check if any of the loader plugins supports this format
            auto plugin = plugin_manager.FindPluginForFormat(ext.substr(1));
            if (plugin == nullptr)
                return std::nullopt;

            return plugin->Load(path);
        }
    }
}

horizon::services::ns::ApplicationControlProperty* ILoader::LoadNacp() {
    if (nacp_file == nullptr)
        return nullptr;

    auto stream = nacp_file->Open(filesystem::FileOpenFlags::Read);

    ASSERT(stream->getSize() ==
               sizeof(horizon::services::ns::ApplicationControlProperty),
           Loader, "Invalid NACP file size 0x{:x}", stream->getSize());
    auto nacp = new horizon::services::ns::ApplicationControlProperty();
    stream->readToRef(*nacp);

    delete stream;

    return nacp;
}

uchar4* ILoader::LoadIcon(u32& out_width, u32& out_height) {
    if (icon_file == nullptr)
        return nullptr;

    return LoadImage(icon_file, out_width, out_height);
}

uchar4* ILoader::LoadNintendoLogo(u32& out_width, u32& out_height) {
    if (nintendo_logo_file == nullptr)
        return nullptr;

    return LoadImage(nintendo_logo_file, out_width, out_height);
}

uchar4*
ILoader::LoadStartupMovie(std::vector<std::chrono::milliseconds>& out_delays,
                          u32& out_width, u32& out_height,
                          u32& out_frame_count) {
    if (startup_movie_file == nullptr)
        return nullptr;

    return LoadGIF(startup_movie_file, out_delays, out_width, out_height,
                   out_frame_count);
}

void ILoader::ExtractExeFs(std::string_view path) const {
    ASSERT(exefs_dir != nullptr, Loader, "No exeFS");
    LOG_INFO(Loader, "Exporting exeFS to \"{}\"", path);
    exefs_dir->Save(path);
}

void ILoader::ExtractRomFs(std::string_view path) const {
    ASSERT(romfs_entry != nullptr, Loader, "No romFS");
    LOG_INFO(Loader, "Exporting romFS to \"{}\"", path);
    if (romfs_entry->IsDirectory()) {
        static_cast<filesystem::Directory*>(romfs_entry)->Save(path);
    } else {
        filesystem::romfs::RomFS romfs(
            static_cast<filesystem::IFile*>(romfs_entry));
        romfs.Save(path);
    }
}

void ILoader::ExtractIcon(std::string_view path) const {
    ASSERT(icon_file != nullptr, Loader, "No icon");
    LOG_INFO(Loader, "Exporting icon to \"{}\"", path);
    icon_file->Save(path);
}

} // namespace hydra::horizon::loader
