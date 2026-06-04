#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::mii {

class IImageDatabaseService : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t Initialize(bool use_database, bool* out_is_dirty);
    result_t GetCount(u32* out_count);
};

} // namespace hydra::horizon::services::mii
