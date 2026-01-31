#include "core/c_api.h"

#include "core/debugger/debugger_manager.hpp"
#include "core/emulation_context.hpp"
#include "core/horizon/filesystem/content_archive.hpp"
#include "core/horizon/filesystem/disk_file.hpp"
#include "core/horizon/firmware.hpp"
#include "core/horizon/loader/nca_loader.hpp"
#include "core/horizon/loader/plugins/manager.hpp"
#include "core/horizon/ui/handler_base.hpp"
#include <string>

#define HYDRA_EXPORT extern "C" __attribute__((visibility("default")))

namespace {

hydra_string hydra_string_from_string_view(std::string_view str) {
    return hydra_string{str.data(), str.size()};
}

std::string_view string_view_from_hydra_string(hydra_string str) {
    return std::string_view(str.data, str.size);
}

} // namespace

// String list
HYDRA_EXPORT void* hydra_create_string_list() {
    return new std::vector<std::string>();
}

HYDRA_EXPORT void hydra_string_list_destroy(void* list) {
    delete reinterpret_cast<std::vector<std::string>*>(list);
}

HYDRA_EXPORT uint32_t hydra_string_list_get_count(const void* list) {
    return static_cast<uint32_t>(
        reinterpret_cast<const std::vector<std::string>*>(list)->size());
}

HYDRA_EXPORT hydra_string hydra_string_list_get(const void* list,
                                                uint32_t index) {
    return hydra_string_from_string_view(
        reinterpret_cast<const std::vector<std::string>*>(list)->at(index));
}

HYDRA_EXPORT void hydra_string_list_resize(void* list, uint32_t size) {
    reinterpret_cast<std::vector<std::string>*>(list)->resize(size);
}

HYDRA_EXPORT void hydra_string_list_set(void* list, uint32_t index,
                                        hydra_string value) {
    (*reinterpret_cast<std::vector<std::string>*>(list))[index] =
        string_view_from_hydra_string(value);
}

HYDRA_EXPORT void hydra_string_list_append(void* list, hydra_string value) {
    reinterpret_cast<std::vector<std::string>*>(list)->push_back(
        std::string(string_view_from_hydra_string(value)));
}

// String view list
HYDRA_EXPORT uint32_t hydra_string_view_list_get_count(const void* list) {
    return static_cast<uint32_t>(
        reinterpret_cast<const std::vector<std::string_view>*>(list)->size());
}

HYDRA_EXPORT hydra_string hydra_string_view_list_get(const void* list,
                                                     uint32_t index) {
    return hydra_string_from_string_view(
        reinterpret_cast<const std::vector<std::string_view>*>(list)->at(
            index));
}

HYDRA_EXPORT void hydra_string_view_list_resize(void* list, uint32_t size) {
    reinterpret_cast<std::vector<std::string_view>*>(list)->resize(size);
}

HYDRA_EXPORT void hydra_string_view_list_set(void* list, uint32_t index,
                                             hydra_string value) {
    (*reinterpret_cast<std::vector<std::string_view>*>(list))[index] =
        string_view_from_hydra_string(value);
}

HYDRA_EXPORT void hydra_string_view_list_append(void* list,
                                                hydra_string value) {
    reinterpret_cast<std::vector<std::string_view>*>(list)->push_back(
        string_view_from_hydra_string(value));
}

// String to string map
HYDRA_EXPORT void* hydra_create_string_to_string_map() {
    return new std::map<std::string, std::string>();
}

HYDRA_EXPORT void hydra_string_to_string_map_destroy(void* map) {
    delete reinterpret_cast<std::map<std::string, std::string>*>(map);
}

HYDRA_EXPORT uint32_t hydra_string_to_string_map_get_count(const void* map) {
    return static_cast<uint32_t>(
        reinterpret_cast<const std::map<std::string, std::string>*>(map)
            ->size());
}

HYDRA_EXPORT hydra_string hydra_string_to_string_map_get_key(const void* map,
                                                             uint32_t index) {
    auto it = reinterpret_cast<const std::map<std::string, std::string>*>(map)
                  ->begin();
    std::advance(it, index);
    return hydra_string_from_string_view(it->first);
}

HYDRA_EXPORT hydra_string hydra_string_to_string_map_get_value(const void* map,
                                                               uint32_t index) {
    auto it = reinterpret_cast<const std::map<std::string, std::string>*>(map)
                  ->begin();
    std::advance(it, index);
    return hydra_string_from_string_view(it->second);
}

HYDRA_EXPORT hydra_string
hydra_string_to_string_map_get_value_by_key(const void* map, hydra_string key) {
    return hydra_string_from_string_view(
        reinterpret_cast<const std::map<std::string, std::string>*>(map)->at(
            std::string(string_view_from_hydra_string(key))));
}

