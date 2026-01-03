#pragma once

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/loader/loader_base.hpp"

namespace hydra::horizon::loader {

class NxLoader : public LoaderBase {
  public:
    enum class Error {
        InvalidNpdmMagic,
    };

    NxLoader(const filesystem::Directory& dir_);

    u64 GetTitleID() const override { return title_id; }

    const std::string& GetName() const { return title_name; }

    void LoadProcess(kernel::Process* process) override;

  private:
    const filesystem::Directory& dir;

    u64 title_id;
    std::string title_name;

    u8 main_thread_priority;
    u8 main_thread_core_number;
    u32 main_thread_stack_size;
    u32 system_resource_size;

    // Helpers
    void ParseInfo();
    void ParseNpdm();
    void LoadCode(kernel::Process* process, filesystem::Directory* dir);
};

} // namespace hydra::horizon::loader
