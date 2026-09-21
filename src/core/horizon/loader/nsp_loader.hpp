#pragma once

#include "core/horizon/filesystem/partition_filesystem.hpp"
#include "core/horizon/loader/nso_loader.hpp"

namespace hydra::horizon::loader {

// HACK: assumes Homebrew NSP
class NspLoader : public ILoader {
  public:
    explicit NspLoader(filesystem::IFile* file)
        : NspLoader(
              *filesystem::PartitionFilesystem().initialize<false>(file)) {}
    explicit NspLoader(filesystem::PartitionFilesystem pfs_);

    u64 getTitleId() const override { return invalid<u64>(); }

    void loadProcess(System& system, kernel::Process* process) override {
        nso_loader->loadProcess(system, process);
    }

  private:
    filesystem::PartitionFilesystem pfs;

    NsoLoader* nso_loader{nullptr};
};

} // namespace hydra::horizon::loader