HYDRA_EXPORT void hydra_string_to_string_map_remove_all(void* map) {
    reinterpret_cast<std::map<std::string, std::string>*>(map)->clear();
}

HYDRA_EXPORT void hydra_string_to_string_map_set_by_key(void* map,
                                                        hydra_string key,
                                                        hydra_string value) {
    (*reinterpret_cast<std::map<std::string, std::string>*>(
        map))[std::string(string_view_from_hydra_string(key))] =
        string_view_from_hydra_string(value);
}

// Loader plugin
HYDRA_EXPORT hydra_string hydra_loader_plugin_get_path(const void* plugin) {
    return hydra_string_from_string_view(
        reinterpret_cast<const hydra::LoaderPlugin*>(plugin)->path);
}

HYDRA_EXPORT void hydra_loader_plugin_set_path(void* plugin,
                                               hydra_string path) {
    reinterpret_cast<hydra::LoaderPlugin*>(plugin)->path =
        string_view_from_hydra_string(path);
}

HYDRA_EXPORT void* hydra_loader_plugin_get_options(void* plugin) {
    return &reinterpret_cast<hydra::LoaderPlugin*>(plugin)->options;
}

HYDRA_EXPORT uint32_t hydra_loader_plugin_list_get_count(const void* list) {
    return static_cast<uint32_t>(
        reinterpret_cast<const std::vector<hydra::LoaderPlugin>*>(list)
            ->size());
}

HYDRA_EXPORT void* hydra_loader_plugin_list_get(void* list, uint32_t index) {
    return &reinterpret_cast<std::vector<hydra::LoaderPlugin>*>(list)->at(
        index);
}

HYDRA_EXPORT void hydra_loader_plugin_list_resize(void* list, uint32_t size) {
    reinterpret_cast<std::vector<hydra::LoaderPlugin>*>(list)->resize(size);
}

// Config
HYDRA_EXPORT void hydra_config_serialize() {
    hydra::CONFIG_INSTANCE.Serialize();
}

HYDRA_EXPORT void hydra_config_deserialize() {
    hydra::CONFIG_INSTANCE.Deserialize();
}

HYDRA_EXPORT hydra_string hydra_config_get_app_data_path() {
    return hydra_string_from_string_view(
        hydra::CONFIG_INSTANCE.GetAppDataPath());
}

HYDRA_EXPORT void* hydra_config_get_game_paths() {
    return &hydra::CONFIG_INSTANCE.GetGamePaths();
}

HYDRA_EXPORT void* hydra_config_get_loader_plugins() {
    return &hydra::CONFIG_INSTANCE.GetLoaderPlugins();
}

HYDRA_EXPORT void* hydra_config_get_patch_paths() {
    return &hydra::CONFIG_INSTANCE.GetPatchPaths();
}

HYDRA_EXPORT void* hydra_config_get_input_profiles() {
    return &hydra::CONFIG_INSTANCE.GetInputProfiles();
}

HYDRA_EXPORT uint32_t* hydra_config_get_cpu_backend() {
    return reinterpret_cast<uint32_t*>(&hydra::CONFIG_INSTANCE.GetCpuBackend());
}

HYDRA_EXPORT uint32_t* hydra_config_get_gpu_renderer() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.GetGpuRenderer());
}

HYDRA_EXPORT uint32_t* hydra_config_get_shader_backend() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.GetShaderBackend());
}

HYDRA_EXPORT uint32_t* hydra_config_get_display_resolution() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.GetDisplayResolution());
}

HYDRA_EXPORT hydra_uint2* hydra_config_get_custom_display_resolution() {
    return reinterpret_cast<hydra_uint2*>(
        &hydra::CONFIG_INSTANCE.GetCustomDisplayResolution());
}

HYDRA_EXPORT uint32_t* hydra_config_get_audio_backend() {
    return reinterpret_cast<uint32_t*>(
        &hydra::CONFIG_INSTANCE.GetAudioBackend());
}

HYDRA_EXPORT hydra_u128* hydra_config_get_user_id() {
    return reinterpret_cast<hydra_u128*>(&hydra::CONFIG_INSTANCE.GetUserId());
}

HYDRA_EXPORT hydra_string hydra_config_get_firmware_path() {
    return hydra_string_from_string_view(
        hydra::CONFIG_INSTANCE.GetFirmwarePath());
}

HYDRA_EXPORT void hydra_config_set_firmware_path(hydra_string value) {
    hydra::CONFIG_INSTANCE.GetFirmwarePath() =
        string_view_from_hydra_string(value);
}

HYDRA_EXPORT hydra_string hydra_config_get_sd_card_path() {
    return hydra_string_from_string_view(
        hydra::CONFIG_INSTANCE.GetSdCardPath());
}

HYDRA_EXPORT void hydra_config_set_sd_card_path(hydra_string value) {
    hydra::CONFIG_INSTANCE.GetSdCardPath() =
        string_view_from_hydra_string(value);
}

