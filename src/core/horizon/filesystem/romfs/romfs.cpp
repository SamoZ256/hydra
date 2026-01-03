#include "core/horizon/filesystem/romfs/romfs.hpp"

#include "core/horizon/filesystem/file_view.hpp"
#include "core/horizon/filesystem/romfs/builder.hpp"
#include "core/horizon/filesystem/romfs/const.hpp"
#include "core/horizon/filesystem/romfs/parser.hpp"

namespace hydra::horizon::filesystem::romfs {

RomFS::RomFS(IFile* file) {
    auto stream = file->Open(FileOpenFlags::Read);

    // Header
    const auto header = stream->Read<Header>();
    ASSERT_THROWING(header.header_size == sizeof(Header), Filesystem,
                    Error::InvalidHeaderSize,
                    "Invalid romFS header size 0x{:x}", header.header_size);

    // Content
    Parser parser(stream, new FileView(file, header.data_offset),
                  header.file_meta, header.directory_meta);
    auto root_container = new Directory();
    parser.LoadDirectory(root_container, 0);

    delete stream;

    // Get root
    IEntry* root;
    const auto res = root_container->GetEntry(EMPTY_PLACEHOLDER_NAME, root);
    ASSERT(res == FsResult::Success, Filesystem,
           "Failed to get root romFS directory: {}", res);

    auto root_dir = dynamic_cast<Directory*>(root);
    ASSERT(root_dir != nullptr, Filesystem, "Root entry is not a directory");

    for (const auto& [name, entry] : root_dir->GetEntries()) {
        const auto res = AddEntry(name, entry);
        ASSERT_DEBUG(res == FsResult::Success, Filesystem,
                     "Failed to add entry");
    }
}

RomFS::RomFS(const Directory& dir) {
    for (const auto& [name, entry] : dir.GetEntries()) {
        const auto res = AddEntry(name, entry);
        ASSERT_DEBUG(res == FsResult::Success, Filesystem,
                     "Failed to add entry");
    }
}

SparseFile* RomFS::Build() {
    Builder builder(this);
    auto chunks = builder.Build();

    // Build the sparse file
    u64 size = 0;
    for (const auto& chunk : chunks)
        size = std::max(size, chunk.offset + chunk.file->GetSize());
    SparseFile* file = new SparseFile(chunks, size);

    return file;
}

} // namespace hydra::horizon::filesystem::romfs
