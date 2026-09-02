#pragma once

#include "core/horizon/filesystem/file.hpp"

#define LOG_FS_ACCESS(host_path, f, ...)                                       \
    if (CONFIG_INSTANCE.GetLogFsAccess()) {                                    \
        LOG_INFO(Filesystem, "\"{}\": " f,                                     \
                 host_path ZTD_PASS_VA_ARGS(__VA_ARGS__));                     \
    }

namespace hydra::horizon::filesystem {

class DiskStream : public ztd::io::FileStream {
  public:
    // HACK: FileStream takes a reference to File, so its okay to initialize
    // file after calling the base constructor
    DiskStream(const std::string_view path,
               ztd::fs::File::OpenFlags flags) noexcept
        : ztd::io::FileStream(file) {
        // HACK: construct a temporary string
        ZTD_ASSIGN_OR(
            file, ztd::fs::openFileAbsolute(std::string(path), flags),
            { LOG_FATAL(Filesystem, "Failed to open file at path {}", path); });

        LOG_FS_ACCESS(path, "file opened (flags: {})", flags);
    }

  private:
    ztd::fs::File file;
};

class DiskFile : public IFile {
  public:
    DiskFile(const std::string_view path_, bool is_mutable_ = false);
    ~DiskFile() override;

    void Resize(u64 new_size) override;
    void Flush() override;

    ztd::io::IStream* Open(FileOpenFlags flags) override;

    u64 GetSize() const override;

  private:
    std::string path;
    bool is_mutable;

    // u64 size;

    void DeleteImpl() override;
};

} // namespace hydra::horizon::filesystem
