#include "core/horizon/services/account/profile.hpp"

#include "core/system.hpp"

namespace hydra::horizon::services::account {

DEFINE_SERVICE_COMMAND_TABLE(IProfile, 0, get, 1, getBase, 10, getImageSize, 11,
                             loadImage)

result_t
IProfile::get(System* system, ProfileBase* out_base,
              OutBuffer<BufferAttr::HipcPointer> out_user_data_buffer) const {
    const auto& user = system->getOs().getUserManager().getUser(user_id);
    *out_base = user.getBase();
    out_user_data_buffer.stream->write(user.getData());
    return RESULT_SUCCESS;
}

result_t IProfile::getBase(System* system, ProfileBase* out_base) const {
    const auto& user = system->getOs().getUserManager().getUser(user_id);
    *out_base = user.getBase();
    return RESULT_SUCCESS;
}

// TODO: get the size without loading the whole image?
result_t IProfile::getImageSize(System* system, u32* out_size) const {
    const auto& user = system->getOs().getUserManager().getUser(user_id);

    std::vector<u8> data;
    system->getOs().getUserManager().loadAvatarImageAsJpeg(
        user.getAvatarPath(), user.getAvatarBgColor(), data);

    *out_size = static_cast<u32>(data.size());
    return RESULT_SUCCESS;
}

result_t IProfile::loadImage(System* system,
                             OutBuffer<BufferAttr::MapAlias> out_buffer,
                             u32* out_size) const {
    const auto& user = system->getOs().getUserManager().getUser(user_id);

    // Load image
    std::vector<u8> data;
    system->getOs().getUserManager().loadAvatarImageAsJpeg(
        user.getAvatarPath(), user.getAvatarBgColor(), data);

    out_buffer.stream->writeSpan(std::span<const u8>(data));

    *out_size = static_cast<u32>(data.size());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::account
