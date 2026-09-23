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

HydraString hydraStringFromStringView(std::string_view str) {
    return HydraString{.data = str.data(), .size = str.size()};
}

std::string_view stringViewFromHydraString(HydraString str) {
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

HYDRA_EXPORT HydraString hydraStringListGet(const void* list, uint32_t index) {
    return hydraStringFromStringView(
        reinterpret_cast<const std::vector<std::string>*>(list)->at(index));
}

HYDRA_EXPORT void hydraStringListResize(void* list, uint32_t size) {
    reinterpret_cast<std::vector<std::string>*>(list)->resize(size);
}

HYDRA_EXPORT void hydraStringListSet(void* list, uint32_t index,
                                     HydraString value) {
    (*reinterpret_cast<std::vector<std::string>*>(list))[index] =
        stringViewFromHydraString(value);
}

HYDRA_EXPORT void hydraStringListAppend(void* list, HydraString value) {
    reinterpret_cast<std::vector<std::string>*>(list)->emplace_back(
        stringViewFromHydraString(value));
}

// String view list
HYDRA_EXPORT uint32_t hydraStringViewListGetCount(const void* list) {
    return static_cast<uint32_t>(
        reinterpret_cast<const std::vector<std::string_view>*>(list)->size());
}

HYDRA_EXPORT HydraString hydraStringViewListGet(const void* list,
                                                 uint32_t index) {
    return hydraStringFromStringView(
        reinterpret_cast<const std::vector<std::string_view>*>(list)->at(
            index));
}

HYDRA_EXPORT void hydraStringViewListResize(void* list, uint32_t size) {
    reinterpret_cast<std::vector<std::string_view>*>(list)->resize(size);
}

HYDRA_EXPORT void hydraStringViewListSet(void* list, uint32_t index,
                                         HydraString value) {
    (*reinterpret_cast<std::vector<std::string_view>*>(list))[index] =
        stringViewFromHydraString(value);
}

HYDRA_EXPORT void hydraStringViewListAppend(void* list, HydraString value) {
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

HYDRA_EXPORT HydraString hydraStringToStringMapGetKey(const void* map,
                                                       uint32_t index) {
    auto it = reinterpret_cast<const std::map<std::string, std::string>*>(map)
                  ->begin();
    std::advance(it, index);
    return hydraStringFromStringView(it->first);
}

HYDRA_EXPORT HydraString hydraStringToStringMapGetValue(const void* map,
                                                         uint32_t index) {
    auto it = reinterpret_cast<const std::map<std::string, std::string>*>(map)
                  ->begin();
    std::advance(it, index);
    return hydraStringFromStringView(it->second);
}

HYDRA_EXPORT HydraString
hydraStringToStringMapGetValueByKey(const void* map, HydraString key) {
    return hydraStringFromStringView(
        reinterpret_cast<const std::map<std::string, std::string>*>(map)->at(
            std::string(stringViewFromHydraString(key))));
}

HYDRA_EXPORT void hydraStringToStringMapRemoveAll(void* map) {
    reinterpret_cast<std::map<std::string, std::string>*>(map)->clear();
}

HYDRA_EXPORT void hydraStringToStringMapSetByKey(void* map, HydraString key,
                                                 HydraString value) {
    (*reinterpret_cast<std::map<std::string, std::string>*>(
        map))[std::string(stringViewFromHydraString(key))] =
        stringViewFromHydraString(value);
}

// Loader plugin
HYDRA_EXPORT HydraString hydraLoaderPluginGetPath(const void* plugin) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::LoaderPlugin*>(plugin)->path);
}

HYDRA_EXPORT void hydraLoaderPluginSetPath(void* plugin, HydraString path) {
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
HYDRA_EXPORT void hydraConfigSerialize() { hydra::CONFIG_INSTANCE.serialize(); }

HYDRA_EXPORT void hydraConfigDeserialize() {
    hydra::CONFIG_INSTANCE.deserialize();
}

HYDRA_EXPORT HydraString hydraConfigGetAppDataPath() {
    return hydraStringFromStringView(hydra::CONFIG_INSTANCE.getAppDataPath());
}

HYDRA_EXPORT HydraString hydraConfigGetLogsPath() {
    return hydraStringFromStringView(hydra::CONFIG_INSTANCE.getLogsPath());
}

HYDRA_EXPORT void* hydraConfigGetGamePaths() {
    return &hydra::CONFIG_INSTANCE.getGamePaths();
}

HYDRA_EXPORT void* hydraConfigGetLoaderPlugins() {
    return &hydra::CONFIG_INSTANCE.getLoaderPlugins();
}

HYDRA_EXPORT void* hydraConfigGetPatchPaths() {
    return &hydra::CONFIG_INSTANCE.getPatchPaths();
}

HYDRA_EXPORT uint32_t* hydraConfigGetInputBackend() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.getInputBackend());
}

