#pragma once

#include "core/horizon/filesystem/content_archive.hpp"
#include "core/horizon/loader/loader.hpp"

namespace hydra {
class System;
}

namespace hydra::horizon::loader {

class NcaLoader : public ILoader {
  public:
    explicit NcaLoader(filesystem::IFile* file)
        : NcaLoader(filesystem::ContentArchive(file)) {}
    explicit NcaLoader(filesystem::ContentArchive content_archive_);

    u64 getTitleId() const override { return content_archive.getTitleId(); }

    const std::string& getName() const { return name; }

    void loadProcess(System& system, kernel::Process* process) override;

  private:
    filesystem::ContentArchive content_archive;

    std::string name;
    u8 main_thread_priority;
    u8 main_thread_core_number;
    u32 main_thread_stack_size;
    u32 system_resource_size;

    // Helpers
    void loadCode(System& system, kernel::Process* process,
                  filesystem::Directory* dir) const;
};

} // namespace hydra::horizon::loader
