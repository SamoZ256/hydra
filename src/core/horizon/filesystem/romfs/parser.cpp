#include "core/horizon/filesystem/romfs/parser.hpp"

#include "core/horizon/filesystem/file_view.hpp"

namespace hydra::horizon::filesystem::romfs {

Parser::Parser(ztd::io::IStream* stream, IFile* data_file_,
               const TableLocation& file_meta_loc,
               const TableLocation& directory_meta_loc)
    : data_file{data_file_} {
    // File meta
    file_meta.resize(file_meta_loc.size);
    stream->seekTo(file_meta_loc.offset);
    stream->readToSpan(std::span(file_meta));

    // Directory meta
    directory_meta.resize(directory_meta_loc.size);
    stream->seekTo(directory_meta_loc.offset);
    stream->readToSpan(std::span(directory_meta));
}

void Parser::LoadFile(Directory* parent, u32 offset) const {
    while (offset != ENTRY_EMPTY) {
        auto [entry, name] = LoadEntry<FileEntry, &Parser::file_meta>(offset);

        const auto res = parent->AddEntry(
            name, new FileView(data_file, entry.offset, entry.size));
        ASSERT_DEBUG(res == FsResult::Success, Filesystem,
                     "Failed to add file");
        offset = entry.sibling;
    }
}

void Parser::LoadDirectory(Directory* parent, u32 offset) const {
    while (offset != ENTRY_EMPTY) {
        auto [entry, name] =
            LoadEntry<DirectoryEntry, &Parser::directory_meta>(offset);

        auto dir = new Directory();
        if (entry.child_file != ENTRY_EMPTY)
            LoadFile(dir, entry.child_file);
        if (entry.child_dir != ENTRY_EMPTY)
            LoadDirectory(dir, entry.child_dir);

        const auto res = parent->AddEntry(name, dir);
        ASSERT_DEBUG(res == FsResult::Success, Filesystem,
                     "Failed to add directory");
        offset = entry.sibling;
    }
}

} // namespace hydra::horizon::filesystem::romfs
