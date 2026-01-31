#ifndef HYDRA_C_API
#define HYDRA_C_API

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Types
typedef struct {
    uint64_t lo;
    uint64_t hi;
} hydra_u128;

typedef struct {
    uint32_t x;
    uint32_t y;
} hydra_uint2;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t z;
} hydra_uchar3;

typedef struct {
    const char* data;
    size_t size;
} hydra_string;

// Enums
typedef enum : uint32_t {
    HYDRA_CPU_BACKEND_INVALID = 0,

    HYDRA_CPU_BACKEND_APPLE_HYPERVISOR,
    HYDRA_CPU_BACKEND_DYNARMIC,
} HydraCpuBackend;

typedef enum : uint32_t {
    HYDRA_GPU_RENDERER_INVALID = 0,

    HYDRA_GPU_RENDERER_METAL,
} HydraGpuRenderer;

typedef enum : uint32_t {
    HYDRA_SHADER_BACKEND_INVALID = 0,

    HYDRA_SHADER_BACKEND_MSL,
    HYDRA_SHADER_BACKEND_AIR,
} HydraShaderBackend;

typedef enum : uint32_t {
    HYDRA_RESOLUTION_INVALID = 0,

    HYDRA_RESOLUTION_AUTO,
    HYDRA_RESOLUTION_720P,
    HYDRA_RESOLUTION_1080P,
    HYDRA_RESOLUTION_1440P,
    HYDRA_RESOLUTION_2160P,
    HYDRA_RESOLUTION_4320P,
    HYDRA_RESOLUTION_AUTO_EXACT,
    HYDRA_RESOLUTION_CUSTOM,
} HydraResolution;

typedef enum : uint32_t {
    HYDRA_AUDIO_BACKEND_INVALID = 0,

    HYDRA_AUDIO_BACKEND_NULL,
    HYDRA_AUDIO_BACKEND_CUBEB,
} HydraAudioBackend;

typedef enum : uint32_t {
    HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_PROGRAM = 0,
    HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_META = 1,
    HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_CONTROL = 2,
    HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_MANUAL = 3,
    HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_DATA = 4,
    HYDRA_CONTENT_ARCHIVE_CONTENT_TYPE_PUBLIC_DATA = 5,
} HydraContentArchiveContentType;

typedef enum : uint32_t {
    HYDRA_LOG_OUTPUT_INVALID = 0,

    HYDRA_LOG_OUTPUT_NONE,
    HYDRA_LOG_OUTPUT_STD_OUT,
    HYDRA_LOG_OUTPUT_FILE,
} HydraLogOutput;

typedef enum : uint32_t {
    HYDRA_LOG_LEVEL_DEBUG,
    HYDRA_LOG_LEVEL_INFO,
    HYDRA_LOG_LEVEL_STUB,
    HYDRA_LOG_LEVEL_WARNING,
    HYDRA_LOG_LEVEL_ERROR,
    HYDRA_LOG_LEVEL_FATAL,
} HydraLogLevel;

typedef enum : uint32_t {
    HYDRA_LOG_CLASS_COMMON,
    HYDRA_LOG_CLASS_MMU,
    HYDRA_LOG_CLASS_CPU,
    HYDRA_LOG_CLASS_GPU,
    HYDRA_LOG_CLASS_ENGINES,
    HYDRA_LOG_CLASS_MACRO,
    HYDRA_LOG_CLASS_SHADER_DECOMPILER,
    HYDRA_LOG_CLASS_METAL_RENDERER,
    HYDRA_LOG_CLASS_SDL3_WINDOW,
    HYDRA_LOG_CLASS_HORIZON,
    HYDRA_LOG_CLASS_KERNEL,
    HYDRA_LOG_CLASS_FILESYSTEM,
    HYDRA_LOG_CLASS_LOADER,
    HYDRA_LOG_CLASS_SERVICES,
    HYDRA_LOG_CLASS_APPLETS,
    HYDRA_LOG_CLASS_CUBEB,
    HYDRA_LOG_CLASS_HYPERVISOR,
    HYDRA_LOG_CLASS_DYNARMIC,
    HYDRA_LOG_CLASS_INPUT,
    HYDRA_LOG_CLASS_OTHER,
} HydraLogClass;

