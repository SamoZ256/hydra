#include "core/c_api.h"

#include "core/debugger/debugger_manager.hpp"
#include "core/horizon/filesystem/content_archive.hpp"
#include "core/horizon/filesystem/disk_file.hpp"
#include "core/horizon/loader/nca_loader.hpp"
#include "core/horizon/loader/plugins/manager.hpp"
#include "core/horizon/services/timesrv/internal/time_zone_manager.hpp"
#include "core/horizon/ui/handler_base.hpp"
#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture.hpp"
#include "core/system.hpp"

#define HYDRA_EXPORT extern "C" __attribute__((visibility("default")))

namespace {

hydra_string hydraStringFromStringView(std::string_view str) {
    return hydra_string{.data = str.data(), .size = str.size()};
}

std::string_view stringViewFromHydraString(hydra_string str) {
    return {str.data, str.size};
}

} // namespace

// String list
HYDRA_EXPORT void* hydraCreateStringList() {
    return new std::vector<std::string>();
}

HYDRA_EXPORT void hydraStringListDestroy(void* list) {
    delete reinterpret_cast<std::vector<std::string>*>(list);
}

HYDRA_EXPORT uint32_t hydraStringListGetCount(const void* list) {
    return static_cast<uint32_t>(
        reinterpret_cast<const std::vector<std::string>*>(list)->size());
}

HYDRA_EXPORT hydra_string hydraStringListGet(const void* list, uint32_t index) {
    return hydraStringFromStringView(
        reinterpret_cast<const std::vector<std::string>*>(list)->at(index));
}

HYDRA_EXPORT void hydraStringListResize(void* list, uint32_t size) {
    reinterpret_cast<std::vector<std::string>*>(list)->resize(size);
}

HYDRA_EXPORT void hydraStringListSet(void* list, uint32_t index,
                                     hydra_string value) {
    (*reinterpret_cast<std::vector<std::string>*>(list))[index] =
        stringViewFromHydraString(value);
}

HYDRA_EXPORT void hydraStringListAppend(void* list, hydra_string value) {
    reinterpret_cast<std::vector<std::string>*>(list)->emplace_back(
        stringViewFromHydraString(value));
}

// String view list
HYDRA_EXPORT uint32_t hydraStringViewListGetCount(const void* list) {
    return static_cast<uint32_t>(
        reinterpret_cast<const std::vector<std::string_view>*>(list)->size());
}

HYDRA_EXPORT hydra_string hydraStringViewListGet(const void* list,
                                                 uint32_t index) {
    return hydraStringFromStringView(
        reinterpret_cast<const std::vector<std::string_view>*>(list)->at(
            index));
}

HYDRA_EXPORT void hydraStringViewListResize(void* list, uint32_t size) {
    reinterpret_cast<std::vector<std::string_view>*>(list)->resize(size);
}

HYDRA_EXPORT void hydraStringViewListSet(void* list, uint32_t index,
                                         hydra_string value) {
    (*reinterpret_cast<std::vector<std::string_view>*>(list))[index] =
        stringViewFromHydraString(value);
}

HYDRA_EXPORT void hydraStringViewListAppend(void* list, hydra_string value) {
    reinterpret_cast<std::vector<std::string_view>*>(list)->push_back(
        stringViewFromHydraString(value));
}

// String to string map
HYDRA_EXPORT void* hydraCreateStringToStringMap() {
    return new std::map<std::string, std::string>();
}

HYDRA_EXPORT void hydraStringToStringMapDestroy(void* map) {
    delete reinterpret_cast<std::map<std::string, std::string>*>(map);
}

HYDRA_EXPORT uint32_t hydraStringToStringMapGetCount(const void* map) {
    return static_cast<uint32_t>(
        reinterpret_cast<const std::map<std::string, std::string>*>(map)
            ->size());
}

HYDRA_EXPORT hydra_string hydraStringToStringMapGetKey(const void* map,
                                                       uint32_t index) {
    auto it = reinterpret_cast<const std::map<std::string, std::string>*>(map)
                  ->begin();
    std::advance(it, index);
    return hydraStringFromStringView(it->first);
}

HYDRA_EXPORT hydra_string hydraStringToStringMapGetValue(const void* map,
                                                         uint32_t index) {
    auto it = reinterpret_cast<const std::map<std::string, std::string>*>(map)
                  ->begin();
    std::advance(it, index);
    return hydraStringFromStringView(it->second);
}

HYDRA_EXPORT hydra_string
hydraStringToStringMapGetValueByKey(const void* map, hydra_string key) {
    return hydraStringFromStringView(
        reinterpret_cast<const std::map<std::string, std::string>*>(map)->at(
            std::string(stringViewFromHydraString(key))));
}

HYDRA_EXPORT void hydraStringToStringMapRemoveAll(void* map) {
    reinterpret_cast<std::map<std::string, std::string>*>(map)->clear();
}

HYDRA_EXPORT void hydraStringToStringMapSetByKey(void* map, hydra_string key,
                                                 hydra_string value) {
    (*reinterpret_cast<std::map<std::string, std::string>*>(
        map))[std::string(stringViewFromHydraString(key))] =
        stringViewFromHydraString(value);
}

// Loader plugin
HYDRA_EXPORT hydra_string hydraLoaderPluginGetPath(const void* plugin) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::LoaderPlugin*>(plugin)->path);
}

HYDRA_EXPORT void hydraLoaderPluginSetPath(void* plugin, hydra_string path) {
    reinterpret_cast<hydra::LoaderPlugin*>(plugin)->path =
        stringViewFromHydraString(path);
}

HYDRA_EXPORT void* hydraLoaderPluginGetOptions(void* plugin) {
    return &reinterpret_cast<hydra::LoaderPlugin*>(plugin)->options;
}