HYDRA_EXPORT void* hydraConfigGetInputProfiles() {
    return &hydra::CONFIG_INSTANCE.getInputProfiles();
}

HYDRA_EXPORT uint32_t* hydraConfigGetCpuBackend() {
    return reinterpret_cast<uint32_t*>(&hydra::CONFIG_INSTANCE.getCpuBackend());
}

HYDRA_EXPORT uint32_t* hydraConfigGetGpuRenderer() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.getGpuRenderer());
}

HYDRA_EXPORT uint32_t* hydraConfigGetShaderBackend() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.getShaderBackend());
}

HYDRA_EXPORT uint32_t* hydraConfigGetDisplayResolution() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.getDisplayResolution());
}

HYDRA_EXPORT HydraUint2* hydraConfigGetCustomDisplayResolution() {
    return reinterpret_cast<HydraUint2*>(
        &hydra::CONFIG_INSTANCE.getCustomDisplayResolution());
}

HYDRA_EXPORT uint32_t* hydraConfigGetAudioBackend() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.getAudioBackend());
}

HYDRA_EXPORT HydraU128* hydraConfigGetUserId() {
    return reinterpret_cast<HydraU128*>(&hydra::CONFIG_INSTANCE.getUserId());
}

HYDRA_EXPORT HydraString hydraConfigGetDeviceNickname() {
    return hydraStringFromStringView(
        hydra::CONFIG_INSTANCE.getDeviceNickname());
}

HYDRA_EXPORT void hydraConfigSetDeviceNickname(HydraString value) {
    hydra::CONFIG_INSTANCE.getDeviceNickname() =
        stringViewFromHydraString(value);
}

HYDRA_EXPORT uint32_t* hydraConfigGetSystemLanguage() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.getSystemLanguage());
}

HYDRA_EXPORT HydraString hydraConfigGetSystemLocation() {
    return hydraStringFromStringView(
        hydra::CONFIG_INSTANCE.getSystemLocation());
}

HYDRA_EXPORT void hydraConfigSetSystemLocation(HydraString value) {
    hydra::CONFIG_INSTANCE.getSystemLocation() =
        stringViewFromHydraString(value);
}

HYDRA_EXPORT HydraString hydraConfigGetFirmwarePath() {
    return hydraStringFromStringView(hydra::CONFIG_INSTANCE.getFirmwarePath());
}

HYDRA_EXPORT void hydraConfigSetFirmwarePath(HydraString value) {
    hydra::CONFIG_INSTANCE.getFirmwarePath() = stringViewFromHydraString(value);
}

HYDRA_EXPORT HydraString hydraConfigGetSdCardPath() {
    return hydraStringFromStringView(hydra::CONFIG_INSTANCE.getSdCardPath());
}

HYDRA_EXPORT void hydraConfigSetSdCardPath(HydraString value) {
    hydra::CONFIG_INSTANCE.getSdCardPath() = stringViewFromHydraString(value);
}

HYDRA_EXPORT HydraString hydraConfigGetSavePath() {
    return hydraStringFromStringView(hydra::CONFIG_INSTANCE.getSavePath());
}

HYDRA_EXPORT void hydraConfigSetSavePath(HydraString value) {
    hydra::CONFIG_INSTANCE.getSavePath() = stringViewFromHydraString(value);
}

HYDRA_EXPORT HydraString hydraConfigGetSysmodulesPath() {
    return hydraStringFromStringView(
        hydra::CONFIG_INSTANCE.getSysmodulesPath());
}

HYDRA_EXPORT void hydraConfigSetSysmodulesPath(HydraString value) {
    hydra::CONFIG_INSTANCE.getSysmodulesPath() =
        stringViewFromHydraString(value);
}

