#pragma once

#include "core/horizon/filesystem/const.hpp"

namespace hydra::horizon::filesystem {

class Directory;

class IEntry {
  public:
    virtual ~IEntry() = default;

    virtual bool isFile() const { return false; }
    virtual bool isDirectory() const { return false; }

    virtual void save(std::string_view host_path) const = 0;

    virtual FsResult deleteEntry(bool recursive = false) = 0;

    Directory* getParent() const { return parent; }
    void setParent(Directory* parent_) { parent = parent_; }

  protected:
    Directory* parent;
};

} // namespace hydra::horizon::filesystem
