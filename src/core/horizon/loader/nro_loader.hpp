#pragma once

#include "core/horizon/loader/loader_base.hpp"

namespace hydra::horizon::loader {

class NroLoader : public LoaderBase {
  public:
    NroLoader(StreamReader reader);

    kernel::Process* LoadProcess(StreamReader reader,
                                 const std::string_view rom_filename) override;

  private:
    u32 text_offset;
    usize bss_size;
};

} // namespace hydra::horizon::loader