typedef enum : uint32_t {
    HYDRA_DEBUGGER_THREAD_STATUS_RUNNING,
    HYDRA_DEBUGGER_THREAD_STATUS_BREAK,
} HydraDebuggerThreadStatus;

// String list
__attribute__((returns_nonnull)) void* hydra_create_string_list();
void hydra_string_list_destroy(void* list);
uint32_t hydra_string_list_get_count(const void* list);
hydra_string hydra_string_list_get(const void* list, uint32_t index);
void hydra_string_list_resize(void* list, uint32_t size);
void hydra_string_list_set(void* list, uint32_t index, hydra_string value);
void hydra_string_list_append(void* list, hydra_string value);

// String view list
uint32_t hydra_string_view_list_get_count(const void* list);
hydra_string hydra_string_view_list_get(const void* list, uint32_t index);
void hydra_string_view_list_resize(void* list, uint32_t size);
void hydra_string_view_list_set(void* list, uint32_t index, hydra_string value);
void hydra_string_view_list_append(void* list, hydra_string value);

// String to string map
__attribute__((returns_nonnull)) void* hydra_create_string_to_string_map();
void hydra_string_to_string_map_destroy(void* map);
uint32_t hydra_string_to_string_map_get_count(const void* map);
hydra_string hydra_string_to_string_map_get_key(const void* map,
                                                uint32_t index);
hydra_string hydra_string_to_string_map_get_value(const void* map,
                                                  uint32_t index);
hydra_string hydra_string_to_string_map_get_value_by_key(const void* map,
                                                         hydra_string key);
void hydra_string_to_string_map_remove_all(void* map);
void hydra_string_to_string_map_set_by_key(void* map, hydra_string key,
                                           hydra_string value);

// Loader plugin
hydra_string hydra_loader_plugin_get_path(const void* plugin);
void hydra_loader_plugin_set_path(void* plugin, hydra_string path);
void* hydra_loader_plugin_get_options(void* plugin);

uint32_t hydra_loader_plugin_list_get_count(const void* list);
void* hydra_loader_plugin_list_get(void* list, uint32_t index);
void hydra_loader_plugin_list_resize(void* list, uint32_t size);

// Config
void hydra_config_serialize();
void hydra_config_deserialize();

hydra_string hydra_config_get_app_data_path();

void* hydra_config_get_game_paths();
void* hydra_config_get_loader_plugins();
void* hydra_config_get_patch_paths();
void* hydra_config_get_input_profiles();
uint32_t* hydra_config_get_cpu_backend();
uint32_t* hydra_config_get_gpu_renderer();
uint32_t* hydra_config_get_shader_backend();
uint32_t* hydra_config_get_display_resolution();
hydra_uint2* hydra_config_get_custom_display_resolution();
uint32_t* hydra_config_get_audio_backend();
hydra_u128* hydra_config_get_user_id();
hydra_string hydra_config_get_firmware_path();
void hydra_config_set_firmware_path(hydra_string value);
hydra_string hydra_config_get_sd_card_path();
void hydra_config_set_sd_card_path(hydra_string value);
hydra_string hydra_config_get_save_path();
void hydra_config_set_save_path(hydra_string value);
hydra_string hydra_config_get_sysmodules_path();
void hydra_config_set_sysmodules_path(hydra_string value);
bool* hydra_config_get_handheld_mode();
uint32_t* hydra_config_get_log_output();
bool* hydra_config_get_log_fs_access();
bool* hydra_config_get_debug_logging();
void* hydra_config_get_process_args();
bool* hydra_config_get_recover_from_segfault();
bool* hydra_config_get_gdb_enabled();
uint16_t* hydra_config_get_gdb_port();
bool* hydra_config_get_gdb_wait_for_client();

// Loader plugins

// Manager
void hydra_loader_plugin_manager_refresh();

// Plugin
void* hydra_create_loader_plugin(hydra_string path);
void hydra_loader_plugin_destroy(void* plugin);
hydra_string hydra_loader_plugin_get_name(const void* plugin);
hydra_string hydra_loader_plugin_get_display_version(const void* plugin);
uint32_t hydra_loader_plugin_get_supported_format_count(const void* plugin);
hydra_string hydra_loader_plugin_get_supported_format(const void* plugin,
                                                      uint32_t index);
