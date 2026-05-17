#pragma once

#include "core/horizon/filesystem/file.hpp"

namespace hydra::horizon::filesystem {

class FileView : public IFile {
  public:
    enum class InitError {
        SizeTooLarge,
    };

    FileView(IFile* base_, u64 offset_, u64 size_ = invalid<u64>())
        : base{base_}, offset{offset_}, size{size_} {
        if (size == invalid<u64>())
            size = base->GetSize() - offset;
        else
            ASSERT_THROWING(size <= base->GetSize() - offset, Filesystem,
                            InitError::SizeTooLarge,
                            "File view size (0x{:08x}) is too large "
                            "(max size: 0x{:08x})",
                            size, base->GetSize() - offset);
    }

    io::IStream* Open(FileOpenFlags flags) override {
        return new io::OwnedStreamView(base->Open(flags), offset, size);
    }

    u64 GetSize() const override { return size; }

  private:
    IFile* base;
    u64 offset;
    u64 size;

  public:
    GETTER(base, GetBase);
    GETTER(offset, GetOffset);
};

} // namespace hydra::horizon::filesystem
