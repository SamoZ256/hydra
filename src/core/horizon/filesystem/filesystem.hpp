#pragma once

#include "core/horizon/filesystem/device.hpp"

#define FILESYSTEM_INSTANCE horizon::filesystem::Filesystem::GetInstance()

namespace hydra::horizon::filesystem {

class FileBase;

class Filesystem {
  public:
    static Filesystem& GetInstance();

    Filesystem();
    ~Filesystem();

    void Mount(const std::string_view mount);
    void Mount(const std::string_view mount, const std::string_view root_path);

    [[nodiscard]] FsResult AddEntry(const std::string_view path,
                                    EntryBase* entry,
                                    bool add_intermediate = false);
    [[nodiscard]] FsResult AddEntry(const std::string_view path,
                                    const std::string_view host_path,
                                    bool add_intermediate = false);
    [[nodiscard]] FsResult CreateFile(const std::string_view path,
                                      usize size = invalid<usize>(),
                                      bool add_intermediate = false);
    [[nodiscard]] FsResult CreateDirectory(const std::string_view path,
                                           bool add_intermediate = false);

    [[nodiscard]] FsResult DeleteEntry(const std::string_view path,
                                       bool recursive = false);

    [[nodiscard]] FsResult GetEntry(const std::string_view path,
                                    EntryBase*& out_entry);
    [[nodiscard]] FsResult GetFile(const std::string_view path,
                                   FileBase*& out_file);
    [[nodiscard]] FsResult GetDirectory(const std::string_view path,
                                        Directory*& out_directory);

  private:
    std::map<std::string, Device> devices;

    void MountImpl(const std::string_view mount, Directory* root);
};

} // namespace hydra::horizon::filesystem
