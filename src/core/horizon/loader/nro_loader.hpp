#pragma once

#include "core/horizon/loader/loader_base.hpp"

namespace hydra::horizon::loader {

class NroLoader : public LoaderBase {
  public:
    enum class Error {
        InvalidMagic,
    };

    NroLoader(filesystem::IFile* file_, const bool is_entry_point_);

    void LoadProcess(kernel::Process* process) override;

    vaddr_t GetEntryPoint() const;

  private:
    filesystem::IFile* file;
    const bool is_entry_point;

    u64 size;
    u32 text_offset;
    u32 bss_size;

    uptr executable_ptr{invalid<uptr>()};
    vaddr_t executable_base{invalid<vaddr_t>()};
    usize executable_size{invalid<usize>()};

    void TryLoadAssetSection(filesystem::IFile* file);

  public:
    GETTER(size, GetSize);
    GETTER(executable_ptr, GetExecutablePtr);
    GETTER(executable_base, GetExecutableBase);
    GETTER(executable_size, GetExecutableSize);
};

} // namespace hydra::horizon::loader
