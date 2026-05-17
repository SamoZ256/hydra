#include "core/horizon/loader/nsp_loader.hpp"

namespace hydra::horizon::loader {

NspLoader::NspLoader(const filesystem::PartitionFilesystem& pfs_) : pfs(pfs_) {
    /*
    // TODO: ticket file

    // NCAs
    // HACK: find the largest one
    struct {
        filesystem::IFile* file;
        u64 size;
    } largest_entry{nullptr, 0};
    for (const auto& [filename, entry] : pfs.GetEntries()) {
        auto file = dynamic_cast<filesystem::IFile*>(entry);
        if (!file)
            continue;

        if (file->GetSize() > largest_entry.size)
            largest_entry = {file, file->GetSize()};
    }

    // TODO: check for encryption
    program_nca_loader = new NcaLoader(largest_entry.file);
    */

    filesystem::IFile* main_file;
    const auto res = pfs.GetFile("main", main_file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to get main file");
        return;
    }

    nso_loader = new NsoLoader(main_file);
}

} // namespace hydra::horizon::loader
