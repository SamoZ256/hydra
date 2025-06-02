#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::ts {

enum class Location {
    Internal,
    External,
};

class IMeasurementServer : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetTemperature(Location location, i32* out_temperature_celsius);
};

} // namespace hydra::horizon::services::ts

ENABLE_ENUM_FORMATTING(hydra::horizon::services::ts::Location, Internal,
                       "internal", External, "external")
