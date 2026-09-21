#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IAudioController : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t setExpectedMasterVolume(f32 main_applet_volume,
                                     f32 library_applet_volume);
    result_t getMainAppletExpectedMasterVolume(f32* out_volume);
    result_t getLibraryAppletExpectedMasterVolume(f32* out_volume);
};

} // namespace hydra::horizon::services::am
