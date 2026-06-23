#include "core/horizon/loader/nsp_loader.hpp"

namespace hydra::horizon::loader {

NspLoader::NspLoader(filesystem::PartitionFilesystem pfs_)
    : pfs(std::move(pfs_)) {
    filesystem::IFile* main_file;
    const auto res = pfs.GetFile("main", main_file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to get main file");
        return;
    }

    nso_loader = new NsoLoader(main_file);
}

} // namespace hydra::horizon::loader
