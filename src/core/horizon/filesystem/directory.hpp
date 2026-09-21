#pragma once

#include "core/horizon/filesystem/entry.hpp"

namespace hydra::horizon::filesystem {

class IFile;

class Directory : public IEntry {
  public:
    Directory() = default;
    explicit Directory(const std::string_view host_path);
    ~Directory() override;

    bool isDirectory() const override { return true; }

    void save(std::string_view host_path) const override;

    // TODO: use exceptions
    [[nodiscard]] FsResult deleteEntry(bool recursive = false) override;

    [[nodiscard]] FsResult addEntry(const std::string_view path, IEntry* entry,
                                    bool add_intermediate = false);
    [[nodiscard]] FsResult addEntry(const std::string_view path,
                                    const std::string_view host_path,
                                    bool add_intermediate = false);

    [[nodiscard]] FsResult deleteEntry(const std::string_view path,
                                       bool recursive = false);

    [[nodiscard]] FsResult getEntry(const std::string_view path,
                                    IEntry*& out_entry) const;
    [[nodiscard]] FsResult getFile(const std::string_view path,
                                   IFile*& out_file) const;
    [[nodiscard]] FsResult getDirectory(const std::string_view path,
                                        Directory*& out_directory) const;

  protected:
    std::map<std::string, IEntry*> entries;

  public:
    CONST_REF_GETTER(entries, getEntries);

  private:
    // Impl
    FsResult addEntryImpl(const std::span<std::string_view> path, IEntry* entry,
                          bool add_intermediate = false);
    FsResult deleteEntryImpl(const std::span<std::string_view> path,
                             bool recursive = false);
    FsResult getEntryImpl(const std::span<std::string_view> path,
                          IEntry*& out_entry) const;

    // Helpers
    static void breakPath(std::string_view path,
                          std::vector<std::string_view>& out_path);
};

} // namespace hydra::horizon::filesystem
