#pragma once

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/file.hpp"
#include "core/horizon/filesystem/romfs/const.hpp"

namespace hydra::horizon::filesystem::romfs {

class Parser {
  public:
    Parser(ztd::io::IStream* stream, IFile* data_file_,
           const TableLocation& file_meta_loc,
           const TableLocation& directory_meta_loc);

    void LoadFile(Directory* parent, u32 offset) const;
    void LoadDirectory(Directory* parent, u32 offset) const;

  private:
    IFile* data_file;
    std::vector<u8> file_meta;
    std::vector<u8> directory_meta;

    // Helpers
    template <typename T, auto meta_member>
    std::pair<T, std::string> LoadEntry(u32 offset) const {
        // Entry
        const auto& meta = this->*meta_member;
        T entry;
        memcpy(&entry, &meta[offset], sizeof(T));

        // Name
        std::string name(
            reinterpret_cast<const char*>(&meta[offset + sizeof(T)]),
            entry.name_length);
        if (name.empty())
            name = EMPTY_PLACEHOLDER_NAME;

        return {entry, name};
    }
};

} // namespace hydra::horizon::filesystem::romfs
