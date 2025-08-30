#pragma once

#include "core/horizon/filesystem/file_base.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/ns/const.hpp"

namespace hydra::horizon::loader {

class LoaderBase {
  public:
    static LoaderBase* CreateFromFile(const std::string& path);

    virtual ~LoaderBase() = default;

    virtual u64 GetTitleID() const { return invalid<u64>(); }

    virtual void LoadProcess(kernel::Process* process) = 0;
    horizon::services::ns::ApplicationControlProperty* LoadNacp();
    bool LoadIcon(uchar4*& out_data, usize& out_width, usize& out_height);
    bool LoadNintendoLogo(uchar4*& out_data, usize& out_width,
                          usize& out_height);
    bool LoadStartupMovie(uchar4*& out_data,
                          std::vector<std::chrono::milliseconds>& out_delays,
                          usize& out_width, usize& out_height,
                          u32& out_frame_count);

  protected:
    filesystem::FileBase* nacp_file = nullptr;
    filesystem::FileBase* icon_file = nullptr;
    filesystem::FileBase* nintendo_logo_file = nullptr;
    filesystem::FileBase* startup_movie_file = nullptr;
};

} // namespace hydra::horizon::loader
