#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::ts {

enum class Location {
    Internal,
    External,
};

class IMeasurementServer : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t getTemperature(Location location, i32* out_temperature_celsius);
};

} // namespace hydra::horizon::services::ts

ENABLE_ENUM_FORMATTING(hydra::horizon::services::ts::Location, Internal,
                       "internal", External, "external")
