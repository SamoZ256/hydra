#pragma once

#define CHECK_AND_SET_PROCESS(dst_process, src_process)                        \
    {                                                                          \
        ASSERT(!dst_process, Loader, "Cannot load multiple processes");        \
        dst_process = src_process;                                             \
    }

#define CHECK_AND_RETURN_PROCESS(process)                                      \
    {                                                                          \
        ASSERT(process, Loader, "Failed to load process");                     \
        return process;                                                        \
    }

namespace hydra::horizon::kernel {
class Process;
}

namespace hydra::horizon::loader {

class LoaderBase {
  public:
    virtual ~LoaderBase() = default;

    // TODO: remove the filename arg
    virtual kernel::Process*
    LoadProcess(StreamReader reader, const std::string_view rom_filename) = 0;
    // TODO: load logo
};

} // namespace hydra::horizon::loader
