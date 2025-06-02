#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::account {

class IManagerForApplication : public ServiceBase {
  public:
    IManagerForApplication(uuid_t user_id_) : user_id{user_id_} {}

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    uuid_t user_id;

    // Commands
    STUB_REQUEST_COMMAND(CheckAvailability);
    result_t GetAccountId(u64* out_id);
};

} // namespace hydra::horizon::services::account