HYDRA_EXPORT hydra_string hydra_config_get_save_path() {
    return hydra_string_from_string_view(hydra::CONFIG_INSTANCE.GetSavePath());
}

HYDRA_EXPORT void hydra_config_set_save_path(hydra_string value) {
    hydra::CONFIG_INSTANCE.GetSavePath() = string_view_from_hydra_string(value);
}

HYDRA_EXPORT hydra_string hydra_config_get_sysmodules_path() {
    return hydra_string_from_string_view(
        hydra::CONFIG_INSTANCE.GetSysmodulesPath());
}

HYDRA_EXPORT void hydra_config_set_sysmodules_path(hydra_string value) {
    hydra::CONFIG_INSTANCE.GetSysmodulesPath() =
        string_view_from_hydra_string(value);
}

HYDRA_EXPORT bool* hydra_config_get_handheld_mode() {
    return &hydra::CONFIG_INSTANCE.GetHandheldMode();
}

HYDRA_EXPORT uint32_t* hydra_config_get_log_output() {
    return reinterpret_cast<uint32_t*>(&hydra::CONFIG_INSTANCE.GetLogOutput());
}

HYDRA_EXPORT bool* hydra_config_get_log_fs_access() {
    return &hydra::CONFIG_INSTANCE.GetLogFsAccess();
}

HYDRA_EXPORT bool* hydra_config_get_debug_logging() {
    return &hydra::CONFIG_INSTANCE.GetDebugLogging();
}

HYDRA_EXPORT void* hydra_config_get_process_args() {
    return &hydra::CONFIG_INSTANCE.GetProcessArgs();
}

HYDRA_EXPORT bool* hydra_config_get_recover_from_segfault() {
    return &hydra::CONFIG_INSTANCE.GetRecoverFromSegfault();
}

HYDRA_EXPORT bool* hydra_config_get_gdb_enabled() {
    return &hydra::CONFIG_INSTANCE.GetGdbEnabled();
}

HYDRA_EXPORT uint16_t* hydra_config_get_gdb_port() {
    return &hydra::CONFIG_INSTANCE.GetGdbPort();
}

HYDRA_EXPORT bool* hydra_config_get_gdb_wait_for_client() {
    return &hydra::CONFIG_INSTANCE.GetGdbWaitForClient();
}

// Loader plugins

// Manager
HYDRA_EXPORT void hydra_loader_plugin_manager_refresh() {
    hydra::horizon::loader::plugins::Manager::GetInstance().Refresh();
}

// Plugin
HYDRA_EXPORT void* hydra_create_loader_plugin(hydra_string path) {
    try {
        return new hydra::horizon::loader::plugins::Plugin(
            std::string(string_view_from_hydra_string(path)));
    } catch (...) {
        // TODO: return an error
        return nullptr;
    }
}

HYDRA_EXPORT void hydra_loader_plugin_destroy(void* plugin) {
    delete reinterpret_cast<hydra::horizon::loader::plugins::Plugin*>(plugin);
}

HYDRA_EXPORT hydra_string hydra_loader_plugin_get_name(const void* plugin) {
    return hydra_string_from_string_view(
        reinterpret_cast<const hydra::horizon::loader::plugins::Plugin*>(plugin)
            ->GetName());
}

HYDRA_EXPORT hydra_string
hydra_loader_plugin_get_display_version(const void* plugin) {
    return hydra_string_from_string_view(
        reinterpret_cast<const hydra::horizon::loader::plugins::Plugin*>(plugin)
            ->GetDisplayVersion());
}

HYDRA_EXPORT uint32_t
hydra_loader_plugin_get_supported_format_count(const void* plugin) {
    return static_cast<uint32_t>(
        reinterpret_cast<const hydra::horizon::loader::plugins::Plugin*>(plugin)
            ->GetSupportedFormats()
            .size());
}

HYDRA_EXPORT hydra_string
hydra_loader_plugin_get_supported_format(const void* plugin, uint32_t index) {
    return hydra_string_from_string_view(
        reinterpret_cast<const hydra::horizon::loader::plugins::Plugin*>(plugin)
            ->GetSupportedFormats()[index]);
}

HYDRA_EXPORT uint32_t
hydra_loader_plugin_get_option_config_count(const void* plugin) {
    return static_cast<uint32_t>(
        reinterpret_cast<const hydra::horizon::loader::plugins::Plugin*>(plugin)
            ->GetOptionConfigs()
            .size());
}

HYDRA_EXPORT const void*
hydra_loader_plugin_get_option_config(const void* plugin, uint32_t index) {
    return &reinterpret_cast<const hydra::horizon::loader::plugins::Plugin*>(
                plugin)
                ->GetOptionConfigs()[index];
}

