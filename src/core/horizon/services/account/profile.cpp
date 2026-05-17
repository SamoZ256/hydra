#include "core/horizon/services/account/profile.hpp"

#include "core/system.hpp"

namespace hydra::horizon::services::account {

DEFINE_SERVICE_COMMAND_TABLE(IProfile, 0, Get, 1, GetBase, 10, GetImageSize, 11,
                             LoadImage)

result_t
IProfile::Get(System* system, ProfileBase* out_base,
              OutBuffer<BufferAttr::HipcPointer> out_user_data_buffer) {
    const auto& user = system->GetOS().GetUserManager().GetUser(user_id);
    *out_base = user.GetBase();
    out_user_data_buffer.stream->Write(user.GetData());
    return RESULT_SUCCESS;
}

result_t IProfile::GetBase(System* system, ProfileBase* out_base) {
    const auto& user = system->GetOS().GetUserManager().GetUser(user_id);
    *out_base = user.GetBase();
    return RESULT_SUCCESS;
}

// TODO: get the size without loading the whole image?
result_t IProfile::GetImageSize(System* system, u32* out_size) {
    const auto& user = system->GetOS().GetUserManager().GetUser(user_id);

    std::vector<u8> data;
    system->GetOS().GetUserManager().LoadAvatarImageAsJpeg(
        user.GetAvatarPath(), user.GetAvatarBgColor(), data);

    *out_size = static_cast<u32>(data.size());
    return RESULT_SUCCESS;
}

result_t IProfile::LoadImage(System* system,
                             OutBuffer<BufferAttr::MapAlias> out_buffer,
                             u32* out_size) {
    const auto& user = system->GetOS().GetUserManager().GetUser(user_id);

    // Load image
    std::vector<u8> data;
    system->GetOS().GetUserManager().LoadAvatarImageAsJpeg(
        user.GetAvatarPath(), user.GetAvatarBgColor(), data);

    out_buffer.stream->WriteSpan(std::span<const u8>(data));

    *out_size = static_cast<u32>(data.size());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::account
