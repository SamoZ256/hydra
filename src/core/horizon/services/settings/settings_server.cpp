#include "core/horizon/services/settings/settings_server.hpp"

#include "core/horizon/services/settings/const.hpp"

namespace hydra::horizon::services::settings {

DEFINE_SERVICE_COMMAND_TABLE(ISettingsServer, 0, GetLanguageCode, 1,
                             GetAvailableLanguageCodes, 3,
                             GetAvailableLanguageCodeCount, 4, GetRegionCode, 5,
                             GetAvailableLanguageCodes2, 11, GetDeviceNickName)

result_t ISettingsServer::GetLanguageCode(LanguageCode* out_language_code) {
    *out_language_code = ToLanguageCode(CONFIG_INSTANCE.GetSystemLanguage());
    return RESULT_SUCCESS;
}

result_t ISettingsServer::GetAvailableLanguageCodes(
    i32* out_count, OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    out_buffer.stream->writeSpan(
        std::span(available_languages, sizeof_array(available_languages)));
    *out_count = sizeof_array(available_languages);
    return RESULT_SUCCESS;
}

result_t ISettingsServer::GetAvailableLanguageCodeCount(i32* out_count) {
    *out_count = sizeof_array(available_languages);
    return RESULT_SUCCESS;
}

result_t ISettingsServer::GetRegionCode(RegionCode* out_code) {
    // TODO: make this configurable
    *out_code = RegionCode::Europe;
    return RESULT_SUCCESS;
}

result_t ISettingsServer::GetAvailableLanguageCodes2(
    i32* out_count, OutBuffer<BufferAttr::MapAlias> out_buffer) {
    out_buffer.stream->writeSpan(
        std::span(available_languages, sizeof_array(available_languages)));
    *out_count = sizeof_array(available_languages);
    return RESULT_SUCCESS;
}

result_t
ISettingsServer::GetDeviceNickName(OutBuffer<BufferAttr::MapAlias> out_buffer) {
    DeviceNickName nickname{};
    std::memset(nickname.name, 0, sizeof_array(nickname.name));
    std::memcpy(nickname.name, CONFIG_INSTANCE.GetDeviceNickname().data(),
                CONFIG_INSTANCE.GetDeviceNickname().size());
    out_buffer.stream->write(nickname);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::settings
