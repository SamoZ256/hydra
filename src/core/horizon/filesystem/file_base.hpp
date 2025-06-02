#pragma once

#include "core/horizon/filesystem/entry_base.hpp"

namespace hydra::horizon::filesystem {

class FileStream {
  public:
    FileStream(std::iostream* stream_, u32 offset_, usize size_,
               FileOpenFlags flags_)
        : stream{stream_}, offset{offset_}, size{size_}, flags{flags_} {}
    virtual ~FileStream() = default;

    StreamReader CreateReader() {
        ASSERT(any(flags & FileOpenFlags::Read), Filesystem,
               "No read permission");
        return StreamReader(*stream, offset, size);
    }

    StreamWriter CreateWriter() {
        ASSERT(any(flags & FileOpenFlags::Write), Filesystem,
               "No write permission");
        return StreamWriter(*stream, offset, size);
    }

    // Getters
    std::iostream* GetStream() const { return stream; }

  protected:
    std::iostream* stream;
    u32 offset;
    usize size;

    FileOpenFlags flags;
};

class FileBase : public EntryBase {
  public:
    FileBase(u32 offset_) : offset{offset_} {}

    bool IsDirectory() const override { return false; }

    FsResult Delete(bool recursive = false) override {
        ASSERT(!recursive, Filesystem, "Cannot recursively delete file");
        DeleteImpl();
        return FsResult::Success;
    }

    virtual void Resize(usize new_size) = 0;

    virtual FileStream Open(FileOpenFlags flags) = 0;
    virtual void Close(FileStream& stream) = 0;

    // Getters
    virtual usize GetSize() = 0;

  protected:
    u32 offset;

    virtual void DeleteImpl() = 0;
};

} // namespace hydra::horizon::filesystem