HYDRA_EXPORT bool* hydraConfigGetHandheldMode() {
    return &hydra::CONFIG_INSTANCE.getHandheldMode();
}

HYDRA_EXPORT uint32_t* hydraConfigGetLogOutput() {
    return reinterpret_cast<uint32_t*>(&hydra::CONFIG_INSTANCE.getLogOutput());
}

HYDRA_EXPORT bool* hydraConfigGetLogFsAccess() {
    return &hydra::CONFIG_INSTANCE.getLogFsAccess();
}

HYDRA_EXPORT bool* hydraConfigGetDebugLogging() {
    return &hydra::CONFIG_INSTANCE.getDebugLogging();
}

HYDRA_EXPORT void* hydraConfigGetProcessArgs() {
    return &hydra::CONFIG_INSTANCE.getProcessArgs();
}

HYDRA_EXPORT bool* hydraConfigGetRecoverFromSegfault() {
    return &hydra::CONFIG_INSTANCE.getRecoverFromSegfault();
}

HYDRA_EXPORT bool* hydraConfigGetGdbEnabled() {
    return &hydra::CONFIG_INSTANCE.getGdbEnabled();
}

HYDRA_EXPORT uint16_t* hydraConfigGetGdbPort() {
    return &hydra::CONFIG_INSTANCE.getGdbPort();
}

HYDRA_EXPORT bool* hydraConfigGetGdbWaitForClient() {
    return &hydra::CONFIG_INSTANCE.getGdbWaitForClient();
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

HYDRA_EXPORT HydraString
hydraLoaderPluginOptionConfigGetName(const void* config) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::horizon::loader::plugins::OptionConfig*>(
            config)
            ->name);
}

HYDRA_EXPORT HydraString
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

HYDRA_EXPORT void* hydraOpenFile(HydraString path) {
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

HYDRA_EXPORT HydraString hydraTimeZoneManagerGetLocation(void* manager,
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
HYDRA_EXPORT void* hydraCreateLoaderFromPath(HydraString path,
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
                                         HydraString path) {
    reinterpret_cast<const hydra::horizon::loader::ILoader*>(loader)
        ->extractIcon(stringViewFromHydraString(path));
}

HYDRA_EXPORT bool hydraLoaderHasExefs(const void* loader) {
    return reinterpret_cast<const hydra::horizon::loader::ILoader*>(loader)
        ->hasExeFs();
}

HYDRA_EXPORT void hydraLoaderExtractExefs(const void* loader,
                                          HydraString path) {
    reinterpret_cast<const hydra::horizon::loader::ILoader*>(loader)
        ->extractExeFs(stringViewFromHydraString(path));
}

HYDRA_EXPORT bool hydraLoaderHasRomfs(const void* loader) {
    return reinterpret_cast<const hydra::horizon::loader::ILoader*>(loader)
        ->hasRomFs();
}

HYDRA_EXPORT void hydraLoaderExtractRomfs(const void* loader,
                                          HydraString path) {
    reinterpret_cast<const hydra::horizon::loader::ILoader*>(loader)
        ->extractRomFs(stringViewFromHydraString(path));
}

HYDRA_EXPORT void*
hydraCreateNcaLoaderFromContentArchive(void* content_archive) {
    return new hydra::horizon::loader::NcaLoader(
        *reinterpret_cast<hydra::horizon::filesystem::ContentArchive*>(
            content_archive));
}

HYDRA_EXPORT HydraString hydraNcaLoaderGetName(void* nca_loader) {
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
HYDRA_EXPORT void* hydraCreateLoaderPlugin(HydraString path) {
    // TODO: return the error
    return hydra::horizon::loader::plugins::Plugin::create(
               std::string(stringViewFromHydraString(path)))
        .transform([](hydra::horizon::loader::plugins::Plugin plugin) {
            return new hydra::horizon::loader::plugins::Plugin(
                std::move(plugin));
        })
        .value_or(nullptr);
}

HYDRA_EXPORT void hydraLoaderPluginDestroy(void* plugin) {
    delete reinterpret_cast<hydra::horizon::loader::plugins::Plugin*>(plugin);
}

HYDRA_EXPORT HydraString hydraLoaderPluginGetName(const void* plugin) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::horizon::loader::plugins::Plugin*>(plugin)
            ->getName());
}

HYDRA_EXPORT HydraString
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

HYDRA_EXPORT HydraString
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

HYDRA_EXPORT HydraString hydraNacpGetDisplayVersion(void* nacp) {
    return hydraStringFromStringView(
        reinterpret_cast<
            hydra::horizon::services::ns::ApplicationControlProperty*>(nacp)
            ->display_version);
}

// NACP title
HYDRA_EXPORT HydraString hydraNacpTitleGetName(const void* title) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::horizon::services::ns::ApplicationTitle*>(
            title)
            ->name);
}

