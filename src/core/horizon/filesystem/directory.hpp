#pragma once

#include "core/horizon/filesystem/entry_base.hpp"

namespace hydra::horizon::filesystem {

class Directory : public EntryBase {
  public:
    Directory() = default;
    Directory(const std::string_view host_path);
    ~Directory() override;

    bool IsDirectory() const override { return true; }

    FsResult Delete(bool recursive = false) override;

    FsResult AddEntry(const std::string_view path, EntryBase* entry,
                      bool add_intermediate = false);
    FsResult AddEntry(const std::string_view path,
                      const std::string_view host_path,
                      bool add_intermediate = false);
    FsResult DeleteEntry(const std::string_view path, bool recursive = false);
    FsResult GetEntry(const std::string_view path, EntryBase*& out_entry);

    // Getters
    const std::map<std::string, EntryBase*>& GetEntries() const {
        return entries;
    }

  private:
    std::map<std::string, EntryBase*> entries;

    // Impl
    FsResult AddEntryImpl(const std::span<std::string_view> path,
                          EntryBase* entry, bool add_intermediate = false);
    FsResult DeleteEntryImpl(const std::span<std::string_view> path,
                             bool recursive = false);
    FsResult GetEntryImpl(const std::span<std::string_view> path,
                          EntryBase*& out_entry);

    // Helpers
    void BreakPath(std::string_view path,
                   std::vector<std::string_view>& out_path);
};

} // namespace hydra::horizon::filesystem
