#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::spl {

class IRandomInterface : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    // TODO: map alias?
    result_t GenerateRandomBytes(OutBuffer<BufferAttr::MapAlias> out_buffer);
};

} // namespace hydra::horizon::services::spl
