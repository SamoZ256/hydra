#pragma once

#include "core/horizon/loader/nro_loader.hpp"

namespace hydra {
class System;
}

namespace hydra::horizon::loader {

class HomebrewLoader : public LoaderBase {
  public:
    HomebrewLoader(filesystem::IFile* file_);

    void LoadProcess(System& system, kernel::Process* process) override;

  private:
    filesystem::IFile* file;

    NroLoader nro_loader;

    void TryLoadAssetSection(filesystem::IFile* asset_file);
};

} // namespace hydra::horizon::loader
