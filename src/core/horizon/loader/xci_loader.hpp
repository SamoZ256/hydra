#pragma once

#include "core/horizon/loader/nsp_loader.hpp"

namespace hydra::horizon::loader {

class XciLoader : public LoaderBase {
  public:
    XciLoader(filesystem::FileBase* file);

    u64 GetTitleID() const override { return nsp_loader->GetTitleID(); }

    void LoadProcess(kernel::Process* process) override {
        nsp_loader->LoadProcess(process);
    }

  private:
    NspLoader* nsp_loader{nullptr};
};

} // namespace hydra::horizon::loader
