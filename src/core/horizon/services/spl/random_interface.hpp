#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::spl {

class IRandomInterface : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    // TODO: map alias?
    result_t generateRandomBytes(OutBuffer<BufferAttr::MapAlias> out_buffer);
};

} // namespace hydra::horizon::services::spl
