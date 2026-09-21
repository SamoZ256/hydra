#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::account::baas {

class IManagerForApplication : public IService {
  public:
    explicit IManagerForApplication(uuid_t user_id_) : user_id{user_id_} {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // TODO: use
    [[maybe_unused]] uuid_t user_id;

    // Commands
    STUB_REQUEST_COMMAND(checkAvailability);
    result_t getAccountId(u64* out_id);
};

} // namespace hydra::horizon::services::account::baas
