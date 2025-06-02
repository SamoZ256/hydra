#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

enum class PerformanceMode : i32 {
    Invalid = -1,
    Normal = 0,
    Boost = 1,
};

class ISession : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t SetPerformanceConfiguration(PerformanceMode mode, u32 config);
};

} // namespace hydra::horizon::services::am

ENABLE_ENUM_FORMATTING(hydra::horizon::services::am::PerformanceMode, Invalid,
                       "invalid", Normal, "normal", Boost, "boost");
