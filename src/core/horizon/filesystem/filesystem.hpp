#pragma once

#include "core/horizon/filesystem/device.hpp"

namespace hydra::horizon::filesystem {

class IFile;

class Filesystem {
  public:
    Filesystem();

    void Mount(const std::string_view mount);
    void Mount(const std::string_view mount, const std::string_view root_path);

    [[nodiscard]] FsResult AddEntry(const std::string_view path, IEntry* entry,
                                    bool add_intermediate = false);
    [[nodiscard]] FsResult AddEntry(const std::string_view path,
                                    const std::string_view host_path,
                                    bool add_intermediate = false);
    [[nodiscard]] FsResult CreateFile(const std::string_view path,
                                      u64 size = invalid<u64>(),
                                      bool add_intermediate = false);
    [[nodiscard]] FsResult CreateDirectory(const std::string_view path,
                                           bool add_intermediate = false);

    [[nodiscard]] FsResult DeleteEntry(const std::string_view path,
                                       bool recursive = false);

    [[nodiscard]] FsResult GetEntry(const std::string_view path,
                                    IEntry*& out_entry);
    [[nodiscard]] FsResult GetFile(const std::string_view path,
                                   IFile*& out_file);
    [[nodiscard]] FsResult GetDirectory(const std::string_view path,
                                        Directory*& out_directory);

  private:
    std::map<std::string, Device> devices;

    void MountImpl(const std::string_view mount, Directory* root);
};

} // namespace hydra::horizon::filesystem
