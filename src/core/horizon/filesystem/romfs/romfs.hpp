#pragma once

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/sparse_file.hpp"

namespace hydra::horizon::filesystem::romfs {

class RomFS final : public Directory {
  public:
    explicit RomFS(IFile* file);
    explicit RomFS(const Directory& dir);
    ~RomFS() override = default;

    SparseFile* build();
};

} // namespace hydra::horizon::filesystem::romfs