uint32_t hydra_loader_plugin_get_option_config_count(const void* plugin);
const void* hydra_loader_plugin_get_option_config(const void* plugin,
                                                  uint32_t index);

// Option config
typedef enum HydraLoaderPluginOptionType : uint32_t {
    HYDRA_LOADER_PLUGIN_OPTION_TYPE_BOOLEAN = 0,
    HYDRA_LOADER_PLUGIN_OPTION_TYPE_INTEGER = 1,
    HYDRA_LOADER_PLUGIN_OPTION_TYPE_ENUMERATION = 2,
    HYDRA_LOADER_PLUGIN_OPTION_TYPE_STRING = 3,
    HYDRA_LOADER_PLUGIN_OPTION_TYPE_PATH = 4,
} HydraLoaderPluginOptionType;

void* hydra_loader_plugin_option_config_copy(const void* config);
void hydra_loader_plugin_option_config_destroy(void* config);
hydra_string hydra_loader_plugin_option_config_get_name(const void* config);
hydra_string
hydra_loader_plugin_option_config_get_description(const void* config);
HydraLoaderPluginOptionType
hydra_loader_plugin_option_config_get_type(const void* config);
bool hydra_loader_plugin_option_config_get_is_required(const void* config);
__attribute__((returns_nonnull)) const void*
hydra_loader_plugin_option_config_get_enum_value_names(const void* config);
__attribute__((returns_nonnull)) const void*
hydra_loader_plugin_option_config_get_path_content_types(const void* config);

// Filesystem
void* hydra_create_filesystem();
void hydra_filesystem_destroy(void* fs);
void hydra_try_install_firmware_to_filesystem(void* fs);

void* hydra_open_file(hydra_string path);
void hydra_file_close(void* file);

void* hydra_create_content_archive(void* file);
void hydra_content_archive_destroy(void* content_archive);
HydraContentArchiveContentType
hydra_content_archive_get_content_type(void* content_archive);

// Loader
void* hydra_create_loader_from_path(hydra_string path);
void hydra_loader_destroy(void* loader);
uint64_t hydra_loader_get_title_id(void* loader);
void* hydra_loader_load_nacp(void* loader);
void* hydra_loader_load_icon(void* loader, uint32_t* width, uint32_t* height);
bool hydra_loader_has_icon(const void* loader);
void hydra_loader_extract_icon(const void* loader, hydra_string path);
bool hydra_loader_has_exefs(const void* loader);
void hydra_loader_extract_exefs(const void* loader, hydra_string path);
bool hydra_loader_has_romfs(const void* loader);
void hydra_loader_extract_romfs(const void* loader, hydra_string path);

void* hydra_create_nca_loader_from_content_archive(void* content_archive);
hydra_string hydra_nca_loader_get_name(void* nca_loader);

// NACP
void hydra_nacp_destroy(void* nacp);
const void* hydra_nacp_get_title(void* nacp);
hydra_string hydra_nacp_get_display_version(void* nacp);

// NACP title
hydra_string hydra_nacp_title_get_name(const void* title);
hydra_string hydra_nacp_title_get_author(const void* title);

// User manager
void* hydra_create_user_manager();
void hydra_user_manager_destroy(void* user_manager);
void hydra_user_manager_flush(void* user_manager);
hydra_u128 hydra_user_manager_create_user(void* user_manager);
uint32_t hydra_user_manager_get_user_count(void* user_manager);
hydra_u128 hydra_user_manager_get_user_id(void* user_manager, uint32_t index);
void* hydra_user_manager_get_user(void* user_manager, hydra_u128 user_id);
void hydra_user_manager_load_system_avatars(void* user_manager, void* fs);
const void* hydra_user_manager_load_avatar_image(void* user_manager,
                                                 hydra_string path,
                                                 uint32_t* out_dimensions);
uint32_t hydra_user_manager_get_avatar_count(void* user_manager);
hydra_string hydra_user_manager_get_avatar_path(void* user_manager,
                                                uint32_t index);

