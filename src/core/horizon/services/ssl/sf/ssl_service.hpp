#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::ssl::sf {

enum class SystemVersion : u32 {
    _3_0_0 = 1,
    _5_0_0 = 2,
    _6_0_0 = 3,
    _20_0_0 = 4,
};

class ISslService : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t SetInterfaceVersion(SystemVersion version);
};

} // namespace hydra::horizon::services::ssl::sf

ENABLE_ENUM_FORMATTING(hydra::horizon::services::ssl::sf::SystemVersion, _3_0_0,
                       "3.0.0+", _5_0_0, "5.0.0+", _6_0_0, "6.0.0+", _20_0_0,
                       "20.0.0+")
