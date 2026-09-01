#pragma once

#include <sys/stat.h>

#include "ztd/io/stream.hpp"

namespace ztd::io {

class FileStream : public IStream {
  public:
    FileStream(const ztd::fs::File& file_) noexcept : file{file_} {}

    [[nodiscard]] auto getSeek() const noexcept -> u64 override {
        return static_cast<u64>(lseek(file.getHandle(), 0, SEEK_CUR));
    }
    auto seekTo(u64 seek) noexcept -> void override {
        lseek(file.getHandle(), static_cast<off_t>(seek), SEEK_SET);
    }
    auto seekBy(u64 offset) noexcept -> void override {
        lseek(file.getHandle(), static_cast<off_t>(offset), SEEK_CUR);
    }

    [[nodiscard]] auto getSize() const noexcept -> u64 override {
        struct stat st{};
        if (fstat(file.getHandle(), &st) == -1) {
            // TODO: error
            unreachable();
        } else {
            return static_cast<u64>(st.st_size);
        }
    }

    auto flush() noexcept -> void override {
        if (fsync(file.getHandle()) == -1) {
            // TODO: error
            unreachable();
        }
    }

    auto readRaw(std::span<u8> buffer) noexcept -> void override {
        // TODO: return size
        (void)::read(file.getHandle(), buffer.data(), buffer.size());
    }

    auto writeRaw(std::span<const u8> buffer) noexcept -> void override {
        // TODO: return size
        (void)::write(file.getHandle(), buffer.data(), buffer.size());
    }

  private:
    const ztd::fs::File& file;
};

} // namespace ztd::io
