#pragma once

#include <fmt/ranges.h>

#include "common/log.hpp"
#include "common/types.hpp"

#define CONFIG_INSTANCE Config::GetInstance()

namespace hydra {

enum class CpuBackend : u32 {
    Invalid = 0,

    AppleHypervisor,
    Dynarmic,
};

enum class GpuRenderer : u32 {
    Invalid = 0,

    Metal,
};

enum class ShaderBackend : u32 {
    Invalid = 0,

    Msl,
    Air,
};

enum class Resolution : u32 {
    Invalid = 0,

    Auto,
    _720p,
    _1080p,
    _1440p,
    _2160p,
    _4320p,
    AutoExact,
    Custom,
};

STRONG_TYPEDEF(CustomResolution, uint2);

enum class AudioBackend : u32 {
    Invalid = 0,

    Null,
    Cubeb,
};

template <typename T, typename GetT>
class _Option {
  public:
    _Option() {}
    _Option(GetT value_) : value{value_} {}

    operator GetT() const { return value; }
    void operator=(GetT other) { value = other; }

    GetT Get() const { return value; }
    void Set(GetT other) { value = other; }

  private:
    T value;
};

template <typename T>
using Option = _Option<T, T>;

using StringOption = _Option<std::string, std::string_view>;

template <typename T, typename GetT>
class _ArrayOption {
  public:
    _ArrayOption() {}
    _ArrayOption(const std::vector<T>& values_) : values{values_} {}

    void operator=(const std::vector<T>& other) { values = other; }
    GetT operator[](u32 index) const {
        VerifyIndex(index);
        return values[index];
    }
    T& operator[](u32 index) {
        VerifyIndex(index);
        return values[index];
    }

    const std::vector<T>& Get() const { return values; }
    GetT Get(u32 index) const { return values[index]; }
    usize GetCount() const { return values.size(); }

    void Resize(u32 size) { values.resize(size); }
    void Set(u32 index, GetT value) {
        VerifyIndex(index);
        values[index] = value;
    }
    void Append(GetT value) { values.push_back(T(value)); }

  private:
    std::vector<T> values;

    // Helpers
    inline void VerifyIndex(u32 index) const {
        ASSERT(index < values.size(), Other, "Index ({}) out of range ({})",
               index, values.size());
    }
};

template <typename T>
using ArrayOption = _ArrayOption<T, T>;

using StringArrayOption = _ArrayOption<std::string, std::string_view>;

class Config {
  public:
    static Config& GetInstance() {
        static Config g_config;
        return g_config;
    }

    Config();

    void LoadDefaults();

    void Serialize();
    void Deserialize();

    void Log();

    // Paths
    const std::string_view GetAppDataPath() const { return app_data_path; }
    const std::string_view GetLogsPath() const { return logs_path; }
    const std::string_view GetPicturesPath() const { return pictures_path; }

    std::string GetConfigPath() const {
        return fmt::format("{}/config.toml", app_data_path);
    }

    // Getters
    StringArrayOption& GetGamePaths() { return game_paths; }
    StringArrayOption& GetPatchPaths() { return patch_paths; }
    Option<CpuBackend>& GetCpuBackend() { return cpu_backend; }
    Option<GpuRenderer>& GetGpuRenderer() { return gpu_renderer; }
    Option<ShaderBackend>& GetShaderBackend() { return shader_backend; }
    Option<Resolution>& GetDisplayResolution() { return display_resolution; }
    Option<uint2>& GetCustomDisplayResolution() {
        return custom_display_resolution;
    }
    Option<AudioBackend>& GetAudioBackend() { return audio_backend; }
    Option<uuid_t>& GetUserID() { return user_id; }
    StringOption& GetFirmwarePath() { return firmware_path; }
    StringOption& GetSdCardPath() { return sd_card_path; }
    StringOption& GetSavePath() { return save_path; }
    StringOption& GetSysmodulesPath() { return sysmodules_path; }
    Option<bool>& GetHandheldMode() { return handheld_mode; }
    Option<LogOutput>& GetLogOutput() { return log_output; }
    Option<bool>& GetLogFsAccess() { return log_fs_access; }
    Option<bool>& GetDebugLogging() { return debug_logging; }
    StringArrayOption& GetProcessArgs() { return process_args; }
    Option<bool>& GetGdbEnabled() { return gdb_enabled; }
    Option<u16>& GetGdbPort() { return gdb_port; }
    Option<bool>& GetGdbWaitForClient() { return gdb_wait_for_client; }