// Option config
HYDRA_EXPORT void* hydra_loader_plugin_option_config_copy(const void* config) {
    return new hydra::horizon::loader::plugins::OptionConfig(
        *reinterpret_cast<const hydra::horizon::loader::plugins::OptionConfig*>(
            config));
}

HYDRA_EXPORT void hydra_loader_plugin_option_config_destroy(void* config) {
    delete reinterpret_cast<hydra::horizon::loader::plugins::OptionConfig*>(
        config);
}

HYDRA_EXPORT hydra_string
hydra_loader_plugin_option_config_get_name(const void* config) {
    return hydra_string_from_string_view(
        reinterpret_cast<const hydra::horizon::loader::plugins::OptionConfig*>(
            config)
            ->name);
}

HYDRA_EXPORT hydra_string
hydra_loader_plugin_option_config_get_description(const void* config) {
    return hydra_string_from_string_view(
        reinterpret_cast<const hydra::horizon::loader::plugins::OptionConfig*>(
            config)
            ->description);
}

HYDRA_EXPORT HydraLoaderPluginOptionType
hydra_loader_plugin_option_config_get_type(const void* config) {
    return static_cast<HydraLoaderPluginOptionType>(
        reinterpret_cast<const hydra::horizon::loader::plugins::OptionConfig*>(
            config)
            ->type);
}

HYDRA_EXPORT bool
hydra_loader_plugin_option_config_get_is_required(const void* config) {
    return reinterpret_cast<
               const hydra::horizon::loader::plugins::OptionConfig*>(config)
        ->is_required;
}

HYDRA_EXPORT const void*
hydra_loader_plugin_option_config_get_enum_value_names(const void* config) {
    return &reinterpret_cast<
                const hydra::horizon::loader::plugins::OptionConfig*>(config)
                ->enum_value_names;
}

HYDRA_EXPORT const void*
hydra_loader_plugin_option_config_get_path_content_types(const void* config) {
    return &reinterpret_cast<
                const hydra::horizon::loader::plugins::OptionConfig*>(config)
                ->path_content_types;
}

// Filesystem
HYDRA_EXPORT void* hydra_create_filesystem() {
    return new hydra::horizon::filesystem::Filesystem();
}

HYDRA_EXPORT void hydra_filesystem_destroy(void* fs) {
    delete reinterpret_cast<hydra::horizon::filesystem::Filesystem*>(fs);
}

HYDRA_EXPORT void hydra_try_install_firmware_to_filesystem(void* fs) {
    hydra::horizon::try_install_firmware_to_filesystem(
        *reinterpret_cast<hydra::horizon::filesystem::Filesystem*>(fs));
}

HYDRA_EXPORT void* hydra_open_file(hydra_string path) {
    return new hydra::horizon::filesystem::DiskFile(
        string_view_from_hydra_string(path));
}

HYDRA_EXPORT void hydra_file_close(void* file) {
    delete reinterpret_cast<hydra::horizon::filesystem::DiskFile*>(file);
}

HYDRA_EXPORT void* hydra_create_content_archive(void* file) {
    return new hydra::horizon::filesystem::ContentArchive(
        reinterpret_cast<hydra::horizon::filesystem::IFile*>(file));
}

HYDRA_EXPORT void hydra_content_archive_destroy(void* content_archive) {
    delete reinterpret_cast<hydra::horizon::filesystem::ContentArchive*>(
        content_archive);
}

HYDRA_EXPORT HydraContentArchiveContentType
hydra_content_archive_get_content_type(void* content_archive) {
    return static_cast<HydraContentArchiveContentType>(
        reinterpret_cast<hydra::horizon::filesystem::ContentArchive*>(
            content_archive)
            ->GetContentType());
}

// Loader
HYDRA_EXPORT void* hydra_create_loader_from_path(hydra_string path) {
    try {
        return hydra::horizon::loader::LoaderBase::CreateFromPath(
            string_view_from_hydra_string(path));
    } catch (...) {
        // TODO: return an error
        return nullptr;
    }
}

HYDRA_EXPORT void hydra_loader_destroy(void* loader) {
    delete reinterpret_cast<hydra::horizon::loader::LoaderBase*>(loader);
}

HYDRA_EXPORT uint64_t hydra_loader_get_title_id(void* loader) {
    return reinterpret_cast<hydra::horizon::loader::LoaderBase*>(loader)
        ->GetTitleID();
}

HYDRA_EXPORT void* hydra_loader_load_nacp(void* loader) {
    return reinterpret_cast<hydra::horizon::loader::LoaderBase*>(loader)
        ->LoadNacp();
}

HYDRA_EXPORT void* hydra_loader_load_icon(void* loader, uint32_t* width,
                                          uint32_t* height) {
    return reinterpret_cast<hydra::horizon::loader::LoaderBase*>(loader)
        ->LoadIcon(*width, *height);
}

