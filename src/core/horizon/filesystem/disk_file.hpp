#pragma once

#include "core/horizon/filesystem/file.hpp"

#define LOG_FS_ACCESS(host_path, f, ...)                                       \
    if (CONFIG_INSTANCE.GetLogFsAccess()) {                                    \
        LOG_INFO(Filesystem, "\"{}\": " f,                                     \
                 host_path PASS_VA_ARGS(__VA_ARGS__));                         \
    }

namespace hydra::horizon::filesystem {

class DiskStream : public io::IostreamStream {
  public:
    // Fucking C++ STL doesn't allow creating std::fstream with std::string_view
    // as a path
    DiskStream(const std::string_view path, std::ios::openmode flags)
        : io::IostreamStream(stream), stream(std::string(path), flags) {
        LOG_FS_ACCESS(path, "file opened (flags: {})", flags);
    }
    ~DiskStream() override {
        stream.close();

        // TODO
        // LOG_FS_ACCESS(path, "file closed");
    }

  private:
    std::fstream stream;
};

class DiskFile : public IFile {
  public:
    enum class InitError {
        ImmutableFileDoesNotExist,
    };

    DiskFile(const std::string_view path_, bool is_mutable_ = false);
    ~DiskFile() override;

    void Resize(usize new_size) override;
    void Flush() override;

    io::IStream* Open(FileOpenFlags flags) override;

    // Getters
    usize GetSize() override;

  private:
    std::string path;
    bool is_mutable;

    // usize size;

    void DeleteImpl() override;
};

} // namespace hydra::horizon::filesystem
