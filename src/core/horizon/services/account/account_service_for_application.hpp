#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::account {

class IAccountServiceForApplication : public ServiceBase {
  public:
    usize GetPointerBufferSize() override { return 0x1000; }

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetUserCount(i32* out_count);
    result_t GetUserExistence(uuid_t user_id, bool* out_exists);
    result_t ListAllUsers(OutBuffer<BufferAttr::HipcPointer> out_buffer);
    result_t GetLastOpenedUser(uuid_t* out_user_id);
    result_t GetProfile(add_service_fn_t add_service, uuid_t user_id);
    STUB_REQUEST_COMMAND(InitializeApplicationInfoV0);
    result_t GetBaasAccountManagerForApplication(add_service_fn_t add_service,
                                                 uuid_t user_id);
    STUB_REQUEST_COMMAND(InitializeApplicationInfo);
    STUB_REQUEST_COMMAND(IsUserAccountSwitchLocked);
};

} // namespace hydra::horizon::services::account
