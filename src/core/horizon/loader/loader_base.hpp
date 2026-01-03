#pragma once

#include "core/horizon/filesystem/file.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/ns/const.hpp"

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

    virtual void LoadProcess(kernel::Process* process) = 0;
    enum class LoadNacpError {
        InvalidSize,
    };
    horizon::services::ns::ApplicationControlProperty* LoadNacp();
    uchar4* LoadIcon(u32& out_width, u32& out_height);
    uchar4* LoadNintendoLogo(u32& out_width, u32& out_height);
    uchar4* LoadStartupMovie(std::vector<std::chrono::milliseconds>& out_delays,
                             u32& out_width, u32& out_height,
                             u32& out_frame_count);

  protected:
    filesystem::IFile* nacp_file = nullptr;
    filesystem::IFile* icon_file = nullptr;
    filesystem::IFile* nintendo_logo_file = nullptr;
    filesystem::IFile* startup_movie_file = nullptr;
};

} // namespace hydra::horizon::loader
