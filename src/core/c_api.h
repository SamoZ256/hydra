#ifndef HYDRA_C_API
#define HYDRA_C_API

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

// Options
bool hydra_bool_option_get(const void* option);
void hydra_bool_option_set(void* option, const bool value);

int32_t hydra_i32_option_get(const void* option);
void hydra_i32_option_set(void* option, const int32_t value);

uint32_t hydra_u32_option_get(const void* option);
void hydra_u32_option_set(void* option, const uint32_t value);

__uint128_t hydra_u128_option_get(const void* option);
void hydra_u128_option_set(void* option, const __uint128_t value);

const char* hydra_string_option_get(const void* option);
void hydra_string_option_set(void* option, const char* value);

uint32_t hydra_string_array_option_get_count(const void* option);
const char* hydra_string_array_option_get(const void* option, uint32_t index);
void hydra_string_array_option_resize(void* option, uint64_t size);
void hydra_string_array_option_set(void* option, uint32_t index,
                                   const char* value);

// Config
void hydra_config_serialize();
void hydra_config_deserialize();

void* hydra_config_get_game_paths();
void* hydra_config_get_patch_paths();
void* hydra_config_get_cpu_backend();
void* hydra_config_get_gpu_renderer();
void* hydra_config_get_shader_backend();
void* hydra_config_get_user_id();
void* hydra_config_get_firmware_path();
void* hydra_config_get_sd_card_path();
void* hydra_config_get_save_path();
void* hydra_config_get_log_output();
void* hydra_config_get_debug_logging();
void* hydra_config_get_process_args();

// Filesystem
void* hydra_open_file(const char* path);
void hydra_file_close(void* file);

void* hydra_create_content_archive(void* file);
void hydra_content_archive_destroy(void* content_archive);
HydraContentArchiveContentType
hydra_content_archive_get_content_type(void* content_archive);

// Loader
void* hydra_create_loader_from_file(const char* path);
void hydra_loader_destroy(void* loader);
uint64_t hydra_loader_get_title_id(void* loader);
void hydra_loader_load_icon(void* loader, void** data, uint64_t* width,
                            uint64_t* height);

void* hydra_create_nca_loader_from_content_archive(void* content_archive);
const char* hydra_nca_loader_get_name(void* nca_loader);

// Emulation context
void* hydra_create_emulation_context();
void hydra_emulation_context_destroy(void* ctx);

void hydra_emulation_context_set_surface(void* ctx, void* surface);
void hydra_emulation_context_load_and_start(void* ctx, void* loader);
void hydra_emulation_context_request_stop(void* ctx);
void hydra_emulation_context_force_stop(void* ctx);

void hydra_emulation_context_progress_frame(void* ctx, uint32_t width,
                                            uint32_t height,
                                            bool* out_dt_average_updated);

bool hydra_emulation_context_is_running(void* ctx);
float hydra_emulation_context_get_last_delta_time_average(void* ctx);

// Input
// TODO

// Debugger

// Debugger
void hydra_debugger_enable();
void hydra_debugger_disable();
void hydra_debugger_lock();
void hydra_debugger_unlock();
void hydra_debugger_register_this_thread(const char* name);
void hydra_debugger_unregister_this_thread();
uint64_t hydra_debugger_get_thread_count();
void* hydra_debugger_get_thread(uint32_t index);

// Thread
void hydra_debugger_thread_lock(void* thread);
void hydra_debugger_thread_unlock(void* thread);
const char* hydra_debugger_thread_get_name(void* thread);
HydraDebuggerThreadStatus hydra_debugger_thread_get_status(void* thread);
const char* hydra_debugger_thread_get_break_reason(void* thread);
uint64_t hydra_debugger_thread_get_message_count(void* thread);
const void* hydra_debugger_thread_get_message(void* thread, uint32_t index);

// Message
HydraLogLevel hydra_debugger_message_get_log_level(const void* msg);
HydraLogClass hydra_debugger_message_get_log_class(const void* msg);
const char* hydra_debugger_message_get_file(const void* msg);
uint32_t hydra_debugger_message_get_line(const void* msg);
const char* hydra_debugger_message_get_function(const void* msg);
const char* hydra_debugger_message_get_string(const void* msg);
const void* hydra_debugger_message_get_stack_trace(const void* msg);

// Stack trace
void* hydra_debugger_stack_trace_copy(const void* stack_trace);
void hydra_debugger_stack_trace_destroy(void* stack_trace);
uint32_t hydra_debugger_stack_trace_get_frame_count(const void* stack_trace);
const void* hydra_debugger_stack_trace_get_frame(const void* stack_trace,
                                                 uint32_t index);

// Stack frame
void* hydra_debugger_stack_frame_resolve_unmanaged(const void* stack_frame);

// Resolved stack frame
void hydra_debugger_resolved_stack_frame_destroy(void* resolved_stack_frame);
const char* hydra_debugger_resolved_stack_frame_get_module(
    const void* resolved_stack_frame);
const char* hydra_debugger_resolved_stack_frame_get_function(
    const void* resolved_stack_frame);
uint64_t hydra_debugger_resolved_stack_frame_get_address(
    const void* resolved_stack_frame);

#ifdef __cplusplus
}
#endif

#endif
