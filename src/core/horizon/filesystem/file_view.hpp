#pragma once

#include "core/horizon/filesystem/file.hpp"

namespace hydra::horizon::filesystem {

class OwnedStreamView : public ztd::io::StreamView {
  public:
    OwnedStreamView(ztd::io::IStream* base_, u64 offset, u64 size) noexcept
        : StreamView(base_, offset, size), base{base_} {}
    ~OwnedStreamView() noexcept override { delete base; }

    ZTD_MAKE_NON_COPYABLE(OwnedStreamView);

  private:
    ztd::io::IStream* base;
};

class FileView : public IFile {
  public:
    FileView(IFile* base_, u64 offset_, u64 size_ = invalid<u64>())
        : base{base_}, offset{offset_}, size{size_} {
        if (size == invalid<u64>()) {
            size = base->GetSize() - offset;
        } else {
            ASSERT(size <= base->GetSize() - offset, Filesystem,
                   "File view size (0x{:08x}) is too large "
                   "(max size: 0x{:08x})",
                   size, base->GetSize() - offset);
        }
    }

    ztd::io::IStream* Open(FileOpenFlags flags) override {
        return new OwnedStreamView(base->Open(flags), offset, size);
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
