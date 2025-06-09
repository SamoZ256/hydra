#pragma once

#include "core/horizon/loader/loader_base.hpp"

namespace hydra::horizon::loader {

struct FileEntry {
    u64 offset;
    u64 size;
    u32 string_offset;
    u32 hashed_region_size;
    u64 _reserved_x18;
    u8 hash[0x20];
} PACKED;

class XciLoader : public LoaderBase {
  public:
    XciLoader(StreamReader reader);

    kernel::Process* LoadProcess(StreamReader reader,
                                 const std::string_view rom_filename) override;

  private:
    std::vector<FileEntry> entries;
    std::string string_table;
    u32 entries_offset;

    void LoadNormalArea(StreamReader reader);
    void LoadRomArea(StreamReader reader);
};

} // namespace hydra::horizon::loader
