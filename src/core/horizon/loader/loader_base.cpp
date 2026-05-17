#include "core/horizon/loader/loader_base.hpp"

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

    std::vector<u8> raw_data(stream->GetSize());
    stream->ReadToSpan(std::span(raw_data));

    delete stream;

    i32 w, h;
    i32 comp;
    auto data = reinterpret_cast<uchar4*>(stbi_load_from_memory(
        raw_data.data(), static_cast<i32>(raw_data.size()), &w, &h, &comp,
        STBI_rgb_alpha));
    if (!data) {
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

    std::vector<u8> raw_data(stream->GetSize());
    stream->ReadToSpan(std::span(raw_data));

    delete stream;

    i32 w, h, f;
    i32 comp;
    i32* delays_ms;
    auto data = reinterpret_cast<uchar4*>(stbi_load_gif_from_memory(
        raw_data.data(), static_cast<i32>(raw_data.size()), &delays_ms, &w, &h,
        &f, &comp, STBI_rgb_alpha));
    if (!data) {
        LOG_ERROR(Loader, "Failed to load GIF");
        return nullptr;
    }

    out_width = static_cast<u32>(w);
    out_height = static_cast<u32>(h);
    out_frame_count = static_cast<u32>(f);

    out_delays.reserve(static_cast<usize>(out_frame_count));
    for (u32 i = 0; i < out_frame_count; i++)
        out_delays.push_back(std::chrono::milliseconds(delays_ms[i]));
    free(delays_ms);

    return data;
}

} // namespace

LoaderBase* LoaderBase::CreateFromPath(std::string_view path,
                                       plugins::Manager* plugin_manager) {
    while (path.back() == '/') {
        path.remove_suffix(1);
    }

    // Check if the path exists
    ASSERT_THROWING(std::filesystem::exists(path), Loader,
                    CreateFromPathError::DoesNotExist,
                    "Path \"{}\" does not exist", path);

    // Create loader
    const auto ext = std::string_view(path).substr(path.find_last_of("."));
    horizon::loader::LoaderBase* loader{nullptr};
    if (ext == ".nx") {
        const auto dir = new horizon::filesystem::Directory(path);
        loader = new horizon::loader::NxLoader(*dir);
    } else {
        const auto file = new horizon::filesystem::DiskFile(path);
        if (ext == ".nro") {
            // Assumes that all NROs are Homebrew
            loader = new horizon::loader::HomebrewLoader(file);
        } else if (ext == ".nso") {
            loader = new horizon::loader::NsoLoader(file);
        } else if (ext == ".nca") {
            loader = new horizon::loader::NcaLoader(file);
        } else {
            // Check if we need to create a temporary plugin manager
            bool has_plugin_manager = (plugin_manager != nullptr);
            if (!has_plugin_manager)
                plugin_manager = new plugins::Manager();

            // First, check if any of the loader plugins supports this format
            auto plugin = plugin_manager->FindPluginForFormat(ext.substr(1));
            ASSERT_THROWING(
                plugin, Loader, CreateFromPathError::UnsupportedExtension,
                "Unsupported extension \"{}\" (path: \"{}\")", ext, path);

            loader = plugin->Load(path);

            if (!has_plugin_manager)
                delete plugin_manager;
        }
    }

    return loader;
}

horizon::services::ns::ApplicationControlProperty* LoaderBase::LoadNacp() {
    if (!nacp_file)
        return nullptr;

    auto stream = nacp_file->Open(filesystem::FileOpenFlags::Read);

    ASSERT_THROWING(
        stream->GetSize() ==
            sizeof(horizon::services::ns::ApplicationControlProperty),
        Loader, LoadNacpError::InvalidSize, "Invalid NACP file size 0x{:x}",
        stream->GetSize());
    auto nacp = new horizon::services::ns::ApplicationControlProperty();
    stream->ReadToRef(*nacp);

    delete stream;

    return nacp;
}

uchar4* LoaderBase::LoadIcon(u32& out_width, u32& out_height) {
    if (!icon_file)
        return nullptr;

    return LoadImage(icon_file, out_width, out_height);
}

uchar4* LoaderBase::LoadNintendoLogo(u32& out_width, u32& out_height) {
    if (!nintendo_logo_file)
        return nullptr;

    return LoadImage(nintendo_logo_file, out_width, out_height);
}

uchar4*
LoaderBase::LoadStartupMovie(std::vector<std::chrono::milliseconds>& out_delays,
                             u32& out_width, u32& out_height,
                             u32& out_frame_count) {
    if (!startup_movie_file)
        return nullptr;

    return LoadGIF(startup_movie_file, out_delays, out_width, out_height,
                   out_frame_count);
}

void LoaderBase::ExtractExeFs(std::string_view path) const {
    ASSERT(exefs_dir != nullptr, Loader, "No exeFS");
    LOG_INFO(Loader, "Exporting exeFS to \"{}\"", path);
    exefs_dir->Save(path);
}

void LoaderBase::ExtractRomFs(std::string_view path) const {
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

void LoaderBase::ExtractIcon(std::string_view path) const {
    ASSERT(icon_file != nullptr, Loader, "No icon");
    LOG_INFO(Loader, "Exporting icon to \"{}\"", path);
    icon_file->Save(path);
}

} // namespace hydra::horizon::loader
