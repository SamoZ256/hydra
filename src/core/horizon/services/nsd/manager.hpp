#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::nsd {

class IManager : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    // TODO: buffer attr
    result_t
    getEnvironmentIdentifier(OutBuffer<BufferAttr::AutoSelect> out_buffer);
};

} // namespace hydra::horizon::services::nsd
