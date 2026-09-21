#pragma once

#include "core/horizon/loader/loader.hpp"

namespace hydra::horizon::loader {

struct NroSection {
    u32 offset;
    u32 size;
};

class NroLoader : public ILoader {
  public:
    NroLoader(filesystem::IFile* file_, const bool is_entry_point_);

    void loadProcess(System& system, kernel::Process* process) override;

    vaddr_t getEntryPoint() const;

    u64 getExecutableSize() const { return size + bss_size; }

  private:
    filesystem::IFile* file;
    const bool is_entry_point;

    u64 size;
    NroSection sections[3];
    u64 bss_size;

    uptr executable_ptr{invalid<uptr>()};
    vaddr_t executable_base{invalid<vaddr_t>()};
    u64 executable_size{0};

    void tryLoadAssetSection(filesystem::IFile* file);

  public:
    GETTER(size, getSize);
    GETTER(executable_ptr, getExecutablePtr);
    GETTER(executable_base, getExecutableBase);
};

} // namespace hydra::horizon::loader
