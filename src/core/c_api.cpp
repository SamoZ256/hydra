#include "core/c_api.h"

#include "core/emulation_context.hpp"
#include "core/horizon/ui/handler_base.hpp"

#define HYDRA_EXPORT extern "C" __attribute__((visibility("default")))

// Options
HYDRA_EXPORT bool hydra_bool_option_get(const void* option) {
    return static_cast<const hydra::Option<bool>*>(option)->Get();
}

HYDRA_EXPORT void hydra_bool_option_set(void* option, const bool value) {
    static_cast<hydra::Option<bool>*>(option)->Set(value);
}

HYDRA_EXPORT int32_t hydra_i32_option_get(const void* option) {
    return static_cast<const hydra::Option<hydra::i32>*>(option)->Get();
}

HYDRA_EXPORT void hydra_i32_option_set(void* option, const int32_t value) {
    static_cast<hydra::Option<hydra::i32>*>(option)->Set(value);
}

HYDRA_EXPORT uint32_t hydra_u32_option_get(const void* option) {
    return static_cast<const hydra::Option<hydra::i32>*>(option)->Get();
}

HYDRA_EXPORT void hydra_u32_option_set(void* option, const uint32_t value) {
    static_cast<hydra::Option<hydra::i32>*>(option)->Set(value);
}

HYDRA_EXPORT __uint128_t hydra_u128_option_get(const void* option) {
    return static_cast<const hydra::Option<hydra::u128>*>(option)->Get();
}

HYDRA_EXPORT void hydra_u128_option_set(void* option, const __uint128_t value) {
    static_cast<hydra::Option<hydra::u128>*>(option)->Set(value);
}

HYDRA_EXPORT const char* hydra_string_option_get(const void* option) {
    return static_cast<const hydra::Option<std::string>*>(option)
        ->Get()
        .c_str();
}

HYDRA_EXPORT void hydra_string_option_set(void* option, const char* value) {
    static_cast<hydra::Option<std::string>*>(option)->Set(value);
}

HYDRA_EXPORT uint32_t hydra_string_array_option_get_count(const void* option) {
    return static_cast<const hydra::ArrayOption<std::string>*>(option)
        ->GetCount();
}

HYDRA_EXPORT const char* hydra_string_array_option_get(const void* option,
                                                       uint32_t index) {
    return static_cast<const hydra::ArrayOption<std::string>*>(option)
        ->Get(index)
        .c_str();
}

HYDRA_EXPORT void hydra_string_array_option_add(void* option,
                                                const char* value) {
    static_cast<hydra::ArrayOption<std::string>*>(option)->Add(value);
}

HYDRA_EXPORT void hydra_string_array_option_set(void* option, uint32_t index,
                                                const char* value) {
    static_cast<hydra::ArrayOption<std::string>*>(option)->Set(index, value);
}

HYDRA_EXPORT void hydra_string_array_option_remove(void* option,
                                                   uint32_t index) {
    static_cast<hydra::ArrayOption<std::string>*>(option)->Remove(index);
}

// Config
HYDRA_EXPORT void hydra_config_serialize() {
    hydra::CONFIG_INSTANCE.Serialize();
}

HYDRA_EXPORT void hydra_config_deserialize() {
    hydra::CONFIG_INSTANCE.Deserialize();
}

HYDRA_EXPORT void* hydra_config_get_game_paths() {
    return &hydra::CONFIG_INSTANCE.GetGamePaths();
}

HYDRA_EXPORT void* hydra_config_get_patch_paths() {
    return &hydra::CONFIG_INSTANCE.GetPatchPaths();
}

HYDRA_EXPORT void* hydra_config_get_sd_card_path() {
    return &hydra::CONFIG_INSTANCE.GetSdCardPath();
}

HYDRA_EXPORT void* hydra_config_get_save_path() {
    return &hydra::CONFIG_INSTANCE.GetSavePath();
}

HYDRA_EXPORT void* hydra_config_get_cpu_backend() {
    return &hydra::CONFIG_INSTANCE.GetCpuBackend();
}

HYDRA_EXPORT void* hydra_config_get_gpu_renderer() {
    return &hydra::CONFIG_INSTANCE.GetGpuRenderer();
}

HYDRA_EXPORT void* hydra_config_get_shader_backend() {
    return &hydra::CONFIG_INSTANCE.GetShaderBackend();
}

HYDRA_EXPORT void* hydra_config_get_user_id() {
    return &hydra::CONFIG_INSTANCE.GetUserID();
}

HYDRA_EXPORT void* hydra_config_get_logging_output() {
    return &hydra::CONFIG_INSTANCE.GetLoggingOutput();
}

HYDRA_EXPORT void* hydra_config_get_debug_logging() {
    return &hydra::CONFIG_INSTANCE.GetDebugLogging();
}

HYDRA_EXPORT void* hydra_config_get_stack_trace_logging() {
    return &hydra::CONFIG_INSTANCE.GetStackTraceLogging();
}

HYDRA_EXPORT void* hydra_config_get_process_args() {
    return &hydra::CONFIG_INSTANCE.GetProcessArgs();
}

// Emulation context
// TODO: proper UI handler
class UiHandler : public hydra::horizon::ui::HandlerBase {
  public:
    void ShowMessageDialog(const hydra::horizon::ui::MessageDialogType type,
                           const std::string& title,
                           const std::string& message) override {}
};

HYDRA_EXPORT void* hydra_emulation_context_create() {
    return new hydra::EmulationContext(*(new UiHandler()));
}

HYDRA_EXPORT void hydra_emulation_context_destroy(void* ctx) {
    // TODO: also destroy the UI handler
    delete static_cast<hydra::EmulationContext*>(ctx);
}

HYDRA_EXPORT void hydra_emulation_context_set_surface(void* ctx,
                                                      void* surface) {
    static_cast<hydra::EmulationContext*>(ctx)->SetSurface(surface);
}

HYDRA_EXPORT void hydra_emulation_context_load_rom(void* ctx,
                                                   const char* rom_filename) {
    static_cast<hydra::EmulationContext*>(ctx)->LoadRom(rom_filename);
}

HYDRA_EXPORT void hydra_emulation_context_run(void* ctx) {
    static_cast<hydra::EmulationContext*>(ctx)->Run();
}

HYDRA_EXPORT void hydra_emulation_context_progress_frame(
    void* ctx, uint32_t width, uint32_t height, bool* out_dt_average_updated) {
    static_cast<hydra::EmulationContext*>(ctx)->ProgressFrame(
        width, height, *out_dt_average_updated);
}

HYDRA_EXPORT uint64_t hydra_emulation_context_get_title_id(void* ctx) {
    return static_cast<hydra::EmulationContext*>(ctx)->GetTitleID();
}

HYDRA_EXPORT bool hydra_emulation_context_is_running(void* ctx) {
    return static_cast<hydra::EmulationContext*>(ctx)->IsRunning();
}

HYDRA_EXPORT float
hydra_emulation_context_get_last_delta_time_average(void* ctx) {
    return static_cast<hydra::EmulationContext*>(ctx)
        ->GetLastDeltaTimeAverage();
}
