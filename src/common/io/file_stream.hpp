#pragma once

#include <sys/stat.h>

#include "common/io/stream.hpp"

namespace hydra::io {

class FileStream : public IStream {
  public:
    FileStream(const ztd::fs::File& file_) : file{file_} {}

    u64 GetSeek() const override {
        return static_cast<u64>(lseek(file.getHandle(), 0, SEEK_CUR));
    }
    void SeekTo(u64 seek) override {
        lseek(file.getHandle(), static_cast<off_t>(seek), SEEK_SET);
    }
    void SeekBy(u64 offset) override {
        lseek(file.getHandle(), static_cast<off_t>(offset), SEEK_CUR);
    }

    u64 GetSize() const override {
        struct stat st;
        if (fstat(file.getHandle(), &st) == -1) {
            // TODO: error
            unreachable();
        } else {
            return static_cast<u64>(st.st_size);
        }
    }

    void Flush() override {
        if (fsync(file.getHandle()) == -1) {
            // TODO: error
            unreachable();
        }
    }

    void ReadRaw(std::span<u8> buffer) override {
        // TODO: return size
        (void)read(file.getHandle(), buffer.data(), buffer.size());
    }

    void WriteRaw(std::span<const u8> buffer) override {
        // TODO: return size
        (void)write(file.getHandle(), buffer.data(), buffer.size());
    }

  private:
    const ztd::fs::File& file;
};

} // namespace hydra::io