HYDRA_EXPORT uint32_t hydraLoaderPluginListGetCount(const void* list) {
    return static_cast<uint32_t>(
        reinterpret_cast<const std::vector<hydra::LoaderPlugin>*>(list)
            ->size());
}

HYDRA_EXPORT void* hydraLoaderPluginListGet(void* list, uint32_t index) {
    return &reinterpret_cast<std::vector<hydra::LoaderPlugin>*>(list)->at(
        index);
}

HYDRA_EXPORT void hydraLoaderPluginListResize(void* list, uint32_t size) {
    reinterpret_cast<std::vector<hydra::LoaderPlugin>*>(list)->resize(size);
}

// Config
HYDRA_EXPORT void hydraConfigSerialize() { hydra::CONFIG_INSTANCE.Serialize(); }

HYDRA_EXPORT void hydraConfigDeserialize() {
    hydra::CONFIG_INSTANCE.Deserialize();
}

HYDRA_EXPORT hydra_string hydraConfigGetAppDataPath() {
    return hydraStringFromStringView(hydra::CONFIG_INSTANCE.GetAppDataPath());
}

HYDRA_EXPORT hydra_string hydraConfigGetLogsPath() {
    return hydraStringFromStringView(hydra::CONFIG_INSTANCE.GetLogsPath());
}

HYDRA_EXPORT void* hydraConfigGetGamePaths() {
    return &hydra::CONFIG_INSTANCE.GetGamePaths();
}

HYDRA_EXPORT void* hydraConfigGetLoaderPlugins() {
    return &hydra::CONFIG_INSTANCE.GetLoaderPlugins();
}

HYDRA_EXPORT void* hydraConfigGetPatchPaths() {
    return &hydra::CONFIG_INSTANCE.GetPatchPaths();
}

HYDRA_EXPORT uint32_t* hydraConfigGetInputBackend() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.GetInputBackend());
}

HYDRA_EXPORT void* hydraConfigGetInputProfiles() {
    return &hydra::CONFIG_INSTANCE.GetInputProfiles();
}

HYDRA_EXPORT uint32_t* hydraConfigGetCpuBackend() {
    return reinterpret_cast<uint32_t*>(&hydra::CONFIG_INSTANCE.GetCpuBackend());
}

HYDRA_EXPORT uint32_t* hydraConfigGetGpuRenderer() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.GetGpuRenderer());
}

HYDRA_EXPORT uint32_t* hydraConfigGetShaderBackend() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.GetShaderBackend());
}

HYDRA_EXPORT uint32_t* hydraConfigGetDisplayResolution() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.GetDisplayResolution());
}

HYDRA_EXPORT hydra_uint2* hydraConfigGetCustomDisplayResolution() {
    return reinterpret_cast<hydra_uint2*>(
        &hydra::CONFIG_INSTANCE.GetCustomDisplayResolution());
}

HYDRA_EXPORT uint32_t* hydraConfigGetAudioBackend() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.GetAudioBackend());
}

HYDRA_EXPORT hydra_u128* hydraConfigGetUserId() {
    return reinterpret_cast<hydra_u128*>(&hydra::CONFIG_INSTANCE.GetUserId());
}

HYDRA_EXPORT hydra_string hydraConfigGetDeviceNickname() {
    return hydraStringFromStringView(
        hydra::CONFIG_INSTANCE.GetDeviceNickname());
}

HYDRA_EXPORT void hydraConfigSetDeviceNickname(hydra_string value) {
    hydra::CONFIG_INSTANCE.GetDeviceNickname() =
        stringViewFromHydraString(value);
}

HYDRA_EXPORT uint32_t* hydraConfigGetSystemLanguage() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.GetSystemLanguage());
}

HYDRA_EXPORT hydra_string hydraConfigGetSystemLocation() {
    return hydraStringFromStringView(
        hydra::CONFIG_INSTANCE.GetSystemLocation());
}

HYDRA_EXPORT void hydraConfigSetSystemLocation(hydra_string value) {
    hydra::CONFIG_INSTANCE.GetSystemLocation() =
        stringViewFromHydraString(value);
}

HYDRA_EXPORT hydra_string hydraConfigGetFirmwarePath() {
    return hydraStringFromStringView(hydra::CONFIG_INSTANCE.GetFirmwarePath());
}

HYDRA_EXPORT void hydraConfigSetFirmwarePath(hydra_string value) {
    hydra::CONFIG_INSTANCE.GetFirmwarePath() = stringViewFromHydraString(value);
}

HYDRA_EXPORT hydra_string hydraConfigGetSdCardPath() {
    return hydraStringFromStringView(hydra::CONFIG_INSTANCE.GetSdCardPath());
}

HYDRA_EXPORT void hydraConfigSetSdCardPath(hydra_string value) {
    hydra::CONFIG_INSTANCE.GetSdCardPath() = stringViewFromHydraString(value);
}

HYDRA_EXPORT hydra_string hydraConfigGetSavePath() {
    return hydraStringFromStringView(hydra::CONFIG_INSTANCE.GetSavePath());
}

HYDRA_EXPORT void hydraConfigSetSavePath(hydra_string value) {
    hydra::CONFIG_INSTANCE.GetSavePath() = stringViewFromHydraString(value);
}

HYDRA_EXPORT hydra_string hydraConfigGetSysmodulesPath() {
    return hydraStringFromStringView(
        hydra::CONFIG_INSTANCE.GetSysmodulesPath());
}

HYDRA_EXPORT void hydraConfigSetSysmodulesPath(hydra_string value) {
    hydra::CONFIG_INSTANCE.GetSysmodulesPath() =
        stringViewFromHydraString(value);
}

HYDRA_EXPORT bool* hydraConfigGetHandheldMode() {
    return &hydra::CONFIG_INSTANCE.GetHandheldMode();
}

