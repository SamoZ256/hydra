#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

enum class PerformanceMode : i32 {
    Invalid = -1,
    Normal = 0,
    Boost = 1,
};

class ISession : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    std::array<u32, 2> performance_configs = {
        0x20004, 0x92220007}; // TODO: what should this be?

    // Commands
    result_t SetPerformanceConfiguration(PerformanceMode mode, u32 config);
    result_t GetPerformanceConfiguration(PerformanceMode mode, u32* out_config);
};

} // namespace hydra::horizon::services::am

ENABLE_ENUM_FORMATTING(hydra::horizon::services::am::PerformanceMode, Invalid,
                       "invalid", Normal, "normal", Boost, "boost");
