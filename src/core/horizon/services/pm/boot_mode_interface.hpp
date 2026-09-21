#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::pm {

enum class BootMode : u32 {
    Normal = 0,
    Maintenance = 1,
    SafeMode = 2,
};

class IBootModeInterface : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t getBootMode(BootMode* out_mode);
};

} // namespace hydra::horizon::services::pm
