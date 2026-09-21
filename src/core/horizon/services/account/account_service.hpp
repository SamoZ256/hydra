#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::account {

enum class AccountServiceType {
    Administrator,
    SystemService,
    Application,
    BaasAccessTokenAccessor,
};

class IAccountService : public IService {
  public:
    explicit IAccountService(AccountServiceType type_) : type{type_} {}

  protected:
    // Commands
    result_t getUserCount(System* system, i32* out_count);
    result_t getUserExistence(System* system, uuid_t user_id, bool* out_exists);
    result_t listAllUsers(System* system,
                          OutBuffer<BufferAttr::HipcPointer> out_buffer);
    // TODO: correct?
    result_t listOpenUsers(System* system,
                           OutBuffer<BufferAttr::HipcPointer> out_buffer);
    result_t getLastOpenedUser(System* system, uuid_t* out_user_id);
    result_t getProfile(RequestContext* ctx, uuid_t user_id);
    // TODO: PID and PID reserved
    result_t isUserRegistrationRequestPermitted(bool* out_permitted);

  private:
    AccountServiceType type;
};

} // namespace hydra::horizon::services::account
