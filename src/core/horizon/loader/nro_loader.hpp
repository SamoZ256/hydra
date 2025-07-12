#pragma once

#include "core/horizon/loader/loader_base.hpp"

namespace hydra::horizon::loader {

class NroLoader : public LoaderBase {
  public:
    NroLoader(filesystem::FileBase* file_);

    void LoadProcess(kernel::Process* process) override;

  private:
    filesystem::FileBase* file;
    u32 text_offset;
    usize bss_size;

    void TryLoadAssetSection(filesystem::FileBase* file);
};

} // namespace hydra::horizon::loader
