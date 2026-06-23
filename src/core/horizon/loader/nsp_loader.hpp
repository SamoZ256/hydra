#pragma once

#include "core/horizon/filesystem/partition_filesystem.hpp"
#include "core/horizon/loader/nso_loader.hpp"

namespace hydra::horizon::loader {

// HACK: assumes Homebrew NSP
class NspLoader : public ILoader {
  public:
    NspLoader(filesystem::IFile* file)
        : NspLoader(
              *filesystem::PartitionFilesystem().Initialize<false>(file)) {}
    NspLoader(filesystem::PartitionFilesystem pfs_);

    u64 GetTitleID() const override { return invalid<u64>(); }

    void LoadProcess(System& system, kernel::Process* process) override {
        nso_loader->LoadProcess(system, process);
    }

  private:
    filesystem::PartitionFilesystem pfs;

    NsoLoader* nso_loader{nullptr};
};

} // namespace hydra::horizon::loader