HYDRA_EXPORT uint32_t* hydraConfigGetLogOutput() {
    return reinterpret_cast<uint32_t*>(&hydra::CONFIG_INSTANCE.GetLogOutput());
}

HYDRA_EXPORT bool* hydraConfigGetLogFsAccess() {
    return &hydra::CONFIG_INSTANCE.GetLogFsAccess();
}

HYDRA_EXPORT bool* hydraConfigGetDebugLogging() {
    return &hydra::CONFIG_INSTANCE.GetDebugLogging();
}

HYDRA_EXPORT void* hydraConfigGetProcessArgs() {
    return &hydra::CONFIG_INSTANCE.GetProcessArgs();
}

HYDRA_EXPORT bool* hydraConfigGetRecoverFromSegfault() {
    return &hydra::CONFIG_INSTANCE.GetRecoverFromSegfault();
}

HYDRA_EXPORT bool* hydraConfigGetGdbEnabled() {
    return &hydra::CONFIG_INSTANCE.GetGdbEnabled();
}

HYDRA_EXPORT uint16_t* hydraConfigGetGdbPort() {
    return &hydra::CONFIG_INSTANCE.GetGdbPort();
}

HYDRA_EXPORT bool* hydraConfigGetGdbWaitForClient() {
    return &hydra::CONFIG_INSTANCE.GetGdbWaitForClient();
}

// Option config
HYDRA_EXPORT void* hydraLoaderPluginOptionConfigCopy(const void* config) {
    return new hydra::horizon::loader::plugins::OptionConfig(
        *reinterpret_cast<const hydra::horizon::loader::plugins::OptionConfig*>(
            config));
}

HYDRA_EXPORT void hydraLoaderPluginOptionConfigDestroy(void* config) {
    delete reinterpret_cast<hydra::horizon::loader::plugins::OptionConfig*>(
        config);
}

HYDRA_EXPORT hydra_string
hydraLoaderPluginOptionConfigGetName(const void* config) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::horizon::loader::plugins::OptionConfig*>(
            config)
            ->name);
}

HYDRA_EXPORT hydra_string
hydraLoaderPluginOptionConfigGetDescription(const void* config) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::horizon::loader::plugins::OptionConfig*>(
            config)
            ->description);
}

HYDRA_EXPORT HydraLoaderPluginOptionType
hydraLoaderPluginOptionConfigGetType(const void* config) {
    return static_cast<HydraLoaderPluginOptionType>(
        reinterpret_cast<const hydra::horizon::loader::plugins::OptionConfig*>(
            config)
            ->type);
}

HYDRA_EXPORT bool
hydraLoaderPluginOptionConfigGetIsRequired(const void* config) {
    return reinterpret_cast<
               const hydra::horizon::loader::plugins::OptionConfig*>(config)
        ->is_required;
}

HYDRA_EXPORT const void*
hydraLoaderPluginOptionConfigGetEnumValueNames(const void* config) {
    return &reinterpret_cast<
                const hydra::horizon::loader::plugins::OptionConfig*>(config)
                ->enum_value_names;
}

HYDRA_EXPORT const void*
hydraLoaderPluginOptionConfigGetPathContentTypes(const void* config) {
    return &reinterpret_cast<
                const hydra::horizon::loader::plugins::OptionConfig*>(config)
                ->path_content_types;
}

// Filesystem
HYDRA_EXPORT void* hydraCreateFilesystem() {
    return new hydra::horizon::filesystem::Filesystem();
}

HYDRA_EXPORT void hydraFilesystemDestroy(void* fs) {
    delete reinterpret_cast<hydra::horizon::filesystem::Filesystem*>(fs);
}

HYDRA_EXPORT void* hydraOpenFile(hydra_string path) {
    return new hydra::horizon::filesystem::DiskFile(
        stringViewFromHydraString(path));
}

HYDRA_EXPORT void hydraFileClose(void* file) {
    delete reinterpret_cast<hydra::horizon::filesystem::DiskFile*>(file);
}

HYDRA_EXPORT void* hydraCreateContentArchive(void* file) {
    return new hydra::horizon::filesystem::ContentArchive(
        reinterpret_cast<hydra::horizon::filesystem::IFile*>(file));
}

HYDRA_EXPORT void hydraContentArchiveDestroy(void* content_archive) {
    delete reinterpret_cast<hydra::horizon::filesystem::ContentArchive*>(
        content_archive);
}

HYDRA_EXPORT HydraContentArchiveContentType
hydraContentArchiveGetContentType(void* content_archive) {
    return static_cast<HydraContentArchiveContentType>(
        reinterpret_cast<hydra::horizon::filesystem::ContentArchive*>(
            content_archive)
            ->getContentType());
}

// Time zone manager
HYDRA_EXPORT void* hydraCreateTimeZoneManager(void* filesystem) {
    return new hydra::horizon::services::timesrv::internal::TimeZoneManager(
        *static_cast<hydra::horizon::filesystem::Filesystem*>(filesystem));
}

HYDRA_EXPORT void hydraTimeZoneManagerDestroy(void* manager) {
    delete static_cast<
        hydra::horizon::services::timesrv::internal::TimeZoneManager*>(manager);
}

HYDRA_EXPORT uint32_t hydraTimeZoneManagerGetLocationCount(void* manager) {
    return static_cast<uint32_t>(
        static_cast<
            hydra::horizon::services::timesrv::internal::TimeZoneManager*>(
            manager)
            ->getLocations()
            .size());
}

HYDRA_EXPORT hydra_string hydraTimeZoneManagerGetLocation(void* manager,
                                                          uint32_t index) {
    // HACK
    auto it =
        static_cast<
            hydra::horizon::services::timesrv::internal::TimeZoneManager*>(
            manager)
            ->getLocations()
            .begin();
    std::advance(it, index);
    return hydraStringFromStringView(*it);
}

