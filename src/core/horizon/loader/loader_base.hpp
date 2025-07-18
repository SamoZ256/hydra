#pragma once

#include "core/horizon/filesystem/file_base.hpp"
#include "core/horizon/kernel/process.hpp"

#define CHECK_AND_SET_PROCESS_PARAMS(dst_process, src_process)                 \
    {                                                                          \
        ASSERT(!dst_process, Loader, "Cannot load multiple processes");        \
        dst_process = src_process;                                             \
    }

#define CHECK_AND_RETURN_PROCESS_PARAMS(process)                               \
    {                                                                          \
        ASSERT(process, Loader, "Failed to load process");                     \
        return process;                                                        \
    }

namespace hydra::horizon::loader {

class LoaderBase {
  public:
    static LoaderBase* CreateFromFile(const std::string& path);

    virtual ~LoaderBase() = default;

    virtual u64 GetTitleID() const { return invalid<u64>(); }
    // TODO: NACP

    virtual std::optional<kernel::ProcessParams> LoadProcess() = 0;
    void LoadIcon(uchar4*& out_data, usize& out_width, usize& out_height);
    void LoadNintendoLogo(uchar4*& out_data, usize& out_width,
                          usize& out_height);
    void LoadStartupMovie(uchar4*& out_data,
                          std::vector<std::chrono::milliseconds>& out_delays,
                          usize& out_width, usize& out_height,
                          u32& out_frame_count);

  protected:
    // TODO: NACP

    filesystem::FileBase* icon_file = nullptr;
    filesystem::FileBase* nintendo_logo_file = nullptr;
    filesystem::FileBase* startup_movie_file = nullptr;
};

} // namespace hydra::horizon::loader
