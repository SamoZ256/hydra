#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::fatalsrv {

class IService : public ::hydra::horizon::services::IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    STUB_REQUEST_COMMAND(throwFatalWithPolicy);
    // TODO: PID descriptor
    result_t throwFatalWithCpuContext(u64 code, u64 _unknown_x8,
                                      InBuffer<BufferAttr::MapAlias> in_buffer);
};

} // namespace hydra::horizon::services::fatalsrv
