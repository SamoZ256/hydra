#pragma once

#include "core/horizon/filesystem/partition_filesystem.hpp"
#include "core/horizon/loader/nca_loader.hpp"

namespace hydra::horizon::loader {

class NspLoader : public LoaderBase {
  public:
    NspLoader(filesystem::FileBase* file)
        : NspLoader(std::move(
              *filesystem::PartitionFilesystem().Initialize<false>(file))) {}
    NspLoader(const filesystem::PartitionFilesystem& pfs_);

    u64 GetTitleID() const override { return program_nca_loader->GetTitleID(); }

    void LoadProcess(kernel::Process* process) override {
        program_nca_loader->LoadProcess(process);
    }

  private:
    filesystem::PartitionFilesystem pfs;

    NcaLoader* program_nca_loader{nullptr};
};

} // namespace hydra::horizon::loader
