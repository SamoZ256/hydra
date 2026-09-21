#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/i2c/const.hpp"

namespace hydra::horizon::services::i2c {

class IManager : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t openSession(RequestContext* ctx, I2cDevice device);
    result_t hasDevice(I2cDevice device, bool* out_has);
};

} // namespace hydra::horizon::services::i2c
