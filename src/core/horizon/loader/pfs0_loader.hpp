#pragma once

#include "core/horizon/loader/loader_base.hpp"

namespace hydra::horizon::loader {

struct PartitionEntry {
    u64 offset;
    u64 size;
    u32 string_offset;
    u32 reserved;
};

class Pfs0Loader : public LoaderBase {
  public:
    Pfs0Loader(StreamReader reader);

    kernel::Process* LoadProcess(StreamReader reader,
                                 const std::string_view rom_filename) override;

  private:
    std::vector<PartitionEntry> entries;
    std::string string_table;
    u32 entries_offset;
};

} // namespace hydra::horizon::loader
