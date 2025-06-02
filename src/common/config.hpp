#pragma once

#include <fmt/ranges.h>

#include "common/log.hpp"
#include "common/types.hpp"

#define CONFIG_INSTANCE Config::GetInstance()

namespace hydra {

enum class CpuBackend : u32 {
    Invalid = 0,

    AppleHypervisor = 1,
    Dynarmic = 2,
};

enum class GpuRenderer : u32 {
    Invalid = 0,

    Metal = 1,
};

enum class ShaderBackend : u32 {
    Invalid = 0,

    Msl = 1,
    Air = 2,
};

enum class AudioBackend : u32 {
    Invalid = 0,

    Null = 1,
    Cubeb = 2,
};

template <typename T> class Option {
  public:
    Option() {}
    Option(const T& value_) : value{value_} {}

    operator T() { return value; }
    operator T() const { return value; }
    void operator=(const T& other) { value = other; }

    const T& Get() const { return value; }
    void Set(const T& other) { value = other; }

  private:
    T value;
};

template <typename T> class ArrayOption {
  public:
    ArrayOption() {}
    ArrayOption(const std::vector<T>& values_) : values{values_} {}

    void operator=(const std::vector<T>& other) { values = other; }
    const T& operator[](u32 index) const {
        VerifyIndex(index);
        return values[index];
    }
    T& operator[](u32 index) {
        VerifyIndex(index);
        return values[index];
    }

    const std::vector<T>& Get() const { return values; }
    const T& Get(u32 index) const { return values[index]; }
    usize GetCount() const { return values.size(); }

    void Add(const T& value) { values.push_back(value); }
    void Set(u32 index, const T& value) {
        VerifyIndex(index);
        values[index] = value;
    }
    void Remove(u32 index) {
        VerifyIndex(index);
        values.erase(values.begin() + index);
    }

  private:
    std::vector<T> values;

    // Helpers
    inline void VerifyIndex(u32 index) const {
        ASSERT(index < values.size(), Other, "Index ({}) out of range ({})",
               index, values.size());
    }
};

class Config {
  public:
    static Config& GetInstance() {
        static Config g_config;
        return g_config;
    }

    void Initialize();

    void LoadDefaults();

    void Serialize();
    void Deserialize();

    void Log();

    // Paths
    const std::string_view GetAppDataPath() const { return app_data_path; }
    const std::string_view GetLogsPath() const { return logs_path; }

    std::string GetConfigPath() const {
        return fmt::format("{}/config.toml", app_data_path);
    }

    // Getters
    ArrayOption<std::string>& GetGamePaths() { return game_paths; }
    ArrayOption<std::string>& GetPatchPaths() { return patch_paths; }
    Option<std::string>& GetSdCardPath() { return sd_card_path; }
    Option<std::string>& GetSavePath() { return save_path; }
    Option<CpuBackend>& GetCpuBackend() { return cpu_backend; }
    Option<GpuRenderer>& GetGpuRenderer() { return gpu_renderer; }
    Option<ShaderBackend>& GetShaderBackend() { return shader_backend; }
    Option<AudioBackend>& GetAudioBackend() { return audio_backend; }
    Option<uuid_t>& GetUserID() { return user_id; }
    Option<Output>& GetLoggingOutput() { return logging_output; }
    Option<bool>& GetLogFsAccess() { return log_fs_access; }
    Option<bool>& GetDebugLogging() { return debug_logging; }
    Option<bool>& GetStackTraceLogging() { return stack_trace_logging; }
    ArrayOption<std::string>& GetProcessArgs() { return process_args; }

  private:
    std::string app_data_path;
    std::string logs_path;

    // Config
    ArrayOption<std::string> game_paths;
    ArrayOption<std::string> patch_paths;
    Option<std::string> sd_card_path;
    Option<std::string> save_path;
    Option<CpuBackend> cpu_backend;
    Option<GpuRenderer> gpu_renderer;
    Option<ShaderBackend> shader_backend;
    Option<AudioBackend> audio_backend;
    Option<uuid_t> user_id;
    Option<Output> logging_output =
        Output::StdOut; // Set to stdout so that messages logged
                        // before logs path is initialized get logged
                        // properly
    Option<bool> log_fs_access;
    Option<bool> debug_logging;
    Option<bool> stack_trace_logging;
    ArrayOption<std::string> process_args;

    // Default values
    std::vector<std::string> GetDefaultGamePaths() const { return {}; }
    std::vector<std::string> GetDefaultPatchPaths() const { return {}; }
    std::string GetDefaultSdCardPath() const {
        return fmt::format("{}/sdmc", app_data_path);
    }
    std::string GetDefaultSavePath() const {
        return fmt::format("{}/save", app_data_path);
    }
    CpuBackend GetDefaultCpuBackend() const {
        // TODO: use Dynarmic by default on all platforms except for Apple
        // Silicon
        return CpuBackend::AppleHypervisor;
    }
    GpuRenderer GetDefaultGpuRenderer() const { return GpuRenderer::Metal; }
    ShaderBackend GetDefaultShaderBackend() const { return ShaderBackend::Msl; }
    AudioBackend GetDefaultAudioBackend() const { return AudioBackend::Null; }
    uuid_t GetDefaultUserID() const {
        return 0x0; // TODO: INVALID_USER_ID
    }
    Output GetDefaultLoggingOutput() const { return Output::File; }
    bool GetDefaultLogFsAccess() const { return false; }
    bool GetDefaultDebugLogging() const { return false; }
    bool GetDefaultStackTraceLogging() const { return false; }
    std::vector<std::string> GetDefaultProcessArgs() const { return {}; }
};

} // namespace hydra

template <typename T>
struct fmt::formatter<hydra::Option<T>> : formatter<string_view> {
    template <typename FormatContext>
    auto format(const hydra::Option<T>& option, FormatContext& ctx) const {
        return formatter<string_view>::format(fmt::format("{}", option.Get()),
                                              ctx);
    }
};

template <typename T>
struct fmt::formatter<hydra::ArrayOption<T>> : formatter<string_view> {
    template <typename FormatContext>
    auto format(const hydra::ArrayOption<T>& option, FormatContext& ctx) const {
        // TODO: simplify
        return formatter<string_view>::format(
            fmt::format("{}", fmt::join(option.Get(), ", ")), ctx);
    }
};

ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, CpuBackend, cpu_backend,
                                   AppleHypervisor, "Apple Hypervisor",
                                   Dynarmic, "dynarmic")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, GpuRenderer, gpu_renderer, Metal,
                                   "Metal")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, ShaderBackend, shader_backend, Msl,
                                   "MSL", Air, "AIR")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, AudioBackend, audio_backend, Null,
                                   "Null", Cubeb, "Cubeb")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, Output, output, StdOut, "stdout",
                                   File, "file")
