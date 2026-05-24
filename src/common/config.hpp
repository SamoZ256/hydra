#pragma once

#include <fmt/ranges.h>

#include "common/log.hpp"
#include "common/platform.hpp"
#include "common/types.hpp"

#define CONFIG_INSTANCE Config::GetInstance()

namespace hydra {

enum class InputBackend : u32 {
    AppleGameController,
    Sdl,
};

enum class CpuBackend : u32 {
    AppleHypervisor,
    Dynarmic,
};

enum class GpuRenderer : u32 {
    Metal,
};

enum class ShaderBackend : u32 {
    Msl,
    Air,
};

enum class Resolution : u32 {
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
    Null,
    Cubeb,
};

enum class SystemLanguage : u32 {
    AmericanEnglish = 0,
    BritishEnglish = 1,
    Japanese = 2,
    French = 3,
    German = 4,
    LatinAmericanSpanish = 5,
    Spanish = 6,
    Italian = 7,
    Dutch = 8,
    CanadianFrench = 9,
    Portuguese = 10,
    Russian = 11,
    Korean = 12,
    TraditionalChinese = 13,
    SimplifiedChinese = 14,
    BrazilianPortuguese = 15,
    Polish = 16,
    Thai = 17,
};

struct LoaderPlugin {
    std::string path;
    std::map<std::string, std::string> options;
};

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

    // Default values
    static std::vector<std::string> GetDefaultGamePaths() { return {}; }
    static std::vector<LoaderPlugin> GetDefaultLoaderPlugins() { return {}; }
    static std::vector<std::string> GetDefaultPatchPaths() { return {}; }
    static InputBackend GetDefaultInputBackend() {
#ifdef PLATFORM_APPLE
        return InputBackend::AppleGameController;
#else
        return InputBackend::Sdl;
#endif
    }
    static std::vector<std::string> GetDefaultInputProfiles() {
        return {"Default", "", "", "", "", "", "", "", "", ""};
    }
    static CpuBackend GetDefaultCpuBackend() {
#ifdef HYDRA_HYPERVISOR_ENABLED
        return CpuBackend::AppleHypervisor;
#else
        return CpuBackend::Dynarmic;
#endif
    }
    static GpuRenderer GetDefaultGpuRenderer() { return GpuRenderer::Metal; }
    static ShaderBackend GetDefaultShaderBackend() {
        return ShaderBackend::Msl;
    }
    static Resolution GetDefaultDisplayResolution() { return Resolution::Auto; }
    static uint2 GetDefaultCustomDisplayResolution() { return {1920, 1080}; }
    static AudioBackend GetDefaultAudioBackend() {
#ifdef HYDRA_CUBEB_ENABLED
        return AudioBackend::Cubeb;
#else
        return AudioBackend::Null;
#endif
    }
    static uuid_t GetDefaultUserID() {
        return 0x0; // TODO: INVALID_USER_ID
    }
    static SystemLanguage GetDefaultSystemLanguage() {
        return SystemLanguage::AmericanEnglish;
    }
    static std::string GetDefaultFirmwarePath() { return ""; }
    std::string GetDefaultSdCardPath() const {
        return fmt::format("{}/sdmc", app_data_path);
    }
    std::string GetDefaultSavePath() const {
        return fmt::format("{}/save", app_data_path);
    }
    std::string GetDefaultSysmodulesPath() const {
        return fmt::format("{}/sysmodules", app_data_path);
    }
    static bool GetDefaultHandheldMode() { return true; }
    static LogOutput GetDefaultLogOutput() { return LogOutput::File; }
    static bool GetDefaultLogFsAccess() { return false; }
    static bool GetDefaultDebugLogging() { return false; }
    static std::vector<std::string> GetDefaultProcessArgs() { return {}; }
    static bool GetDefaultRecoverFromSegfault() { return false; }
    static bool GetDefaultGdbEnabled() { return false; }
    static u16 GetDefaultGdbPort() { return 1234; }
    static bool GetDefaultGdbWaitForClient() { return false; }

  private:
    std::string app_data_path;
    std::string logs_path;
    std::string pictures_path; // TODO: remove this

