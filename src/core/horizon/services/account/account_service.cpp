#include "core/horizon/services/account/account_service_for_application.hpp"

#include "core/horizon/os.hpp"
#include "core/horizon/services/account/profile.hpp"

namespace hydra::horizon::services::account {

result_t IAccountService::GetUserCount(i32* out_count) {
    *out_count = static_cast<i32>(USER_MANAGER_INSTANCE.GetUserCount());
    return RESULT_SUCCESS;
}

result_t IAccountService::GetUserExistence(uuid_t user_id, bool* out_exists) {
    LOG_DEBUG(Services, "User ID: 0x{:08x}", user_id);

    *out_exists = USER_MANAGER_INSTANCE.UserExists(user_id);
    return RESULT_SUCCESS;
}

result_t
IAccountService::ListAllUsers(OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    // Clear buffer
    std::memset(out_buffer.writer->GetBase(), 0, out_buffer.writer->GetSize());

    // Write user IDs
    for (const auto user_id : USER_MANAGER_INSTANCE.GetUserIDs()) {
        // Check if we cen fit the entry in the buffer
        if (out_buffer.writer->Tell() + sizeof(uuid_t) >
            out_buffer.writer->GetSize())
            continue;

        out_buffer.writer->Write(user_id);
    }

    return RESULT_SUCCESS;
}

// TODO: how is this different from ListAllUsers? Or a better question: what is
// the difference between an opened user and a closed user?
result_t
IAccountService::ListOpenUsers(OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    LOG_FUNC_STUBBED(Services);

    // Clear buffer
    std::memset(out_buffer.writer->GetBase(), 0, out_buffer.writer->GetSize());

    // Write user IDs
    for (const auto user_id : USER_MANAGER_INSTANCE.GetUserIDs()) {
        // Check if we cen fit the entry in the buffer
        if (out_buffer.writer->Tell() + sizeof(uuid_t) >
            out_buffer.writer->GetSize())
            continue;

        out_buffer.writer->Write(user_id);
    }
    // memset((void*)out_buffer.writer->GetBase(), 0,
    //        out_buffer.writer->GetSize());

    return RESULT_SUCCESS;
}

result_t IAccountService::GetLastOpenedUser(uuid_t* out_user_id) {
    LOG_FUNC_STUBBED(Services);

    // HACK: return the first user
    *out_user_id = USER_MANAGER_INSTANCE.GetUserIDs()[0];
    return RESULT_SUCCESS;
}

result_t IAccountService::GetProfile(RequestContext* ctx, uuid_t user_id) {
    AddService(*ctx, new IProfile(user_id));
    return RESULT_SUCCESS;
}

result_t
IAccountService::IsUserRegistrationRequestPermitted(bool* out_permitted) {
    *out_permitted = (type != AccountServiceType::Application);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::account
