#include "core/horizon/loader/loader_base.hpp"

#include <stb_image.h>

#include "core/horizon/filesystem/host_file.hpp"
#include "core/horizon/loader/homebrew_loader.hpp"
#include "core/horizon/loader/nca_loader.hpp"
#include "core/horizon/loader/nro_loader.hpp"
#include "core/horizon/loader/nso_loader.hpp"
#include "core/horizon/loader/xci_loader.hpp"

namespace hydra::horizon::loader {

namespace {

uchar4* LoadImage(filesystem::FileBase* file, usize& out_width,
                  usize& out_height) {
    auto stream = file->Open(filesystem::FileOpenFlags::Read);
    auto reader = stream.CreateReader();

    usize raw_data_size = reader.GetSize();
    u8* raw_data = new u8[raw_data_size];
    reader.ReadWhole(raw_data);

    file->Close(stream);

    i32 w, h;
    i32 comp;
    auto data = reinterpret_cast<uchar4*>(
        stbi_load_from_memory(raw_data, static_cast<i32>(raw_data_size), &w, &h,
                              &comp, STBI_rgb_alpha));
    delete[] raw_data;
    if (!data) {
        LOG_ERROR(Loader, "Failed to load image");
        return nullptr;
    }

    out_width = w;
    out_height = h;
    return data;
}

uchar4* LoadGIF(filesystem::FileBase* file,
                std::vector<std::chrono::milliseconds>& out_delays,
                usize& out_width, usize& out_height, u32& out_frame_count) {
    auto stream = file->Open(filesystem::FileOpenFlags::Read);
    auto reader = stream.CreateReader();

    usize raw_data_size = reader.GetSize();
    u8* raw_data = new u8[raw_data_size];
    reader.ReadWhole(raw_data);

    file->Close(stream);

    i32 w, h, f;
    i32 comp;
    i32* delays_ms;
    auto data = reinterpret_cast<uchar4*>(stbi_load_gif_from_memory(
        raw_data, static_cast<i32>(raw_data_size), &delays_ms, &w, &h, &f,
        &comp, STBI_rgb_alpha));
    delete[] raw_data;
    if (!data) {
        LOG_ERROR(Loader, "Failed to load GIF");
        return nullptr;
    }

    out_width = w;
    out_height = h;
    out_frame_count = f;

    out_delays.reserve(f);
    for (u32 i = 0; i < f; i++)
        out_delays.push_back(std::chrono::milliseconds(delays_ms[i]));
    free(delays_ms);

    return data;
}

} // namespace

LoaderBase* LoaderBase::CreateFromFile(std::string_view path) {
    // Check if the file exists
    if (!std::filesystem::exists(path)) {
        // TODO: return an error instead
        LOG_FATAL(Other, "Invalid path \"{}\"", path);
        return nullptr;
    }

    // Create loader
    auto file = new horizon::filesystem::HostFile(path);
    const auto extension =
        std::string_view(path).substr(path.find_last_of(".") + 1);

    horizon::loader::LoaderBase* loader{nullptr};
    if (extension == "nro") {
        // Assumes that all NROs are Homebrew
        loader = new horizon::loader::HomebrewLoader(file);
    } else if (extension == "nso") {
        loader = new horizon::loader::NsoLoader(file);
    } else if (extension == "nca") {
        loader = new horizon::loader::NcaLoader(file);
    } else if (extension == "nsp") {
        loader = new horizon::loader::NspLoader(file);
    } else if (extension == "xci") {
        loader = new horizon::loader::XciLoader(file);
    } else {
        // TODO: return an error instead
        LOG_FATAL(Other, "Unknown ROM extension \"{}\"", extension);
        return nullptr;
    }

    return loader;
}

horizon::services::ns::ApplicationControlProperty* LoaderBase::LoadNacp() {
    if (!nacp_file)
        return nullptr;

    auto stream = nacp_file->Open(filesystem::FileOpenFlags::Read);
    auto reader = stream.CreateReader();

    auto nacp = new horizon::services::ns::ApplicationControlProperty();
    ASSERT(reader.GetSize() == sizeof(*nacp), Loader,
           "Invalid NACP file size 0x{:x}", reader.GetSize());
    reader.ReadWhole(nacp);

    nacp_file->Close(stream);

    return nacp;
}

uchar4* LoaderBase::LoadIcon(usize& out_width, usize& out_height) {
    if (!icon_file)
        return nullptr;

    return LoadImage(icon_file, out_width, out_height);
}

uchar4* LoaderBase::LoadNintendoLogo(usize& out_width, usize& out_height) {
    if (!nintendo_logo_file)
        return nullptr;

    return LoadImage(nintendo_logo_file, out_width, out_height);
}

uchar4*
LoaderBase::LoadStartupMovie(std::vector<std::chrono::milliseconds>& out_delays,
                             usize& out_width, usize& out_height,
                             u32& out_frame_count) {
    if (!startup_movie_file)
        return nullptr;

    return LoadGIF(startup_movie_file, out_delays, out_width, out_height,
                   out_frame_count);
}

} // namespace hydra::horizon::loader