// Loader
HYDRA_EXPORT void* hydraCreateLoaderFromPath(hydra_string path,
                                             void* plugin_manager) {
    // TODO: return the error
    return hydra::horizon::loader::ILoader::createFromPath(
               stringViewFromHydraString(path),
               (plugin_manager != nullptr)
                   ? std::make_optional(
                         reinterpret_cast<
                             hydra::horizon::loader::plugins::Manager*>(
                             plugin_manager))
                   : std::nullopt)
        .value_or(nullptr);
}

HYDRA_EXPORT void hydraLoaderDestroy(void* loader) {
    delete reinterpret_cast<hydra::horizon::loader::ILoader*>(loader);
}

HYDRA_EXPORT uint64_t hydraLoaderGetTitleId(void* loader) {
    return reinterpret_cast<hydra::horizon::loader::ILoader*>(loader)
        ->getTitleId();
}

HYDRA_EXPORT void* hydraLoaderLoadNacp(void* loader) {
    return reinterpret_cast<hydra::horizon::loader::ILoader*>(loader)
        ->loadNacp();
}

HYDRA_EXPORT void* hydraLoaderLoadIcon(void* loader, uint32_t* width,
                                       uint32_t* height) {
    return reinterpret_cast<hydra::horizon::loader::ILoader*>(loader)->loadIcon(
        *width, *height);
}

HYDRA_EXPORT bool hydraLoaderHasIcon(const void* loader) {
    return reinterpret_cast<const hydra::horizon::loader::ILoader*>(loader)
        ->hasIcon();
}

HYDRA_EXPORT void hydraLoaderExtractIcon(const void* loader,
                                         hydra_string path) {
    reinterpret_cast<const hydra::horizon::loader::ILoader*>(loader)
        ->extractIcon(stringViewFromHydraString(path));
}

HYDRA_EXPORT bool hydraLoaderHasExefs(const void* loader) {
    return reinterpret_cast<const hydra::horizon::loader::ILoader*>(loader)
        ->hasExeFs();
}

HYDRA_EXPORT void hydraLoaderExtractExefs(const void* loader,
                                          hydra_string path) {
    reinterpret_cast<const hydra::horizon::loader::ILoader*>(loader)
        ->extractExeFs(stringViewFromHydraString(path));
}

HYDRA_EXPORT bool hydraLoaderHasRomfs(const void* loader) {
    return reinterpret_cast<const hydra::horizon::loader::ILoader*>(loader)
        ->hasRomFs();
}

HYDRA_EXPORT void hydraLoaderExtractRomfs(const void* loader,
                                          hydra_string path) {
    reinterpret_cast<const hydra::horizon::loader::ILoader*>(loader)
        ->extractRomFs(stringViewFromHydraString(path));
}

HYDRA_EXPORT void*
hydraCreateNcaLoaderFromContentArchive(void* content_archive) {
    return new hydra::horizon::loader::NcaLoader(
        *reinterpret_cast<hydra::horizon::filesystem::ContentArchive*>(
            content_archive));
}

HYDRA_EXPORT hydra_string hydraNcaLoaderGetName(void* nca_loader) {
    return hydraStringFromStringView(
        reinterpret_cast<hydra::horizon::loader::NcaLoader*>(nca_loader)
            ->getName());
}

// Plugins

// Manager
HYDRA_EXPORT void* hydraCreateLoaderPluginManager() {
    return new hydra::horizon::loader::plugins::Manager();
}

HYDRA_EXPORT void hydraLoaderPluginManagerDestroy(void* manager) {
    delete reinterpret_cast<hydra::horizon::loader::plugins::Manager*>(manager);
}

HYDRA_EXPORT void hydraLoaderPluginManagerRefresh(void* manager) {
    reinterpret_cast<hydra::horizon::loader::plugins::Manager*>(manager)
        ->refresh();
}

// Plugin
HYDRA_EXPORT void* hydraCreateLoaderPlugin(hydra_string path) {
    // TODO: return the error
    return hydra::horizon::loader::plugins::Plugin::create(
               std::string(stringViewFromHydraString(path)))
        .transform([](hydra::horizon::loader::plugins::Plugin plugin) {
            // HACK
            auto ptr =
                reinterpret_cast<hydra::horizon::loader::plugins::Plugin*>(
                    malloc(sizeof(hydra::horizon::loader::plugins::Plugin)));
            *ptr = std::move(plugin);
            return ptr;
        })
        .value_or(nullptr);
}

HYDRA_EXPORT void hydraLoaderPluginDestroy(void* plugin) {
    delete reinterpret_cast<hydra::horizon::loader::plugins::Plugin*>(plugin);
}

HYDRA_EXPORT hydra_string hydraLoaderPluginGetName(const void* plugin) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::horizon::loader::plugins::Plugin*>(plugin)
            ->getName());
}

HYDRA_EXPORT hydra_string
hydraLoaderPluginGetDisplayVersion(const void* plugin) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::horizon::loader::plugins::Plugin*>(plugin)
            ->getDisplayVersion());
}

HYDRA_EXPORT uint32_t
hydraLoaderPluginGetSupportedFormatCount(const void* plugin) {
    return static_cast<uint32_t>(
        reinterpret_cast<const hydra::horizon::loader::plugins::Plugin*>(plugin)
            ->getSupportedFormats()
            .size());
}

HYDRA_EXPORT hydra_string
hydraLoaderPluginGetSupportedFormat(const void* plugin, uint32_t index) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::horizon::loader::plugins::Plugin*>(plugin)
            ->getSupportedFormats()[index]);
}

HYDRA_EXPORT uint32_t
hydraLoaderPluginGetOptionConfigCount(const void* plugin) {
    return static_cast<uint32_t>(
        reinterpret_cast<const hydra::horizon::loader::plugins::Plugin*>(plugin)
            ->getOptionConfigs()
            .size());
}

