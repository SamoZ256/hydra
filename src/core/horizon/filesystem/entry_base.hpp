#pragma once

#include "core/horizon/filesystem/const.hpp"

namespace hydra::horizon::filesystem {

class Directory;

class EntryBase {
  public:
    virtual ~EntryBase() = default;

    virtual bool IsDirectory() const = 0;

    virtual FsResult Delete(bool recursive = false) = 0;

    Directory* GetParent() const { return parent; }
    void SetParent(Directory* parent_) { parent = parent_; }

  protected:
    Directory* parent;
};

} // namespace hydra::horizon::filesystem