    // Config
    std::vector<std::string> game_paths;
    std::vector<LoaderPlugin> loader_plugins;
    std::vector<std::string> patch_paths;
    InputBackend input_backend;
    std::vector<std::string> input_profiles;
    CpuBackend cpu_backend;
    GpuRenderer gpu_renderer;
    ShaderBackend shader_backend;
    Resolution display_resolution;
    uint2 custom_display_resolution;
    AudioBackend audio_backend;
    uuid_t user_id;
    SystemLanguage system_language;
    std::string firmware_path;
    std::string sd_card_path;
    std::string save_path;
    std::string sysmodules_path;
    bool handheld_mode;
    LogOutput log_output;
    bool log_fs_access;
    bool debug_logging;
    std::vector<std::string> process_args;
    bool recover_from_segfault;
    bool gdb_enabled;
    u16 gdb_port;
    bool gdb_wait_for_client;

  public:
    REF_GETTER(game_paths, GetGamePaths);
    REF_GETTER(loader_plugins, GetLoaderPlugins);
    REF_GETTER(patch_paths, GetPatchPaths);
    REF_GETTER(input_backend, GetInputBackend);
    REF_GETTER(input_profiles, GetInputProfiles);
    REF_GETTER(cpu_backend, GetCpuBackend);
    REF_GETTER(gpu_renderer, GetGpuRenderer);
    REF_GETTER(shader_backend, GetShaderBackend);
    REF_GETTER(display_resolution, GetDisplayResolution);
    REF_GETTER(custom_display_resolution, GetCustomDisplayResolution);
    REF_GETTER(audio_backend, GetAudioBackend);
    REF_GETTER(user_id, GetUserId);
    REF_GETTER(system_language, GetSystemLanguage);
    REF_GETTER(firmware_path, GetFirmwarePath);
    REF_GETTER(sd_card_path, GetSdCardPath);
    REF_GETTER(save_path, GetSavePath);
    REF_GETTER(sysmodules_path, GetSysmodulesPath);
    REF_GETTER(handheld_mode, GetHandheldMode);
    REF_GETTER(log_output, GetLogOutput);
    REF_GETTER(log_fs_access, GetLogFsAccess);
    REF_GETTER(debug_logging, GetDebugLogging);
    REF_GETTER(process_args, GetProcessArgs);
    REF_GETTER(recover_from_segfault, GetRecoverFromSegfault);
    REF_GETTER(gdb_enabled, GetGdbEnabled);
    REF_GETTER(gdb_port, GetGdbPort);
    REF_GETTER(gdb_wait_for_client, GetGdbWaitForClient);
};

} // namespace hydra

ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, InputBackend, AppleGameController,
                                   "Apple GameController", Sdl, "SDL")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, CpuBackend, AppleHypervisor,
                                   "Apple Hypervisor", Dynarmic, "dynarmic")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, GpuRenderer, Metal, "Metal")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, ShaderBackend, Msl, "MSL", Air, "AIR")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, Resolution, Auto, "auto", _720p,
                                   "720p", _1080p, "1080p", _1440p, "1440p",
                                   _2160p, "2160p", _4320p, "4320p", AutoExact,
                                   "Auto exact", Custom, "custom")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, AudioBackend, Null, "Null", Cubeb,
                                   "Cubeb")
ENABLE_ENUM_FORMATTING_AND_CASTING(
    hydra, SystemLanguage, AmericanEnglish, "American English", BritishEnglish,
    "British English", Japanese, "Japanese", French, "French", German, "German",
    LatinAmericanSpanish, "Latin American Spanish", Spanish, "Spanish", Italian,
    "Italian", Dutch, "Dutch", CanadianFrench, "Canadian French", Portuguese,
    "Portuguese", Russian, "Russian", Korean, "Korean", TraditionalChinese,
    "Traditional Chinese", SimplifiedChinese, "Simplified Chinese",
    BrazilianPortuguese, "Brazilian Portuguese", Polish, "Polish", Thai, "Thai")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, LogOutput, None, "none", StdOut,
                                   "stdout", File, "file")