HYDRA_EXPORT bool hydra_loader_has_icon(const void* loader) {
    return reinterpret_cast<const hydra::horizon::loader::LoaderBase*>(loader)
        ->HasIcon();
}

HYDRA_EXPORT void hydra_loader_extract_icon(const void* loader,
                                            hydra_string path) {
    reinterpret_cast<const hydra::horizon::loader::LoaderBase*>(loader)
        ->ExtractIcon(string_view_from_hydra_string(path));
}

HYDRA_EXPORT bool hydra_loader_has_exefs(const void* loader) {
    return reinterpret_cast<const hydra::horizon::loader::LoaderBase*>(loader)
        ->HasExeFs();
}

HYDRA_EXPORT void hydra_loader_extract_exefs(const void* loader,
                                             hydra_string path) {
    reinterpret_cast<const hydra::horizon::loader::LoaderBase*>(loader)
        ->ExtractExeFs(string_view_from_hydra_string(path));
}

HYDRA_EXPORT bool hydra_loader_has_romfs(const void* loader) {
    return reinterpret_cast<const hydra::horizon::loader::LoaderBase*>(loader)
        ->HasRomFs();
}

HYDRA_EXPORT void hydra_loader_extract_romfs(const void* loader,
                                             hydra_string path) {
    reinterpret_cast<const hydra::horizon::loader::LoaderBase*>(loader)
        ->ExtractRomFs(string_view_from_hydra_string(path));
}

HYDRA_EXPORT void*
hydra_create_nca_loader_from_content_archive(void* content_archive) {
    return new hydra::horizon::loader::NcaLoader(
        *reinterpret_cast<hydra::horizon::filesystem::ContentArchive*>(
            content_archive));
}

HYDRA_EXPORT hydra_string hydra_nca_loader_get_name(void* nca_loader) {
    return hydra_string_from_string_view(
        reinterpret_cast<hydra::horizon::loader::NcaLoader*>(nca_loader)
            ->GetName());
}

// NACP
HYDRA_EXPORT void hydra_nacp_destroy(void* nacp) {
    delete reinterpret_cast<
        hydra::horizon::services::ns::ApplicationControlProperty*>(nacp);
}

HYDRA_EXPORT const void* hydra_nacp_get_title(void* nacp) {
    return &reinterpret_cast<
                hydra::horizon::services::ns::ApplicationControlProperty*>(nacp)
                ->GetApplicationTitle();
}

HYDRA_EXPORT hydra_string hydra_nacp_get_display_version(void* nacp) {
    return hydra_string_from_string_view(
        reinterpret_cast<
            hydra::horizon::services::ns::ApplicationControlProperty*>(nacp)
            ->display_version);
}

// NACP title
HYDRA_EXPORT hydra_string hydra_nacp_title_get_name(const void* title) {
    return hydra_string_from_string_view(
        reinterpret_cast<const hydra::horizon::services::ns::ApplicationTitle*>(
            title)
            ->name);
}

HYDRA_EXPORT hydra_string hydra_nacp_title_get_author(const void* title) {
    return hydra_string_from_string_view(
        reinterpret_cast<const hydra::horizon::services::ns::ApplicationTitle*>(
            title)
            ->author);
}

// User manager
HYDRA_EXPORT void* hydra_create_user_manager() {
    return new hydra::horizon::services::account::internal::UserManager();
}

HYDRA_EXPORT void hydra_user_manager_destroy(void* user_manager) {
    delete reinterpret_cast<
        hydra::horizon::services::account::internal::UserManager*>(
        user_manager);
}

HYDRA_EXPORT void hydra_user_manager_flush(void* user_manager) {
    reinterpret_cast<hydra::horizon::services::account::internal::UserManager*>(
        user_manager)
        ->Flush();
}

HYDRA_EXPORT hydra_u128 hydra_user_manager_create_user(void* user_manager) {
    return std::bit_cast<hydra_u128>(
        reinterpret_cast<
            hydra::horizon::services::account::internal::UserManager*>(
            user_manager)
            ->CreateUser());
}

HYDRA_EXPORT uint32_t hydra_user_manager_get_user_count(void* user_manager) {
    return static_cast<uint32_t>(
        reinterpret_cast<
            hydra::horizon::services::account::internal::UserManager*>(
            user_manager)
            ->GetUserCount());
}

HYDRA_EXPORT hydra_u128 hydra_user_manager_get_user_id(void* user_manager,
                                                       uint32_t index) {
    return std::bit_cast<hydra_u128>(
        reinterpret_cast<
            hydra::horizon::services::account::internal::UserManager*>(
            user_manager)
            ->GetUserIDs()[index]);
}

HYDRA_EXPORT void* hydra_user_manager_get_user(void* user_manager,
                                               hydra_u128 user_id) {
    return &reinterpret_cast<
                hydra::horizon::services::account::internal::UserManager*>(
                user_manager)
                ->GetUser(std::bit_cast<hydra::u128>(user_id));
}

