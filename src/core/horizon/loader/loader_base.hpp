#pragma once

#include "core/horizon/filesystem/file.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/ns/const.hpp"

namespace hydra {
class System;
}

namespace hydra::horizon::filesystem {
class Filesystem;
}

namespace hydra::horizon::loader {

class LoaderBase {
  public:
    enum class CreateFromPathError {
        DoesNotExist,
        UnsupportedExtension,
    };
    static LoaderBase* CreateFromPath(std::string_view path);

    virtual ~LoaderBase() = default;

    virtual u64 GetTitleID() const { return invalid<u64>(); }

    virtual void LoadProcess(System& system, kernel::Process* process) = 0;
    enum class LoadNacpError {
        InvalidSize,
    };
    horizon::services::ns::ApplicationControlProperty* LoadNacp();
    uchar4* LoadIcon(u32& out_width, u32& out_height);
    uchar4* LoadNintendoLogo(u32& out_width, u32& out_height);
    uchar4* LoadStartupMovie(std::vector<std::chrono::milliseconds>& out_delays,
                             u32& out_width, u32& out_height,
                             u32& out_frame_count);

    // Extracting
    bool HasIcon() const { return icon_file != nullptr; }
    void ExtractIcon(std::string_view path) const;
    bool HasExeFs() const { return exefs_dir != nullptr; }
    void ExtractExeFs(std::string_view path) const;
    bool HasRomFs() const { return romfs_entry != nullptr; }
    void ExtractRomFs(std::string_view path) const;

  protected:
    filesystem::IFile* nacp_file{nullptr};
    filesystem::IFile* icon_file{nullptr};
    filesystem::IFile* nintendo_logo_file{nullptr};
    filesystem::IFile* startup_movie_file{nullptr};
    filesystem::Directory* exefs_dir{nullptr};
    filesystem::IEntry* romfs_entry{nullptr};
};

} // namespace hydra::horizon::loader
