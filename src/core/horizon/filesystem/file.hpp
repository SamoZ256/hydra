#pragma once

#include "core/horizon/filesystem/entry.hpp"

namespace hydra::horizon::filesystem {

class IFile : public IEntry {
  public:
    bool isFile() const override { return true; }

    FsResult deleteEntry(bool recursive = false) override {
        ASSERT(!recursive, Filesystem, "Cannot recursively delete file");
        deleteImpl();
        return FsResult::Success;
    }

    virtual void resize(u64 new_size) {
        LOG_FATAL(Filesystem, "File cannot be resized (size: 0x{:x})",
                  new_size);
    }
    virtual void flush() {}

    virtual ztd::io::IStream* open(FileOpenFlags flags) = 0;

    virtual u64 getSize() const = 0;

    void save(std::string_view host_path) const override;

  protected:
    virtual void deleteImpl() {
        LOG_FATAL(Filesystem, "File cannot be deleted");
    }
};

} // namespace hydra::horizon::filesystem
