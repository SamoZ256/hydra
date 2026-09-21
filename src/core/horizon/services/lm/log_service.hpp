#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::lm {

class ILogService : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t openLogger(RequestContext* ctx, u64 process_id);
};

} // namespace hydra::horizon::services::lm
