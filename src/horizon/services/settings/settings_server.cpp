#include "horizon/services/settings/settings_server.hpp"

namespace Hydra::Horizon::Services::Settings {

namespace {

enum class LanguageCode : u64 {
    Japanese = str_to_u64("ja"),
    AmericanEnglish = str_to_u64("en-US"),
    French = str_to_u64("fr"),
    German = str_to_u64("de"),
    Italian = str_to_u64("it"),
    Spanish = str_to_u64("es"),
    Chinese = str_to_u64("zh-CN"),
    Korean = str_to_u64("ko"),
    Dutch = str_to_u64("nl"),
    Portuguese = str_to_u64("pt"),
    FuckRussia = str_to_u64("ru"), // TODO: should be "Russian"
    Russian = FuckRussia,
    Taiwanese = str_to_u64("zh-TW"),
    BritishEnglish = str_to_u64("en-GB"),
    CanadianFrench = str_to_u64("fr-CA"),
    LatinAmericanSpanish = str_to_u64("es-419"),
    SimplifiedChinese = str_to_u64("zh-Hans"),
    TraditionalChinese = str_to_u64("zh-Hant"),
    BrazilianPortugese = str_to_u64("pt-BR"),
};

LanguageCode available_languages[] = {
    LanguageCode::Japanese,
    LanguageCode::AmericanEnglish,
    LanguageCode::French,
    LanguageCode::German,
    LanguageCode::Italian,
    LanguageCode::Spanish,
    LanguageCode::Chinese,
    LanguageCode::Korean,
    LanguageCode::Dutch,
    LanguageCode::Portuguese,
    LanguageCode::Russian,
    LanguageCode::Taiwanese,
    LanguageCode::BritishEnglish,
    LanguageCode::CanadianFrench,
    LanguageCode::LatinAmericanSpanish,
    LanguageCode::SimplifiedChinese,
    LanguageCode::TraditionalChinese,
    LanguageCode::BrazilianPortugese,
};

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(ISettingsServer, 0, GetLanguageCode, 1,
                             GetAvailableLanguageCodes, 3,
                             GetAvailableLanguageCodeCount)

void ISettingsServer::GetLanguageCode(REQUEST_COMMAND_PARAMS) {
    // TODO: make this configurable
    writers.writer.Write(LanguageCode::AmericanEnglish);
}

void ISettingsServer::GetAvailableLanguageCodes(REQUEST_COMMAND_PARAMS) {
    writers.recv_list_writers[0].Write(available_languages,
                                       sizeof_array(available_languages));
    writers.writer.Write<i32>(sizeof_array(available_languages));
}

void ISettingsServer::GetAvailableLanguageCodeCount(REQUEST_COMMAND_PARAMS) {
    writers.writer.Write<i32>(sizeof_array(available_languages));
}

} // namespace Hydra::Horizon::Services::Settings
