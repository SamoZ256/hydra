#pragma once

#include "core/horizon/services/pl/shared_font_manager.hpp"

namespace hydra::horizon::services::pl::detail {

class IPlatformServiceManagerForSystem : public ISharedFontManager {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;
};

} // namespace hydra::horizon::services::pl::detail
