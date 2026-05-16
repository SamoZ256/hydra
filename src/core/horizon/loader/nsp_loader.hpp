#pragma once

#include "core/horizon/filesystem/partition_filesystem.hpp"
// #include "core/horizon/loader/nca_loader.hpp"
#include "core/horizon/loader/nso_loader.hpp"

namespace hydra::horizon::loader {

class NspLoader : public LoaderBase {
  public:
    NspLoader(filesystem::IFile* file)
        : NspLoader(std::move(
              *filesystem::PartitionFilesystem().Initialize<false>(file))) {}
    NspLoader(const filesystem::PartitionFilesystem& pfs_);

    u64 GetTitleID() const override {
        return invalid<u64>(); // program_nca_loader->GetTitleID();
    }

    void LoadProcess(System& system, kernel::Process* process) override {
        /*program_nca_loader*/ nso_loader->LoadProcess(system, process);
    }

  private:
    filesystem::PartitionFilesystem pfs;

    NsoLoader* nso_loader{nullptr};
    // NcaLoader* program_nca_loader{nullptr};
};

} // namespace hydra::horizon::loader
