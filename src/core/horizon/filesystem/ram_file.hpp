#pragma once

#include "core/horizon/filesystem/file_base.hpp"

namespace hydra::horizon::filesystem {

class RamFile : public FileBase {
  public:
    RamFile(usize size = invalid<usize>()) : FileBase(0) {
        if (size == invalid<usize>()) {
            size = 0;
        } else {
            LOG_NOT_IMPLEMENTED(Filesystem, "Size (0x{:08x})", size);
        }
    }
    ~RamFile() override = default;

    void Resize(usize new_size) override {
        LOG_NOT_IMPLEMENTED(Filesystem, "RAM file resizing (size: 0x{:x})",
                            new_size);
    }

    FileStream Open(FileOpenFlags flags) override {
        return FileStream(&stream, offset, GetSize(), flags);
    }

    void Close(FileStream& stream) override {
        // Do nothing
    }

    // Getters
    usize GetSize() override {
        stream.seekg(0, std::ios::end);
        return stream.tellg();
    }

  private:
    std::stringstream stream;

    void DeleteImpl() override {
        // Do nothing
    }
};

} // namespace hydra::horizon::filesystem
