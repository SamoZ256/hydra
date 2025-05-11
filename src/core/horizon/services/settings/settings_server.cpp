#include "core/horizon/services/settings/settings_server.hpp"

namespace Hydra::Horizon::Services::Settings {

DEFINE_SERVICE_COMMAND_TABLE(ISettingsServer, 0, GetLanguageCode, 1,
                             GetAvailableLanguageCodes, 3,
                             GetAvailableLanguageCodeCount)

result_t ISettingsServer::GetLanguageCode(LanguageCode* out_language_code) {
    // TODO: make this configurable
    *out_language_code = LanguageCode::AmericanEnglish;
    return RESULT_SUCCESS;
}

result_t ISettingsServer::GetAvailableLanguageCodes(
    i32* out_count, OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    out_buffer.writer->WritePtr(available_languages,
                                sizeof_array(available_languages));
    *out_count = sizeof_array(available_languages);
    return RESULT_SUCCESS;
}

result_t ISettingsServer::GetAvailableLanguageCodeCount(i32* out_count) {
    *out_count = sizeof_array(available_languages);
    return RESULT_SUCCESS;
}

} // namespace Hydra::Horizon::Services::Settings
