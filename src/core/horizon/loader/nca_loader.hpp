#pragma once

#include "core/horizon/filesystem/content_archive.hpp"
#include "core/horizon/loader/loader.hpp"

namespace hydra {
class System;
}

namespace hydra::horizon::loader {

class NcaLoader : public ILoader {
  public:
    NcaLoader(filesystem::IFile* file)
        : NcaLoader(filesystem::ContentArchive(file)) {}
    NcaLoader(filesystem::ContentArchive content_archive_);

    u64 GetTitleID() const override { return content_archive.GetTitleID(); }

    const std::string& GetName() const { return name; }

    void LoadProcess(System& system, kernel::Process* process) override;

  private:
    filesystem::ContentArchive content_archive;

    std::string name;
    u8 main_thread_priority;
    u8 main_thread_core_number;
    u32 main_thread_stack_size;
    u32 system_resource_size;

    // Helpers
    void LoadCode(System& system, kernel::Process* process,
                  filesystem::Directory* dir) const;
};

} // namespace hydra::horizon::loader
