#include "core/horizon/services/settings/settings_server.hpp"

#include "core/horizon/services/settings/const.hpp"

namespace hydra::horizon::services::settings {

DEFINE_SERVICE_COMMAND_TABLE(ISettingsServer, 0, getLanguageCode, 1,
                             getAvailableLanguageCodes, 3,
                             getAvailableLanguageCodeCount, 4, getRegionCode, 5,
                             getAvailableLanguageCodes2, 11, getDeviceNickName)

result_t ISettingsServer::getLanguageCode(LanguageCode* out_language_code) {
    *out_language_code = toLanguageCode(CONFIG_INSTANCE.getSystemLanguage());
    return RESULT_SUCCESS;
}

result_t ISettingsServer::getAvailableLanguageCodes(
    i32* out_count, OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    out_buffer.stream->writeSpan(
        std::span(available_languages, SIZEOF_ARRAY(available_languages)));
    *out_count = SIZEOF_ARRAY(available_languages);
    return RESULT_SUCCESS;
}

result_t ISettingsServer::getAvailableLanguageCodeCount(i32* out_count) {
    *out_count = SIZEOF_ARRAY(available_languages);
    return RESULT_SUCCESS;
}

result_t ISettingsServer::getRegionCode(RegionCode* out_code) {
    // TODO: make this configurable
    *out_code = RegionCode::Europe;
    return RESULT_SUCCESS;
}

result_t ISettingsServer::getAvailableLanguageCodes2(
    i32* out_count, OutBuffer<BufferAttr::MapAlias> out_buffer) {
    out_buffer.stream->writeSpan(
        std::span(available_languages, SIZEOF_ARRAY(available_languages)));
    *out_count = SIZEOF_ARRAY(available_languages);
    return RESULT_SUCCESS;
}

result_t
ISettingsServer::getDeviceNickName(OutBuffer<BufferAttr::MapAlias> out_buffer) {
    DeviceNickName nickname{};
    std::memset(nickname.name, 0, SIZEOF_ARRAY(nickname.name));
    std::memcpy(nickname.name, CONFIG_INSTANCE.getDeviceNickname().data(),
                CONFIG_INSTANCE.getDeviceNickname().size());
    out_buffer.stream->write(nickname);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::settings