  private:
    std::string app_data_path;
    std::string logs_path;
    std::string pictures_path; // TODO: remove this

    // Config
    StringArrayOption game_paths;
    StringArrayOption patch_paths;
    Option<CpuBackend> cpu_backend;
    Option<GpuRenderer> gpu_renderer;
    Option<ShaderBackend> shader_backend;
    Option<Resolution> display_resolution;
    Option<uint2> custom_display_resolution;
    Option<AudioBackend> audio_backend;
    Option<uuid_t> user_id;
    StringOption firmware_path;
    StringOption sd_card_path;
    StringOption save_path;
    StringOption sysmodules_path;
    Option<bool> handheld_mode;
    Option<LogOutput> log_output;
    Option<bool> log_fs_access;
    Option<bool> debug_logging;
    StringArrayOption process_args;
    Option<bool> gdb_enabled;
    Option<u16> gdb_port;
    Option<bool> gdb_wait_for_client;

    // Default values
    std::vector<std::string> GetDefaultGamePaths() const { return {}; }
    std::vector<std::string> GetDefaultPatchPaths() const { return {}; }
    CpuBackend GetDefaultCpuBackend() const {
#if HYDRA_HYPERVISOR_ENABLED
        return CpuBackend::AppleHypervisor;
#else
        return CpuBackend::Dynarmic;
#endif
    }
    GpuRenderer GetDefaultGpuRenderer() const { return GpuRenderer::Metal; }
    ShaderBackend GetDefaultShaderBackend() const { return ShaderBackend::Msl; }
    Resolution GetDefaultDisplayResolution() const { return Resolution::Auto; }
    uint2 GetDefaultCustomDisplayResolution() const { return {1920, 1080}; }
    AudioBackend GetDefaultAudioBackend() const {
#if HYDRA_CUBEB_ENABLED
        return AudioBackend::Cubeb;
#else
        return AudioBackend::Null;
#endif
    }
    uuid_t GetDefaultUserID() const {
        return 0x0; // TODO: INVALID_USER_ID
    }
    std::string GetDefaultFirmwarePath() const { return ""; }
    std::string GetDefaultSdCardPath() const {
        return fmt::format("{}/sdmc", app_data_path);
    }
    std::string GetDefaultSavePath() const {
        return fmt::format("{}/save", app_data_path);
    }
    std::string GetDefaultSysmodulesPath() const {
        return fmt::format("{}/sysmodules", app_data_path);
    }
    bool GetDefaultHandheldMode() const { return true; }
    LogOutput GetDefaultLogOutput() const { return LogOutput::File; }
    bool GetDefaultLogFsAccess() const { return false; }
    bool GetDefaultDebugLogging() const { return false; }
    std::vector<std::string> GetDefaultProcessArgs() const { return {}; }
    bool GetDefaultGdbEnabled() const { return false; }
    u16 GetDefaultGdbPort() const { return 1234; }
    bool GetDefaultGdbWaitForClient() const { return false; }
};

} // namespace hydra

template <typename T, typename GetT>
struct fmt::formatter<hydra::_Option<T, GetT>> : formatter<string_view> {
    template <typename FormatContext>
    auto format(const hydra::_Option<T, GetT>& option,
                FormatContext& ctx) const {
        return formatter<string_view>::format(fmt::format("{}", option.Get()),
                                              ctx);
    }
};

template <typename T, typename GetT>
struct fmt::formatter<hydra::_ArrayOption<T, GetT>> : formatter<string_view> {
    template <typename FormatContext>
    auto format(const hydra::_ArrayOption<T, GetT>& option,
                FormatContext& ctx) const {
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
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, Resolution, resolution, Auto, "auto",
                                   _720p, "720p", _1080p, "1080p", _1440p,
                                   "1440p", _2160p, "2160p", _4320p, "4320p",
                                   AutoExact, "Auto exact", Custom, "custom")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, AudioBackend, audio_backend, Null,
                                   "Null", Cubeb, "Cubeb")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, LogOutput, output, None, "none",
                                   StdOut, "stdout", File, "file")
