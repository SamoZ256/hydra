#pragma once

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/sparse_file.hpp"

namespace hydra::horizon::filesystem::romfs {

class RomFS final : public Directory {
  public:
    RomFS(IFile* file);
    RomFS(const Directory& dir);
    ~RomFS() override {}

    SparseFile* Build();
};

} // namespace hydra::horizon::filesystem::romfs
