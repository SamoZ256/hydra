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
    HYDRA_LOGGING_OUTPUT_INVALID = 0,

    HYDRA_LOGGING_OUTPUT_STD_OUT,
    HYDRA_LOGGING_OUTPUT_FILE,
} HydraLoggingOutput;

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
void hydra_string_array_option_add(void* option, const char* value);
void hydra_string_array_option_set(void* option, uint32_t index,
                                   const char* value);
void hydra_string_array_option_remove(void* option, uint32_t index);

// Config
void hydra_config_serialize();
void hydra_config_deserialize();

void* hydra_config_get_game_paths();
void* hydra_config_get_patch_paths();
void* hydra_config_get_sd_card_path();
void* hydra_config_get_save_path();
void* hydra_config_get_cpu_backend();
void* hydra_config_get_gpu_renderer();
void* hydra_config_get_shader_backend();
void* hydra_config_get_user_id();
void* hydra_config_get_logging_output();
void* hydra_config_get_debug_logging();
void* hydra_config_get_stack_trace_logging();
void* hydra_config_get_process_args();

// Emulation context
void* hydra_emulation_context_create();
void hydra_emulation_context_destroy(void* ctx);

void hydra_emulation_context_set_surface(void* ctx, void* surface);
void hydra_emulation_context_load_rom(void* ctx, const char* rom_filename);
void hydra_emulation_context_run(void* ctx);

void hydra_emulation_context_progress_frame(void* ctx, uint32_t width,
                                            uint32_t height,
                                            bool* out_dt_average_updated);

uint64_t hydra_emulation_context_get_title_id(void* ctx);

bool hydra_emulation_context_is_running(void* ctx);
float hydra_emulation_context_get_last_delta_time_average(void* ctx);

// Input
// TODO

#ifdef __cplusplus
}
#endif

#endif
