#pragma once

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/file.hpp"
#include "core/horizon/filesystem/file_view.hpp"

namespace hydra::horizon::filesystem {

namespace {

struct PfsEntry {
    u64 offset;
    u64 size;
    u32 string_offset;
    u32 _reserved_xc;
};

struct HfsEntry {
    u64 offset;
    u64 size;
    u32 string_offset;
    u32 hashed_region_size;
    u64 _reserved_x18;
    u8 hash[0x20];
};

struct PfsHeader {
    u32 magic;
    u32 entry_count;
    u32 string_table_size;
    u32 _reserved_xc;
};

} // namespace

class PartitionFilesystem final : public Directory {
  public:
    enum class Error {
        InvalidMagic,
    };

    // HACK: need to use a method instead of a constructor, since we have a
    // template parameter
    template <bool is_hfs>
    PartitionFilesystem* Initialize(IFile* file) {
        auto stream = file->Open(FileOpenFlags::Read);

        // Header
        const auto header = stream->Read<PfsHeader>();
        if (!is_hfs) {
            ASSERT_THROWING(header.magic == make_magic4('P', 'F', 'S', '0'),
                            Filesystem, Error::InvalidMagic,
                            "Invalid PFS0 magic 0x{:08x}", header.magic);
        } else {
            ASSERT_THROWING(header.magic == make_magic4('H', 'F', 'S', '0'),
                            Filesystem, Error::InvalidMagic,
                            "Invalid HFS0 magic 0x{:08x}", header.magic);
        }

        using EntryType = std::conditional_t<is_hfs, HfsEntry, PfsEntry>;

        const u64 entries_offset = sizeof(PfsHeader);
        const u64 string_table_offset =
            entries_offset + sizeof(EntryType) * header.entry_count;
        const u64 data_offset = string_table_offset + header.string_table_size;

        // String table
        stream->SeekTo(string_table_offset);
        std::string string_table;
        string_table.resize(header.string_table_size);
        stream->ReadToSpan(std::span(string_table));

        // Entries
        stream->SeekTo(entries_offset);
        for (u32 i = 0; i < header.entry_count; i++) {
            const auto entry = stream->Read<EntryType>();

            const std::string entry_name(string_table.data() +
                                         entry.string_offset);
            LOG_DEBUG(Filesystem, "Partition entry: {}", entry_name);

            const u64 entry_data_offset = data_offset + entry.offset;
            entries.insert({std::string(entry_name),
                            new FileView(file, entry_data_offset, entry.size)});
        }

        delete stream;

        return this;
    }
};

} // namespace hydra::horizon::filesystem
