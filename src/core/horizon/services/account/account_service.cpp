#include "core/horizon/services/account/account_service_for_application.hpp"

#include "core/horizon/services/account/profile.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::account {

result_t IAccountService::getUserCount(System* system, i32* out_count) {
    *out_count =
        static_cast<i32>(system->getOs().getUserManager().getUserCount());
    return RESULT_SUCCESS;
}

result_t IAccountService::getUserExistence(System* system, uuid_t user_id,
                                           bool* out_exists) {
    LOG_DEBUG(Services, "User ID: 0x{:08x}", user_id);

    *out_exists = system->getOs().getUserManager().userExists(user_id);
    return RESULT_SUCCESS;
}

result_t
IAccountService::listAllUsers(System* system,
                              OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    // Clear buffer
    std::memset(out_buffer.stream->getPtr(), 0, out_buffer.stream->getSize());

    // Write user IDs
    for (const auto user_id : system->getOs().getUserManager().getUserIDs()) {
        // Check if we cen fit the entry in the buffer
        if (out_buffer.stream->getSeek() + sizeof(uuid_t) >
            out_buffer.stream->getSize())
            continue;

        out_buffer.stream->write(user_id);
    }

    return RESULT_SUCCESS;
}

// TODO: how is this different from ListAllUsers? Or a better question: what is
// the difference between an opened user and a closed user?
result_t
IAccountService::listOpenUsers(System* system,
                               OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    LOG_FUNC_STUBBED(Services);

    // Clear buffer
    std::memset(out_buffer.stream->getPtr(), 0, out_buffer.stream->getSize());

    // Write user IDs
    for (const auto user_id : system->getOs().getUserManager().getUserIDs()) {
        // Check if we cen fit the entry in the buffer
        if (out_buffer.stream->getSeek() + sizeof(uuid_t) >
            out_buffer.stream->getSize())
            continue;

        out_buffer.stream->write(user_id);
    }
    // memset((void*)out_buffer.writer->GetBase(), 0,
    //        out_buffer.writer->GetSize());

    return RESULT_SUCCESS;
}

result_t IAccountService::getLastOpenedUser(System* system,
                                            uuid_t* out_user_id) {
    LOG_FUNC_STUBBED(Services);

    // HACK: return the first user
    *out_user_id = system->getOs().getUserManager().getUserIDs()[0];
    return RESULT_SUCCESS;
}

result_t IAccountService::getProfile(RequestContext* ctx, uuid_t user_id) {
    addService(*ctx, new IProfile(user_id));
    return RESULT_SUCCESS;
}

result_t
IAccountService::isUserRegistrationRequestPermitted(bool* out_permitted) {
    *out_permitted = (type != AccountServiceType::Application);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::account
