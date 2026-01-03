#pragma once

#include "core/horizon/filesystem/entry.hpp"

namespace hydra::horizon::filesystem {

class IFile;

class Directory : public IEntry {
  public:
    enum class InitError {
        NotADirectory,
    };

    Directory() = default;
    Directory(const std::string_view host_path);
    ~Directory() override;

    bool IsDirectory() const override { return true; }

    // TODO: use exceptions
    [[nodiscard]] FsResult Delete(bool recursive = false) override;

    [[nodiscard]] FsResult AddEntry(const std::string_view path, IEntry* entry,
                                    bool add_intermediate = false);
    [[nodiscard]] FsResult AddEntry(const std::string_view path,
                                    const std::string_view host_path,
                                    bool add_intermediate = false);

    [[nodiscard]] FsResult DeleteEntry(const std::string_view path,
                                       bool recursive = false);

    [[nodiscard]] FsResult GetEntry(const std::string_view path,
                                    IEntry*& out_entry) const;
    [[nodiscard]] FsResult GetFile(const std::string_view path,
                                   IFile*& out_file) const;
    [[nodiscard]] FsResult GetDirectory(const std::string_view path,
                                        Directory*& out_directory) const;

  protected:
    std::map<std::string, IEntry*> entries;

  public:
    CONST_REF_GETTER(entries, GetEntries);

  private:
    // Impl
    FsResult AddEntryImpl(const std::span<std::string_view> path, IEntry* entry,
                          bool add_intermediate = false);
    FsResult DeleteEntryImpl(const std::span<std::string_view> path,
                             bool recursive = false);
    FsResult GetEntryImpl(const std::span<std::string_view> path,
                          IEntry*& out_entry) const;

    // Helpers
    void BreakPath(std::string_view path,
                   std::vector<std::string_view>& out_path) const;
};

} // namespace hydra::horizon::filesystem
