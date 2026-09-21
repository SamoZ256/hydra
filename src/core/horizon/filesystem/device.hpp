#pragma once

#include "core/horizon/filesystem/directory.hpp"

namespace hydra::horizon::filesystem {

class Directory;

class Device {
  public:
    explicit Device(Directory* root_) : root{root_} {}
    ~Device();

    [[nodiscard]] FsResult addEntry(const std::string_view path, IEntry* entry,
                                    bool add_intermediate = false);
    [[nodiscard]] FsResult addEntry(const std::string_view path,
                                    const std::string_view host_path,
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
    Directory* root;
};

} // namespace hydra::horizon::filesystem
