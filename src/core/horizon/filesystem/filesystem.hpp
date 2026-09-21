#pragma once

#include "core/horizon/filesystem/device.hpp"

namespace hydra::horizon::filesystem {

class IFile;

class Filesystem {
  public:
    Filesystem();

    void mount(const std::string_view mount);
    void mount(const std::string_view mount, const std::string_view root_path);

    [[nodiscard]] FsResult addEntry(const std::string_view path, IEntry* entry,
                                    bool add_intermediate = false);
    [[nodiscard]] FsResult addEntry(const std::string_view path,
                                    const std::string_view host_path,
                                    bool add_intermediate = false);
    [[nodiscard]] FsResult createFile(const std::string_view path,
                                      u64 size = invalid<u64>(),
                                      bool add_intermediate = false);
    [[nodiscard]] FsResult createDirectory(const std::string_view path,
                                           bool add_intermediate = false);

    [[nodiscard]] FsResult deleteEntry(const std::string_view path,
                                       bool recursive = false);

    [[nodiscard]] FsResult getEntry(const std::string_view path,
                                    IEntry*& out_entry);
    [[nodiscard]] FsResult getFile(const std::string_view path,
                                   IFile*& out_file);
    [[nodiscard]] FsResult getDirectory(const std::string_view path,
                                        Directory*& out_directory);

  private:
    std::map<std::string, Device> devices;

    void mountImpl(const std::string_view mount, Directory* root);
    void installFirmware();
};

} // namespace hydra::horizon::filesystem