HYDRA_EXPORT const void* hydraLoaderPluginGetOptionConfig(const void* plugin,
                                                          uint32_t index) {
    return &reinterpret_cast<const hydra::horizon::loader::plugins::Plugin*>(
                plugin)
                ->getOptionConfigs()[index];
}

// NACP
HYDRA_EXPORT void hydraNacpDestroy(void* nacp) {
    delete reinterpret_cast<
        hydra::horizon::services::ns::ApplicationControlProperty*>(nacp);
}

HYDRA_EXPORT const void* hydraNacpGetTitle(void* nacp,
                                           HydraSystemLanguage lang) {
    return &reinterpret_cast<
                hydra::horizon::services::ns::ApplicationControlProperty*>(nacp)
                ->getApplicationTitle(static_cast<hydra::SystemLanguage>(lang));
}

HYDRA_EXPORT hydra_string hydraNacpGetDisplayVersion(void* nacp) {
    return hydraStringFromStringView(
        reinterpret_cast<
            hydra::horizon::services::ns::ApplicationControlProperty*>(nacp)
            ->display_version);
}

// NACP title
HYDRA_EXPORT hydra_string hydraNacpTitleGetName(const void* title) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::horizon::services::ns::ApplicationTitle*>(
            title)
            ->name);
}

HYDRA_EXPORT hydra_string hydraNacpTitleGetAuthor(const void* title) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::horizon::services::ns::ApplicationTitle*>(
            title)
            ->author);
}

// User manager
HYDRA_EXPORT void* hydraCreateUserManager() {
    return new hydra::horizon::services::account::internal::UserManager();
}

HYDRA_EXPORT void hydraUserManagerDestroy(void* user_manager) {
    delete reinterpret_cast<
        hydra::horizon::services::account::internal::UserManager*>(
        user_manager);
}

HYDRA_EXPORT void hydraUserManagerFlush(void* user_manager) {
    reinterpret_cast<hydra::horizon::services::account::internal::UserManager*>(
        user_manager)
        ->flush();
}

HYDRA_EXPORT hydra_u128 hydraUserManagerCreateUser(void* user_manager) {
    return std::bit_cast<hydra_u128>(
        reinterpret_cast<
            hydra::horizon::services::account::internal::UserManager*>(
            user_manager)
            ->createUser());
}

HYDRA_EXPORT uint32_t hydraUserManagerGetUserCount(void* user_manager) {
    return static_cast<uint32_t>(
        reinterpret_cast<
            hydra::horizon::services::account::internal::UserManager*>(
            user_manager)
            ->getUserCount());
}

HYDRA_EXPORT hydra_u128 hydraUserManagerGetUserId(void* user_manager,
                                                  uint32_t index) {
    return std::bit_cast<hydra_u128>(
        reinterpret_cast<
            hydra::horizon::services::account::internal::UserManager*>(
            user_manager)
            ->getUserIDs()[index]);
}

HYDRA_EXPORT void* hydraUserManagerGetUser(void* user_manager,
                                           hydra_u128 user_id) {
    return &reinterpret_cast<
                hydra::horizon::services::account::internal::UserManager*>(
                user_manager)
                ->getUser(std::bit_cast<hydra::u128>(user_id));
}

HYDRA_EXPORT void hydraUserManagerLoadSystemAvatars(void* user_manager,
                                                    void* fs) {
    reinterpret_cast<hydra::horizon::services::account::internal::UserManager*>(
        user_manager)
        ->loadSystemAvatars(
            *reinterpret_cast<hydra::horizon::filesystem::Filesystem*>(fs));
}

HYDRA_EXPORT const void*
hydraUserManagerLoadAvatarImage(void* user_manager, hydra_string path,
                                uint32_t* out_dimensions) {
    return reinterpret_cast<
               hydra::horizon::services::account::internal::UserManager*>(
               user_manager)
        ->loadAvatarImage(stringViewFromHydraString(path), *out_dimensions)
        .data();
}

HYDRA_EXPORT uint32_t hydraUserManagerGetAvatarCount(void* user_manager) {
    return static_cast<uint32_t>(
        reinterpret_cast<
            hydra::horizon::services::account::internal::UserManager*>(
            user_manager)
            ->getAvatars()
            .size());
}

HYDRA_EXPORT hydra_string hydraUserManagerGetAvatarPath(void* user_manager,
                                                        uint32_t index) {
    return hydraStringFromStringView(
        reinterpret_cast<
            hydra::horizon::services::account::internal::UserManager*>(
            user_manager)
            ->getAvatarPath(index));
}

HYDRA_EXPORT hydra_string hydraUserGetNickname(void* user) {
    return hydraStringFromStringView(
        reinterpret_cast<hydra::horizon::services::account::internal::User*>(
            user)
            ->getNickname());
}

HYDRA_EXPORT void hydraUserSetNickname(void* user, hydra_string nickname) {
    reinterpret_cast<hydra::horizon::services::account::internal::User*>(user)
        ->setNickname(stringViewFromHydraString(nickname));
}

HYDRA_EXPORT hydra_uchar3 hydraUserGetAvatarBgColor(void* user) {
    return std::bit_cast<hydra_uchar3>(
        reinterpret_cast<hydra::horizon::services::account::internal::User*>(
            user)
            ->getAvatarBgColor());
}

HYDRA_EXPORT void hydraUserSetAvatarBgColor(void* user, hydra_uchar3 color) {
    reinterpret_cast<hydra::horizon::services::account::internal::User*>(user)
        ->setAvatarBgColor(std::bit_cast<hydra::uchar3>(color));
}

HYDRA_EXPORT hydra_string hydraUserGetAvatarPath(void* user) {
    return hydraStringFromStringView(
        reinterpret_cast<hydra::horizon::services::account::internal::User*>(
            user)
            ->getAvatarPath());
}