hydra_string hydra_user_get_nickname(void* user);
void hydra_user_set_nickname(void* user, hydra_string nickname);
hydra_uchar3 hydra_user_get_avatar_bg_color(void* user);
void hydra_user_set_avatar_bg_color(void* user, hydra_uchar3 color);
hydra_string hydra_user_get_avatar_path(void* user);
void hydra_user_set_avatar_path(void* user, hydra_string path);

// Emulation context
void* hydra_create_emulation_context();
void hydra_emulation_context_destroy(void* ctx);

void hydra_emulation_context_set_surface(void* ctx, void* surface);

void hydra_emulation_context_load_and_start(void* ctx, void* loader);
void hydra_emulation_context_request_stop(void* ctx);
void hydra_emulation_context_force_stop(void* ctx);

void hydra_emulation_context_pause(void* ctx);
void hydra_emulation_context_resume(void* ctx);

void hydra_emulation_context_notify_operation_mode_changed(void* ctx);

void hydra_emulation_context_progress_frame(void* ctx, uint32_t width,
                                            uint32_t height,
                                            bool* out_dt_average_updated);

bool hydra_emulation_context_is_running(void* ctx);
float hydra_emulation_context_get_last_delta_time_average(void* ctx);

void hydra_emulation_context_take_screenshot(void* ctx);
void hydra_emulation_context_capture_gpu_frame(void* ctx);

// Input
// TODO

// Debugger

// Debugger manager
void hydra_debugger_manager_lock();
void hydra_debugger_manager_unlock();
uint64_t hydra_debugger_manager_get_debugger_count();
void* hydra_debugger_manager_get_debugger(uint32_t index);
void* hydra_debugger_manager_get_debugger_for_process(void* process);

// Debugger
hydra_string hydra_debugger_get_name(void* debugger);
void hydra_debugger_lock(void* debugger);
void hydra_debugger_unlock(void* debugger);
void hydra_debugger_register_this_thread(void* debugger, hydra_string name);
void hydra_debugger_unregister_this_thread(void* debugger);
uint64_t hydra_debugger_get_thread_count(void* debugger);
void* hydra_debugger_get_thread(void* debugger, uint32_t index);

// Thread
hydra_string hydra_debugger_thread_get_name(void* thread);
void hydra_debugger_thread_lock(void* thread);
void hydra_debugger_thread_unlock(void* thread);
HydraDebuggerThreadStatus hydra_debugger_thread_get_status(void* thread);
hydra_string hydra_debugger_thread_get_break_reason(void* thread);
uint64_t hydra_debugger_thread_get_message_count(void* thread);
const void* hydra_debugger_thread_get_message(void* thread, uint32_t index);

// Message
HydraLogLevel hydra_debugger_message_get_log_level(const void* msg);
HydraLogClass hydra_debugger_message_get_log_class(const void* msg);
hydra_string hydra_debugger_message_get_file(const void* msg);
uint32_t hydra_debugger_message_get_line(const void* msg);
hydra_string hydra_debugger_message_get_function(const void* msg);
hydra_string hydra_debugger_message_get_string(const void* msg);
const void* hydra_debugger_message_get_stack_trace(const void* msg);

// Stack trace
void* hydra_debugger_stack_trace_copy(const void* stack_trace);
void hydra_debugger_stack_trace_destroy(void* stack_trace);
uint32_t hydra_debugger_stack_trace_get_frame_count(const void* stack_trace);
const void* hydra_debugger_stack_trace_get_frame(const void* stack_trace,
                                                 uint32_t index);

// Stack frame
void* hydra_debugger_stack_frame_resolve(const void* stack_frame);

// Resolved stack frame
void hydra_debugger_resolved_stack_frame_destroy(void* resolved_stack_frame);
hydra_string hydra_debugger_resolved_stack_frame_get_module(
    const void* resolved_stack_frame);
hydra_string hydra_debugger_resolved_stack_frame_get_function(
    const void* resolved_stack_frame);
uint64_t hydra_debugger_resolved_stack_frame_get_address(
    const void* resolved_stack_frame);

#ifdef __cplusplus
}
#endif

#endif