HYDRA_EXPORT void hydra_user_manager_load_system_avatars(void* user_manager,
                                                         void* fs) {
    reinterpret_cast<hydra::horizon::services::account::internal::UserManager*>(
        user_manager)
        ->LoadSystemAvatars(
            *reinterpret_cast<hydra::horizon::filesystem::Filesystem*>(fs));
}

HYDRA_EXPORT const void*
hydra_user_manager_load_avatar_image(void* user_manager, hydra_string path,
                                     uint32_t* out_dimensions) {
    return reinterpret_cast<
               hydra::horizon::services::account::internal::UserManager*>(
               user_manager)
        ->LoadAvatarImage(string_view_from_hydra_string(path), *out_dimensions)
        .data();
}

HYDRA_EXPORT uint32_t hydra_user_manager_get_avatar_count(void* user_manager) {
    return static_cast<uint32_t>(
        reinterpret_cast<
            hydra::horizon::services::account::internal::UserManager*>(
            user_manager)
            ->GetAvatars()
            .size());
}

HYDRA_EXPORT hydra_string hydra_user_manager_get_avatar_path(void* user_manager,
                                                             uint32_t index) {
    return hydra_string_from_string_view(
        reinterpret_cast<
            hydra::horizon::services::account::internal::UserManager*>(
            user_manager)
            ->GetAvatarPath(index));
}

HYDRA_EXPORT hydra_string hydra_user_get_nickname(void* user) {
    return hydra_string_from_string_view(
        reinterpret_cast<hydra::horizon::services::account::internal::User*>(
            user)
            ->GetNickname());
}

HYDRA_EXPORT void hydra_user_set_nickname(void* user, hydra_string nickname) {
    reinterpret_cast<hydra::horizon::services::account::internal::User*>(user)
        ->SetNickname(string_view_from_hydra_string(nickname));
}

HYDRA_EXPORT hydra_uchar3 hydra_user_get_avatar_bg_color(void* user) {
    return std::bit_cast<hydra_uchar3>(
        reinterpret_cast<hydra::horizon::services::account::internal::User*>(
            user)
            ->GetAvatarBgColor());
}

HYDRA_EXPORT void hydra_user_set_avatar_bg_color(void* user,
                                                 hydra_uchar3 color) {
    reinterpret_cast<hydra::horizon::services::account::internal::User*>(user)
        ->SetAvatarBgColor(std::bit_cast<hydra::uchar3>(color));
}

HYDRA_EXPORT hydra_string hydra_user_get_avatar_path(void* user) {
    return hydra_string_from_string_view(
        reinterpret_cast<hydra::horizon::services::account::internal::User*>(
            user)
            ->GetAvatarPath());
}

HYDRA_EXPORT void hydra_user_set_avatar_path(void* user, hydra_string path) {
    reinterpret_cast<hydra::horizon::services::account::internal::User*>(user)
        ->SetAvatarPath(string_view_from_hydra_string(path));
}

