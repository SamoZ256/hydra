#include "core/horizon/filesystem/romfs/builder.hpp"

#include <algorithm>
#include <ranges>

#include "core/horizon/filesystem/file_view.hpp"
#include "core/horizon/filesystem/memory_file.hpp"

namespace hydra::horizon::filesystem::romfs {

struct Builder::DirContext {
    std::string path;
    u32 name_offset = 0;
    u32 path_len = 0;
    u32 entry_offset = 0;

    std::shared_ptr<DirContext> parent;
    std::shared_ptr<DirContext> child;
    std::shared_ptr<DirContext> sibling;
    std::shared_ptr<FileContext> file;
};

struct Builder::FileContext {
    std::string path;
    u32 name_offset = 0;
    u32 path_len = 0;
    u32 entry_offset = 0;

    u64 offset = 0;
    u64 size = 0;

    std::shared_ptr<DirContext> parent;
    std::shared_ptr<FileContext> sibling;
    IFile* source = nullptr;
};

Builder::Builder(Directory* root) : root_dir(root) {}

u32 Builder::calcPathHash(u32 parent, const std::string& path, u32 start,
                          u32 len) {
    u32 hash = parent ^ 123456789;
    for (u32 i = 0; i < len; i++) {
        hash = std::rotr(hash, 5);
        hash ^= static_cast<u32>(path[start + i]);
    }
    return hash;
}

u64 Builder::calcHashTableSize(u64 entries) {
    if (entries < 3)
        return 3;

    if (entries < 19)
        return entries | 1;

    u64 count = entries;
    while (count % 2 == 0 || count % 3 == 0 || count % 5 == 0 ||
           count % 7 == 0 || count % 11 == 0 || count % 13 == 0 ||
           count % 17 == 0) {
        count++;
    }
    return count;
}

void Builder::visitDirectory(Directory* dir,
                             const std::shared_ptr<DirContext>& parent) {
    for (const auto& [name, entry] : dir->getEntries()) {
        if (entry->isDirectory()) {
            auto ctx = std::make_shared<DirContext>();
            ctx->parent = parent;
            ctx->name_offset = parent->path_len + 1;
            ctx->path = parent->path + "/" + name;
            ctx->path_len = static_cast<u32>(ctx->path.size());

            addDirectory(ctx);
            visitDirectory(static_cast<Directory*>(entry), ctx);
        } else {
            auto ctx = std::make_shared<FileContext>();
            ctx->parent = parent;
            ctx->name_offset = parent->path_len + 1;
            ctx->path = parent->path + "/" + name;
            ctx->path_len = static_cast<u32>(ctx->path.size());
            ctx->source = static_cast<IFile*>(entry);
            ctx->size = static_cast<IFile*>(entry)->getSize();

            addFile(ctx);
        }
    }
}

void Builder::addDirectory(std::shared_ptr<DirContext> ctx) {
    dir_table_size +=
        sizeof(DirectoryEntry) + align(ctx->path_len - ctx->name_offset, 4u);
    directories.emplace_back(std::move(ctx));
}

void Builder::addFile(std::shared_ptr<FileContext> ctx) {
    file_table_size +=
        sizeof(FileEntry) + align(ctx->path_len - ctx->name_offset, 4u);
    files.emplace_back(std::move(ctx));
}

std::vector<SparseFileEntry> Builder::build() {
    auto root = std::make_shared<DirContext>();
    root->path = "";
    dir_table_size = sizeof(DirectoryEntry);
    directories.emplace_back(root);

    visitDirectory(root_dir, root);

    std::ranges::sort(directories,
                      [](auto& a, auto& b) { return a->path < b->path; });
    std::ranges::sort(files,
                      [](auto& a, auto& b) { return a->path < b->path; });

    // Assign file offsets
    u32 entry_offset = 0;
    for (auto& file : files) {
        data_size = align(data_size, 16ull);
        file->offset = data_size;
        data_size += file->size;
        file->entry_offset = entry_offset;
        entry_offset +=
            sizeof(FileEntry) + align(file->path_len - file->name_offset, 4u);
    }
    // Assign deferred parent/sibling ownership
    for (auto& cur_file : std::views::reverse(files)) {
        cur_file->sibling = cur_file->parent->file;
        cur_file->parent->file = cur_file;
    }

    // Assign directory offsets
    entry_offset = 0;
    for (auto& dir : directories) {
        dir->entry_offset = entry_offset;
        entry_offset += sizeof(DirectoryEntry) +
                        align(dir->path_len - dir->name_offset, 4u);
    }
    // Assign deferred parent/sibling ownership.
    for (auto it = directories.rbegin(); (*it) != root; ++it) {
        auto& cur_dir = *it;
        cur_dir->sibling = cur_dir->parent->child;
        cur_dir->parent->child = cur_dir;
    }

    // Hash table sizes
    const u64 dir_hash_count = calcHashTableSize(directories.size());
    const u64 file_hash_count = calcHashTableSize(files.size());

    dir_hash_table_size = dir_hash_count * sizeof(u32);
    file_hash_table_size = file_hash_count * sizeof(u32);

    std::vector<u8> metadata(dir_hash_table_size + dir_table_size +
                             file_hash_table_size + file_table_size);

    std::memset(metadata.data(), 0xff, metadata.size());

    // Layout metadata views
    auto* dir_hash_table = reinterpret_cast<u32*>(metadata.data());
    auto* dir_table = metadata.data() + dir_hash_table_size;
    auto* file_hash_table = reinterpret_cast<u32*>(dir_table + dir_table_size);
    auto* file_table = reinterpret_cast<u8*>(
        file_hash_table + file_hash_table_size / sizeof(u32));

    // Header
    Header header{};
    header.header_size = sizeof(Header);
    header.data_offset = DATA_OFFSET;
    header.directory_hash = {.offset =
                                 align(header.data_offset + data_size, 4ull),
                             .size = dir_hash_table_size};
    header.directory_meta = {.offset = header.directory_hash.offset +
                                       header.directory_hash.size,
                             .size = dir_table_size};
    header.file_hash = {.offset = header.directory_meta.offset +
                                  header.directory_meta.size,
                        .size = file_hash_table_size};
    header.file_meta = {.offset =
                            header.file_hash.offset + header.file_hash.size,
                        .size = file_table_size};

    std::vector<SparseFileEntry> out;

    // Header output
    out.push_back({.offset = 0,
                   .file = new MemoryFile(std::vector(
                       reinterpret_cast<u8*>(&header),
                       reinterpret_cast<u8*>(&header) + sizeof(Header)))});

    // Populate file table
    for (const auto& file : files) {
        FileEntry entry{};

        entry.parent = file->parent->entry_offset;
        entry.sibling =
            file->sibling ? file->sibling->entry_offset : ENTRY_EMPTY;
        entry.offset = file->offset;
        entry.size = file->size;

        const u32 name_len = file->path_len - file->name_offset;
        const u32 hash = calcPathHash(file->parent->entry_offset, file->path,
                                      file->name_offset, name_len);

        const u32 bucket = hash % file_hash_count;
        entry.hash = file_hash_table[bucket];
        file_hash_table[bucket] = file->entry_offset;

        entry.name_length = name_len;

        std::memcpy(file_table + file->entry_offset, &entry, sizeof(FileEntry));
        std::memset(file_table + file->entry_offset + sizeof(FileEntry), 0,
                    align(name_len, 4u));
        std::memcpy(file_table + file->entry_offset + sizeof(FileEntry),
                    file->path.data() + file->name_offset, name_len);

        // Emit file data
        out.push_back(
            {.offset = DATA_OFFSET + file->offset, .file = file->source});
    }

    // Populate directory table
    for (const auto& dir : directories) {
        DirectoryEntry entry{};

        entry.parent = dir == root ? 0 : dir->parent->entry_offset;
        entry.sibling = dir->sibling ? dir->sibling->entry_offset : ENTRY_EMPTY;
        entry.child_dir = dir->child ? dir->child->entry_offset : ENTRY_EMPTY;
        entry.child_file = dir->file ? dir->file->entry_offset : ENTRY_EMPTY;

        const u32 name_len = dir->path_len - dir->name_offset;
        const u32 hash =
            calcPathHash(dir == root ? 0 : dir->parent->entry_offset, dir->path,
                         dir->name_offset, name_len);

        const u32 bucket = hash % dir_hash_count;
        entry.hash = dir_hash_table[bucket];
        dir_hash_table[bucket] = dir->entry_offset;

        entry.name_length = name_len;

        std::memcpy(dir_table + dir->entry_offset, &entry,
                    sizeof(DirectoryEntry));
        std::memset(dir_table + dir->entry_offset + sizeof(DirectoryEntry), 0,
                    align(name_len, 4u));
        std::memcpy(dir_table + dir->entry_offset + sizeof(DirectoryEntry),
                    dir->path.data() + dir->name_offset, name_len);
    }

    // Metadata output
    out.push_back({.offset = header.directory_hash.offset,
                   .file = new MemoryFile(std::move(metadata))});

    return out;
}

} // namespace hydra::horizon::filesystem::romfs