HYDRA_EXPORT void hydraUserSetAvatarPath(void* user, hydra_string path) {
    reinterpret_cast<hydra::horizon::services::account::internal::User*>(user)
        ->setAvatarPath(stringViewFromHydraString(path));
}

// Emulation context
// TODO: proper UI handler
class UiHandler : public hydra::horizon::ui::IHandler {
  public:
    void showMessageDialog(const hydra::horizon::ui::MessageDialogType type,
                           const std::string& title,
                           const std::string& message) override {
        (void)type;
        (void)title;
        (void)message;
    }
    hydra::horizon::applets::software_keyboard::SoftwareKeyboardResult
    showSoftwareKeyboard(const std::string& header_text,
                         const std::string& sub_text,
                         const std::string& guide_text,
                         std::string& out_text) override {
        (void)header_text;
        (void)sub_text;
        (void)guide_text;
        out_text = "";
        return hydra::horizon::applets::software_keyboard::
            SoftwareKeyboardResult::OK;
    }
};

HYDRA_EXPORT void* hydraCreateSystem() {
    return new hydra::System(*(new UiHandler()));
}

HYDRA_EXPORT void hydraSystemDestroy(void* system) {
    // TODO: also destroy the UI handler
    delete reinterpret_cast<hydra::System*>(system);
}

HYDRA_EXPORT void hydraSystemSetSurface(void* system, void* surface) {
    reinterpret_cast<hydra::System*>(system)->setSurface(surface);
}

HYDRA_EXPORT void hydraSystemLoadAndStart(void* system, void* loader) {
    reinterpret_cast<hydra::System*>(system)->loadAndStart(
        reinterpret_cast<hydra::horizon::loader::ILoader*>(loader));
}

HYDRA_EXPORT void hydraSystemRequestStop(void* system) {
    reinterpret_cast<hydra::System*>(system)->requestStop();
}

HYDRA_EXPORT void hydraSystemForceStop(void* system) {
    reinterpret_cast<hydra::System*>(system)->forceStop();
}

HYDRA_EXPORT void hydraSystemPause(void* system) {
    reinterpret_cast<hydra::System*>(system)->pause();
}

HYDRA_EXPORT void hydraSystemResume(void* system) {
    reinterpret_cast<hydra::System*>(system)->resume();
}

HYDRA_EXPORT void hydraSystemNotifyOperationModeChanged(void* system) {
    reinterpret_cast<hydra::System*>(system)->notifyOperationModeChanged();
}

HYDRA_EXPORT void hydraSystemProgressFrame(void* system, uint32_t width,
                                           uint32_t height,
                                           bool* out_dt_average_updated) {
    reinterpret_cast<hydra::System*>(system)->progressFrame(
        width, height, *out_dt_average_updated);
}

HYDRA_EXPORT bool hydraSystemIsRunning(void* system) {
    return reinterpret_cast<hydra::System*>(system)->isRunning();
}

HYDRA_EXPORT float hydraSystemGetLastDeltaTimeAverage(void* system) {
    return reinterpret_cast<hydra::System*>(system)->getLastDeltaTimeAverage();
}

HYDRA_EXPORT void hydraSystemTakeScreenshot(void* system) {
    reinterpret_cast<hydra::System*>(system)->takeScreenshot();
}

HYDRA_EXPORT void hydraSystemCaptureGpuFrame(void* system) {
    reinterpret_cast<hydra::System*>(system)->captureGpuFrame();
}

HYDRA_EXPORT void hydraSystemTextureCacheLock(void* system) {
    reinterpret_cast<hydra::System*>(system)
        ->getGpu()
        .getRenderer()
        .getTextureCache()
        .getMutex()
        .lock();
}

HYDRA_EXPORT void hydraSystemTextureCacheUnlock(void* system) {
    reinterpret_cast<hydra::System*>(system)
        ->getGpu()
        .getRenderer()
        .getTextureCache()
        .getMutex()
        .unlock();
}

HYDRA_EXPORT uint32_t
hydraSystemTextureCacheGetTextureMemoryCount(void* system) {
    // HACK
    return static_cast<uint32_t>(reinterpret_cast<hydra::System*>(system)
                                     ->getGpu()
                                     .getRenderer()
                                     .getTextureCache()
                                     .getMemoryCount());
}

HYDRA_EXPORT const void*
hydraSystemTextureCacheGetTextureMemory(void* system, uint32_t index) {
    return &reinterpret_cast<hydra::System*>(system)
                ->getGpu()
                .getRenderer()
                .getTextureCache()
                .getMemory(index);
}

// Debugger

// Debugger manager
HYDRA_EXPORT void hydraDebuggerManagerLock() {
    hydra::DEBUGGER_MANAGER_INSTANCE.lock();
}

HYDRA_EXPORT void hydraDebuggerManagerUnlock() {
    hydra::DEBUGGER_MANAGER_INSTANCE.unlock();
}

HYDRA_EXPORT uint64_t hydraDebuggerManagerGetDebuggerCount() {
    return hydra::DEBUGGER_MANAGER_INSTANCE.getDebuggerCount();
}

HYDRA_EXPORT void* hydraDebuggerManagerGetDebugger(uint32_t index) {
    return &hydra::DEBUGGER_MANAGER_INSTANCE.getDebugger(index);
}

HYDRA_EXPORT void* hydraDebuggerManagerGetDebuggerForProcess(void* process) {
    return &hydra::DEBUGGER_MANAGER_INSTANCE.getDebugger(
        reinterpret_cast<hydra::horizon::kernel::Process*>(process));
}

// Debugger
HYDRA_EXPORT hydra_string hydraDebuggerGetName(void* debugger) {
    return hydraStringFromStringView(
        reinterpret_cast<hydra::debugger::Debugger*>(debugger)->getName());
}

