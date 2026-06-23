#pragma once

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/romfs/const.hpp"
#include "core/horizon/filesystem/sparse_file.hpp"

namespace hydra::horizon::filesystem::romfs {

class Builder {
  public:
    explicit Builder(Directory* root);

    std::vector<SparseFileEntry> Build();

  private:
    struct DirContext;
    struct FileContext;

    Directory* root_dir;

    std::vector<std::shared_ptr<DirContext>> directories;
    std::vector<std::shared_ptr<FileContext>> files;

    u64 data_size = 0;
    u64 dir_table_size = 0;
    u64 file_table_size = 0;
    u64 dir_hash_table_size = 0;
    u64 file_hash_table_size = 0;

    void VisitDirectory(Directory* dir, const std::shared_ptr<DirContext>& parent);

    void AddDirectory(std::shared_ptr<DirContext> ctx);
    void AddFile(std::shared_ptr<FileContext> ctx);

    static u32 CalcPathHash(u32 parent, const std::string& path, u32 start,
                            u32 len);

    static u64 CalcHashTableSize(u64 entries);
};

} // namespace hydra::horizon::filesystem::romfs
