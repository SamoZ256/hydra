#include "core/horizon/services/fssrv/directory.hpp"

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/file_base.hpp"

namespace hydra::horizon::services::fssrv {

namespace {

struct FsDirectoryEntry {
    char name[MAX_PATH_SIZE];
    u8 pad[3];
    EntryType type : 8;
    u8 pad2[3];
    i64 file_size;
};

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(IDirectory, 0, Read, 1, GetEntryCount)

result_t IDirectory::Read(u64* out_entry_count,
                          OutBuffer<BufferAttr::MapAlias> out_entries) {
    if (entry_index >= directory->GetEntries().size()) {
        *out_entry_count = 0;
        return RESULT_SUCCESS;
    }

    // TODO: respect filter flags
    u32 i = 0;
    for (const auto& [path, entry] : directory->GetEntries()) {
        // Check if the writer has enough space to write the entry
        if (out_entries.writer->GetWrittenSize() + sizeof(FsDirectoryEntry) >
            out_entries.writer->GetSize())
            break;

        // TODO: find a better way to index
        if (i < entry_index)
            continue;

        if (!entry)
            continue;

        FsDirectoryEntry e{};
        memcpy(e.name, path.c_str(), path.size());
        e.type =
            (entry->IsDirectory() ? EntryType::Directory : EntryType::File);
        if (!entry->IsDirectory())
            e.file_size = static_cast<filesystem::FileBase*>(entry)->GetSize();
        else
            e.file_size = 0;

        out_entries.writer->Write(e);

        entry_index++;
        i++;
    }

    *out_entry_count =
        out_entries.writer->GetWrittenSize() / sizeof(FsDirectoryEntry);

    return RESULT_SUCCESS;
}

result_t IDirectory::GetEntryCount(u64* out_count) {
    *out_count = directory->GetEntries().size();
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::fssrv