HYDRA_EXPORT void hydraDebuggerLock(void* debugger) {
    reinterpret_cast<hydra::debugger::Debugger*>(debugger)->lock();
}

HYDRA_EXPORT void hydraDebuggerUnlock(void* debugger) {
    reinterpret_cast<hydra::debugger::Debugger*>(debugger)->unlock();
}

HYDRA_EXPORT void hydraDebuggerRegisterThisThread(void* debugger,
                                                  hydra_string name) {
    reinterpret_cast<hydra::debugger::Debugger*>(debugger)->registerThisThread(
        stringViewFromHydraString(name));
}

HYDRA_EXPORT void hydraDebuggerUnregisterThisThread(void* debugger) {
    reinterpret_cast<hydra::debugger::Debugger*>(debugger)
        ->unregisterThisThread();
}

HYDRA_EXPORT uint64_t hydraDebuggerGetThreadCount(void* debugger) {
    return reinterpret_cast<hydra::debugger::Debugger*>(debugger)
        ->getThreadCount();
}

HYDRA_EXPORT void* hydraDebuggerGetThread(void* debugger, uint32_t index) {
    return &reinterpret_cast<hydra::debugger::Debugger*>(debugger)->getThread(
        index);
}

// Thread
HYDRA_EXPORT hydra_string hydraDebuggerThreadGetName(void* thread) {
    return hydraStringFromStringView(
        reinterpret_cast<hydra::debugger::Thread*>(thread)->getName());
}

HYDRA_EXPORT void hydraDebuggerThreadLock(void* thread) {
    reinterpret_cast<hydra::debugger::Thread*>(thread)->lock();
}

HYDRA_EXPORT void hydraDebuggerThreadUnlock(void* thread) {
    reinterpret_cast<hydra::debugger::Thread*>(thread)->unlock();
}

HYDRA_EXPORT HydraDebuggerThreadStatus
hydraDebuggerThreadGetStatus(void* thread) {
    return static_cast<HydraDebuggerThreadStatus>(
        reinterpret_cast<hydra::debugger::Thread*>(thread)->getStatus());
}

HYDRA_EXPORT hydra_string hydraDebuggerThreadGetBreakReason(void* thread) {
    return hydraStringFromStringView(
        reinterpret_cast<hydra::debugger::Thread*>(thread)->getBreakReason());
}

HYDRA_EXPORT uint64_t hydraDebuggerThreadGetMessageCount(void* thread) {
    return reinterpret_cast<hydra::debugger::Thread*>(thread)
        ->getMessageCount();
}

HYDRA_EXPORT const void* hydraDebuggerThreadGetMessage(void* thread,
                                                       uint32_t index) {
    return &reinterpret_cast<hydra::debugger::Thread*>(thread)->getMessage(
        index);
}

// Message
HYDRA_EXPORT HydraLogLevel hydraDebuggerMessageGetLogLevel(const void* msg) {
    return static_cast<HydraLogLevel>(
        reinterpret_cast<const hydra::debugger::Message*>(msg)->log.level);
}

HYDRA_EXPORT HydraLogClass hydraDebuggerMessageGetLogClass(const void* msg) {
    return static_cast<HydraLogClass>(
        reinterpret_cast<const hydra::debugger::Message*>(msg)->log.c);
}

HYDRA_EXPORT hydra_string hydraDebuggerMessageGetFile(const void* msg) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::debugger::Message*>(msg)->log.file);
}

HYDRA_EXPORT uint32_t hydraDebuggerMessageGetLine(const void* msg) {
    return reinterpret_cast<const hydra::debugger::Message*>(msg)->log.line;
}

HYDRA_EXPORT hydra_string hydraDebuggerMessageGetFunction(const void* msg) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::debugger::Message*>(msg)->log.function);
}

HYDRA_EXPORT hydra_string hydraDebuggerMessageGetString(const void* msg) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::debugger::Message*>(msg)->log.str);
}

HYDRA_EXPORT const void* hydraDebuggerMessageGetStackTrace(const void* msg) {
    return &reinterpret_cast<const hydra::debugger::Message*>(msg)->stack_trace;
}

// Stack trace
HYDRA_EXPORT void* hydraDebuggerStackTraceCopy(const void* stack_trace) {
    return new hydra::debugger::StackTrace(
        *reinterpret_cast<const hydra::debugger::StackTrace*>(stack_trace));
}

HYDRA_EXPORT void hydraDebuggerStackTraceDestroy(void* stack_trace) {
    delete reinterpret_cast<hydra::debugger::StackTrace*>(stack_trace);
}

HYDRA_EXPORT uint32_t
hydraDebuggerStackTraceGetFrameCount(const void* stack_trace) {
    return static_cast<uint32_t>(
        reinterpret_cast<const hydra::debugger::StackTrace*>(stack_trace)
            ->frames.size());
}

HYDRA_EXPORT const void*
hydraDebuggerStackTraceGetFrame(const void* stack_trace, uint32_t index) {
    return &reinterpret_cast<const hydra::debugger::StackTrace*>(stack_trace)
                ->frames[index];
}

// Stack frame
HYDRA_EXPORT void* hydraDebuggerStackFrameResolve(const void* stack_frame) {
    return new hydra::debugger::ResolvedStackFrame(
        reinterpret_cast<const hydra::debugger::StackFrame*>(stack_frame)
            ->resolve());
}

// Resolved stack frame
HYDRA_EXPORT void
hydraDebuggerResolvedStackFrameDestroy(void* resolved_stack_frame) {
    delete reinterpret_cast<hydra::debugger::ResolvedStackFrame*>(
        resolved_stack_frame);
}

HYDRA_EXPORT hydra_string
hydraDebuggerResolvedStackFrameGetModule(const void* resolved_stack_frame) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::debugger::ResolvedStackFrame*>(
            resolved_stack_frame)
            ->module);
}