HYDRA_EXPORT HydraString hydraNacpTitleGetAuthor(const void* title) {
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

HYDRA_EXPORT HydraU128 hydraUserManagerCreateUser(void* user_manager) {
    return std::bit_cast<HydraU128>(
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

HYDRA_EXPORT HydraU128 hydraUserManagerGetUserId(void* user_manager,
                                                  uint32_t index) {
    return std::bit_cast<HydraU128>(
        reinterpret_cast<
            hydra::horizon::services::account::internal::UserManager*>(
            user_manager)
            ->getUserIDs()[index]);
}

HYDRA_EXPORT void* hydraUserManagerGetUser(void* user_manager,
                                           HydraU128 user_id) {
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
hydraUserManagerLoadAvatarImage(void* user_manager, HydraString path,
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

HYDRA_EXPORT HydraString hydraUserManagerGetAvatarPath(void* user_manager,
                                                        uint32_t index) {
    return hydraStringFromStringView(
        reinterpret_cast<
            hydra::horizon::services::account::internal::UserManager*>(
            user_manager)
            ->getAvatarPath(index));
}

HYDRA_EXPORT HydraString hydraUserGetNickname(void* user) {
    return hydraStringFromStringView(
        reinterpret_cast<hydra::horizon::services::account::internal::User*>(
            user)
            ->getNickname());
}

HYDRA_EXPORT void hydraUserSetNickname(void* user, HydraString nickname) {
    reinterpret_cast<hydra::horizon::services::account::internal::User*>(user)
        ->setNickname(stringViewFromHydraString(nickname));
}

HYDRA_EXPORT HydraUchar3 hydraUserGetAvatarBgColor(void* user) {
    return std::bit_cast<HydraUchar3>(
        reinterpret_cast<hydra::horizon::services::account::internal::User*>(
            user)
            ->getAvatarBgColor());
}

HYDRA_EXPORT void hydraUserSetAvatarBgColor(void* user, HydraUchar3 color) {
    reinterpret_cast<hydra::horizon::services::account::internal::User*>(user)
        ->setAvatarBgColor(std::bit_cast<hydra::uchar3>(color));
}

HYDRA_EXPORT HydraString hydraUserGetAvatarPath(void* user) {
    return hydraStringFromStringView(
        reinterpret_cast<hydra::horizon::services::account::internal::User*>(
            user)
            ->getAvatarPath());
}

HYDRA_EXPORT void hydraUserSetAvatarPath(void* user, HydraString path) {
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
HYDRA_EXPORT HydraString hydraDebuggerGetName(void* debugger) {
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
                                                  HydraString name) {
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
HYDRA_EXPORT HydraString hydraDebuggerThreadGetName(void* thread) {
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

HYDRA_EXPORT HydraString hydraDebuggerThreadGetBreakReason(void* thread) {
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

HYDRA_EXPORT HydraString hydraDebuggerMessageGetFile(const void* msg) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::debugger::Message*>(msg)->log.file);
}

HYDRA_EXPORT uint32_t hydraDebuggerMessageGetLine(const void* msg) {
    return reinterpret_cast<const hydra::debugger::Message*>(msg)->log.line;
}

HYDRA_EXPORT HydraString hydraDebuggerMessageGetFunction(const void* msg) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::debugger::Message*>(msg)->log.function);
}

HYDRA_EXPORT HydraString hydraDebuggerMessageGetString(const void* msg) {
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

HYDRA_EXPORT HydraString
hydraDebuggerResolvedStackFrameGetModule(const void* resolved_stack_frame) {
    return hydraStringFromStringView(
        reinterpret_cast<const hydra::debugger::ResolvedStackFrame*>(
            resolved_stack_frame)
            ->module);
}

HYDRA_EXPORT HydraString
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
