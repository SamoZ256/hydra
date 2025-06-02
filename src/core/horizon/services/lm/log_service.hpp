#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::lm {

class ILogService : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t OpenLogger(add_service_fn_t add_service, u64 process_id);
};

} // namespace hydra::horizon::services::lm