HYDRA_EXPORT hydra_string
hydraDebuggerResolvedStackFrameGetFunction(const void* resolved_stack_frame) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::debugger::ResolvedStackFrame*>(
            resolved_stack_frame)
            ->function);
}

HYDRA_EXPORT uint64_t
hydraDebuggerResolvedStackFrameGetAddress(const void* resolved_stack_frame) {
    return reinterpret_cast<const hydra::debugger::ResolvedStackFrame*>(
               resolved_stack_frame)
        ->addr;
}

// Texture cache

// Texture memory
HYDRA_EXPORT uint32_t hydraTextureMemoryGetTextureGroupCount(const void* mem) {
    // HACK
    return static_cast<uint32_t>(
        static_cast<const hydra::hw::tegra_x1::gpu::renderer::TextureMem*>(mem)
            ->getTextureGroupCount());
}

HYDRA_EXPORT const void* hydraTextureMemoryGetTextureGroup(const void* mem,
                                                           uint32_t index) {
    return &static_cast<const hydra::hw::tegra_x1::gpu::renderer::TextureMem*>(
                mem)
                ->getTextureGroup(index);
}

// Texture group
HYDRA_EXPORT uint32_t
hydraTextureGroupGetTextureStorageCount(const void* group) {
    // HACK
    return static_cast<uint32_t>(
        static_cast<const hydra::hw::tegra_x1::gpu::renderer::TextureGroup*>(
            group)
            ->getStorageCount());
}

HYDRA_EXPORT const void* hydraTextureGroupGetTextureStorage(const void* group,
                                                            uint32_t index) {
    return &static_cast<
                const hydra::hw::tegra_x1::gpu::renderer::TextureGroup*>(group)
                ->getStorage(index);
}

// Texture storage
HYDRA_EXPORT const void*
hydraTextureStorageGetTextureDescriptor(const void* storage) {
    return &static_cast<
                const hydra::hw::tegra_x1::gpu::renderer::TextureStorage*>(
                storage)
                ->base->getDescriptor();
}

// Texture descriptor
HYDRA_EXPORT uint64_t hydraTextureDescriptorGetPtr(const void* descriptor) {
    return static_cast<
               const hydra::hw::tegra_x1::gpu::renderer::TextureDescriptor*>(
               descriptor)
        ->ptr;
}

HYDRA_EXPORT HydraTextureType
hydraTextureDescriptorGetType(const void* descriptor) {
    return static_cast<HydraTextureType>(
        static_cast<
            const hydra::hw::tegra_x1::gpu::renderer::TextureDescriptor*>(
            descriptor)
            ->type);
}

HYDRA_EXPORT HydraTextureFormat
hydraTextureDescriptorGetFormat(const void* descriptor) {
    return static_cast<HydraTextureFormat>(
        static_cast<
            const hydra::hw::tegra_x1::gpu::renderer::TextureDescriptor*>(
            descriptor)
            ->format);
}

HYDRA_EXPORT uint32_t hydraTextureDescriptorGetWidth(const void* descriptor) {
    return static_cast<
               const hydra::hw::tegra_x1::gpu::renderer::TextureDescriptor*>(
               descriptor)
        ->width;
}

HYDRA_EXPORT uint32_t hydraTextureDescriptorGetHeight(const void* descriptor) {
    return static_cast<
               const hydra::hw::tegra_x1::gpu::renderer::TextureDescriptor*>(
               descriptor)
        ->height;
}

HYDRA_EXPORT uint32_t hydraTextureDescriptorGetDepth(const void* descriptor) {
    return static_cast<
               const hydra::hw::tegra_x1::gpu::renderer::TextureDescriptor*>(
               descriptor)
        ->depth;
}

HYDRA_EXPORT uint32_t
hydraTextureDescriptorGetLevelCount(const void* descriptor) {
    return static_cast<
               const hydra::hw::tegra_x1::gpu::renderer::TextureDescriptor*>(
               descriptor)
        ->level_count;
}

HYDRA_EXPORT uint32_t
hydraTextureDescriptorGetLayerCount(const void* descriptor) {
    return static_cast<
               const hydra::hw::tegra_x1::gpu::renderer::TextureDescriptor*>(
               descriptor)
        ->layer_count;
}

HYDRA_EXPORT uint32_t
hydraTextureDescriptorGetBlockWidthGobs(const void* descriptor) {
    return 1u
           << static_cast<
                  const hydra::hw::tegra_x1::gpu::renderer::TextureDescriptor*>(
                  descriptor)
                  ->block_width_gobs_log2;
}

HYDRA_EXPORT uint32_t
hydraTextureDescriptorGetBlockHeightGobs(const void* descriptor) {
    return 1u
           << static_cast<
                  const hydra::hw::tegra_x1::gpu::renderer::TextureDescriptor*>(
                  descriptor)
                  ->block_height_gobs_log2;
}

HYDRA_EXPORT uint32_t
hydraTextureDescriptorGetBlockDepthGobs(const void* descriptor) {
    return 1u
           << static_cast<
                  const hydra::hw::tegra_x1::gpu::renderer::TextureDescriptor*>(
                  descriptor)
                  ->block_depth_gobs_log2;
}

HYDRA_EXPORT uint64_t
hydraTextureDescriptorGetLayerSize(const void* descriptor) {
    return static_cast<
               const hydra::hw::tegra_x1::gpu::renderer::TextureDescriptor*>(
               descriptor)
        ->layer_size;
}

HYDRA_EXPORT uint64_t hydraTextureDescriptorGetSize(const void* descriptor) {
    return static_cast<
               const hydra::hw::tegra_x1::gpu::renderer::TextureDescriptor*>(
               descriptor)
        ->size;
}