// Emulation context
// TODO: proper UI handler
class UIHandler : public hydra::horizon::ui::HandlerBase {
  public:
    void ShowMessageDialog(const hydra::horizon::ui::MessageDialogType type,
                           const std::string& title,
                           const std::string& message) override {
        (void)type;
        (void)title;
        (void)message;
    }
    hydra::horizon::applets::software_keyboard::SoftwareKeyboardResult
    ShowSoftwareKeyboard(const std::string& header_text,
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

HYDRA_EXPORT void* hydra_create_emulation_context() {
    return new hydra::EmulationContext(*(new UIHandler()));
}

HYDRA_EXPORT void hydra_emulation_context_destroy(void* ctx) {
    // TODO: also destroy the UI handler
    delete reinterpret_cast<hydra::EmulationContext*>(ctx);
}

HYDRA_EXPORT void hydra_emulation_context_set_surface(void* ctx,
                                                      void* surface) {
    reinterpret_cast<hydra::EmulationContext*>(ctx)->SetSurface(surface);
}

HYDRA_EXPORT void hydra_emulation_context_load_and_start(void* ctx,
                                                         void* loader) {
    reinterpret_cast<hydra::EmulationContext*>(ctx)->LoadAndStart(
        reinterpret_cast<hydra::horizon::loader::LoaderBase*>(loader));
}

HYDRA_EXPORT void hydra_emulation_context_request_stop(void* ctx) {
    reinterpret_cast<hydra::EmulationContext*>(ctx)->RequestStop();
}

HYDRA_EXPORT void hydra_emulation_context_force_stop(void* ctx) {
    reinterpret_cast<hydra::EmulationContext*>(ctx)->ForceStop();
}

HYDRA_EXPORT void hydra_emulation_context_pause(void* ctx) {
    reinterpret_cast<hydra::EmulationContext*>(ctx)->Pause();
}

HYDRA_EXPORT void hydra_emulation_context_resume(void* ctx) {
    reinterpret_cast<hydra::EmulationContext*>(ctx)->Resume();
}

HYDRA_EXPORT void
hydra_emulation_context_notify_operation_mode_changed(void* ctx) {
    reinterpret_cast<hydra::EmulationContext*>(ctx)
        ->NotifyOperationModeChanged();
}

HYDRA_EXPORT void hydra_emulation_context_progress_frame(
    void* ctx, uint32_t width, uint32_t height, bool* out_dt_average_updated) {
    reinterpret_cast<hydra::EmulationContext*>(ctx)->ProgressFrame(
        width, height, *out_dt_average_updated);
}

HYDRA_EXPORT bool hydra_emulation_context_is_running(void* ctx) {
    return reinterpret_cast<hydra::EmulationContext*>(ctx)->IsRunning();
}

HYDRA_EXPORT float
hydra_emulation_context_get_last_delta_time_average(void* ctx) {
    return reinterpret_cast<hydra::EmulationContext*>(ctx)
        ->GetLastDeltaTimeAverage();
}

HYDRA_EXPORT void hydra_emulation_context_take_screenshot(void* ctx) {
    reinterpret_cast<hydra::EmulationContext*>(ctx)->TakeScreenshot();
}

HYDRA_EXPORT void hydra_emulation_context_capture_gpu_frame(void* ctx) {
    reinterpret_cast<hydra::EmulationContext*>(ctx)->CaptureGpuFrame();
}

// Debugger

// Debugger manager
HYDRA_EXPORT void hydra_debugger_manager_lock() {
    hydra::DEBUGGER_MANAGER_INSTANCE.Lock();
}

HYDRA_EXPORT void hydra_debugger_manager_unlock() {
    hydra::DEBUGGER_MANAGER_INSTANCE.Unlock();
}

HYDRA_EXPORT uint64_t hydra_debugger_manager_get_debugger_count() {
    return hydra::DEBUGGER_MANAGER_INSTANCE.GetDebuggerCount();
}

HYDRA_EXPORT void* hydra_debugger_manager_get_debugger(uint32_t index) {
    return &hydra::DEBUGGER_MANAGER_INSTANCE.GetDebugger(index);
}

HYDRA_EXPORT void*
hydra_debugger_manager_get_debugger_for_process(void* process) {
    return &hydra::DEBUGGER_MANAGER_INSTANCE.GetDebugger(
        reinterpret_cast<hydra::horizon::kernel::Process*>(process));
}

// Debugger
HYDRA_EXPORT hydra_string hydra_debugger_get_name(void* debugger) {
    return hydra_string_from_string_view(
        reinterpret_cast<hydra::debugger::Debugger*>(debugger)->GetName());
}

HYDRA_EXPORT void hydra_debugger_lock(void* debugger) {
    reinterpret_cast<hydra::debugger::Debugger*>(debugger)->Lock();
}

HYDRA_EXPORT void hydra_debugger_unlock(void* debugger) {
    reinterpret_cast<hydra::debugger::Debugger*>(debugger)->Unlock();
}

HYDRA_EXPORT void hydra_debugger_register_this_thread(void* debugger,
                                                      hydra_string name) {
    reinterpret_cast<hydra::debugger::Debugger*>(debugger)->RegisterThisThread(
        string_view_from_hydra_string(name));
}

HYDRA_EXPORT void hydra_debugger_unregister_this_thread(void* debugger) {
    reinterpret_cast<hydra::debugger::Debugger*>(debugger)
        ->UnregisterThisThread();
}

HYDRA_EXPORT uint64_t hydra_debugger_get_thread_count(void* debugger) {
    return reinterpret_cast<hydra::debugger::Debugger*>(debugger)
        ->GetThreadCount();
}

HYDRA_EXPORT void* hydra_debugger_get_thread(void* debugger, uint32_t index) {
    return &reinterpret_cast<hydra::debugger::Debugger*>(debugger)->GetThread(
        index);
}

// Thread
HYDRA_EXPORT hydra_string hydra_debugger_thread_get_name(void* thread) {
    return hydra_string_from_string_view(
        reinterpret_cast<hydra::debugger::Thread*>(thread)->GetName());
}

HYDRA_EXPORT void hydra_debugger_thread_lock(void* thread) {
    reinterpret_cast<hydra::debugger::Thread*>(thread)->Lock();
}

HYDRA_EXPORT void hydra_debugger_thread_unlock(void* thread) {
    reinterpret_cast<hydra::debugger::Thread*>(thread)->Unlock();
}

HYDRA_EXPORT HydraDebuggerThreadStatus
hydra_debugger_thread_get_status(void* thread) {
    return static_cast<HydraDebuggerThreadStatus>(
        reinterpret_cast<hydra::debugger::Thread*>(thread)->GetStatus());
}

HYDRA_EXPORT hydra_string hydra_debugger_thread_get_break_reason(void* thread) {
    return hydra_string_from_string_view(
        reinterpret_cast<hydra::debugger::Thread*>(thread)->GetBreakReason());
}

HYDRA_EXPORT uint64_t hydra_debugger_thread_get_message_count(void* thread) {
    return reinterpret_cast<hydra::debugger::Thread*>(thread)
        ->GetMessageCount();
}

HYDRA_EXPORT const void* hydra_debugger_thread_get_message(void* thread,
                                                           uint32_t index) {
    return &reinterpret_cast<hydra::debugger::Thread*>(thread)->GetMessage(
        index);
}

// Message
HYDRA_EXPORT HydraLogLevel
hydra_debugger_message_get_log_level(const void* msg) {
    return static_cast<HydraLogLevel>(
        reinterpret_cast<const hydra::debugger::Message*>(msg)->log.level);
}

HYDRA_EXPORT HydraLogClass
hydra_debugger_message_get_log_class(const void* msg) {
    return static_cast<HydraLogClass>(
        reinterpret_cast<const hydra::debugger::Message*>(msg)->log.c);
}

HYDRA_EXPORT hydra_string hydra_debugger_message_get_file(const void* msg) {
    return hydra_string_from_string_view(
        reinterpret_cast<const hydra::debugger::Message*>(msg)->log.file);
}

HYDRA_EXPORT uint32_t hydra_debugger_message_get_line(const void* msg) {
    return reinterpret_cast<const hydra::debugger::Message*>(msg)->log.line;
}

HYDRA_EXPORT hydra_string hydra_debugger_message_get_function(const void* msg) {
    return hydra_string_from_string_view(
        reinterpret_cast<const hydra::debugger::Message*>(msg)->log.function);
}

HYDRA_EXPORT hydra_string hydra_debugger_message_get_string(const void* msg) {
    return hydra_string_from_string_view(
        reinterpret_cast<const hydra::debugger::Message*>(msg)->log.str);
}

HYDRA_EXPORT const void*
hydra_debugger_message_get_stack_trace(const void* msg) {
    return &reinterpret_cast<const hydra::debugger::Message*>(msg)->stack_trace;
}

// Stack trace
HYDRA_EXPORT void* hydra_debugger_stack_trace_copy(const void* stack_trace) {
    return new hydra::debugger::StackTrace(
        *reinterpret_cast<const hydra::debugger::StackTrace*>(stack_trace));
}

HYDRA_EXPORT void hydra_debugger_stack_trace_destroy(void* stack_trace) {
    delete reinterpret_cast<hydra::debugger::StackTrace*>(stack_trace);
}

HYDRA_EXPORT uint32_t
hydra_debugger_stack_trace_get_frame_count(const void* stack_trace) {
    return static_cast<uint32_t>(
        reinterpret_cast<const hydra::debugger::StackTrace*>(stack_trace)
            ->frames.size());
}

HYDRA_EXPORT const void*
hydra_debugger_stack_trace_get_frame(const void* stack_trace, uint32_t index) {
    return &reinterpret_cast<const hydra::debugger::StackTrace*>(stack_trace)
                ->frames[index];
}

// Stack frame
HYDRA_EXPORT void* hydra_debugger_stack_frame_resolve(const void* stack_frame) {
    return new hydra::debugger::ResolvedStackFrame(
        reinterpret_cast<const hydra::debugger::StackFrame*>(stack_frame)
            ->Resolve());
}

// Resolved stack frame
HYDRA_EXPORT void
hydra_debugger_resolved_stack_frame_destroy(void* resolved_stack_frame) {
    delete reinterpret_cast<hydra::debugger::ResolvedStackFrame*>(
        resolved_stack_frame);
}

HYDRA_EXPORT hydra_string hydra_debugger_resolved_stack_frame_get_module(
    const void* resolved_stack_frame) {
    return hydra_string_from_string_view(
        reinterpret_cast<const hydra::debugger::ResolvedStackFrame*>(
            resolved_stack_frame)
            ->module);
}

HYDRA_EXPORT hydra_string hydra_debugger_resolved_stack_frame_get_function(
    const void* resolved_stack_frame) {
    return hydra_string_from_string_view(
        reinterpret_cast<const hydra::debugger::ResolvedStackFrame*>(
            resolved_stack_frame)
            ->function);
}

HYDRA_EXPORT uint64_t hydra_debugger_resolved_stack_frame_get_address(
    const void* resolved_stack_frame) {
    return reinterpret_cast<const hydra::debugger::ResolvedStackFrame*>(
               resolved_stack_frame)
        ->addr;
}
