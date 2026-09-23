#pragma once

#include <cstdlib>

#include <fmt/ranges.h>

#include "common/fmt_helper.hpp"
#include "common/log.hpp"
#include "common/types.hpp"

#define CONFIG_INSTANCE Config::getInstance()

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
    Null,
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
    static Config& getInstance() {
        static Config g_config;
        return g_config;
    }

    Config();

    void loadDefaults();

    void serialize();
    void deserialize();

    void log();

    // Paths
    std::string_view getAppDataPath() const { return app_data_path; }
    std::string_view getLogsPath() const { return logs_path; }
    std::string_view getPicturesPath() const { return pictures_path; }

    std::string getConfigPath() const {
        return fmt::format("{}/config.toml", app_data_path);
    }

    // Default values
    static std::vector<std::string> getDefaultGamePaths() { return {}; }
    static std::vector<LoaderPlugin> getDefaultLoaderPlugins() { return {}; }
    static std::vector<std::string> getDefaultPatchPaths() { return {}; }
    static InputBackend getDefaultInputBackend() {
#ifdef ZTD_PLATFORM_APPLE
        return InputBackend::AppleGameController;
#else
        return InputBackend::Sdl;
#endif
    }
    static std::vector<std::string> getDefaultInputProfiles() {
        return {"Default", "", "", "", "", "", "", "", "", ""};
    }
    static CpuBackend getDefaultCpuBackend() {
#ifdef HYDRA_HYPERVISOR_ENABLED
        return CpuBackend::AppleHypervisor;
#else
        return CpuBackend::Dynarmic;
#endif
    }
    static GpuRenderer getDefaultGpuRenderer() {
#ifdef ZTD_PLATFORM_APPLE
        return GpuRenderer::Metal;
#else
        return GpuRenderer::Null;
#endif
    }
    static ShaderBackend getDefaultShaderBackend() {
        return ShaderBackend::Msl;
    }
    static Resolution getDefaultDisplayResolution() { return Resolution::Auto; }
    static uint2 getDefaultCustomDisplayResolution() { return {1920, 1080}; }
    static AudioBackend getDefaultAudioBackend() {
#ifdef HYDRA_CUBEB_ENABLED
        return AudioBackend::Cubeb;
#else
        return AudioBackend::Null;
#endif
    }
    static uuid_t getDefaultUserId() {
        return 0x0; // TODO: INVALID_USER_ID
    }
    static constexpr std::string_view getDefaultDeviceNickname() {
        return "Hydra's Switch";
    }
    static SystemLanguage getDefaultSystemLanguage() {
        return SystemLanguage::AmericanEnglish;
    }
    static constexpr std::string_view getDefaultSystemLocation() {
        return "auto";
    }
    static constexpr std::string_view getDefaultFirmwarePath() { return ""; }
    std::string getDefaultSdCardPath() const {
        return fmt::format("{}/sdmc", app_data_path);
    }
    std::string getDefaultSavePath() const {
        return fmt::format("{}/save", app_data_path);
    }
    std::string getDefaultSysmodulesPath() const {
        return fmt::format("{}/sysmodules", app_data_path);
    }
    static bool getDefaultHandheldMode() { return true; }
    static LogOutput getDefaultLogOutput() { return LogOutput::File; }
    static bool getDefaultLogFsAccess() { return false; }
    static bool getDefaultDebugLogging() { return false; }
    static std::vector<std::string> getDefaultProcessArgs() { return {}; }
    static bool getDefaultRecoverFromSegfault() { return false; }
    static bool getDefaultGdbEnabled() { return false; }
    static u16 getDefaultGdbPort() { return 1234; }
    static bool getDefaultGdbWaitForClient() { return false; }

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
    std::string device_nickname;
    SystemLanguage system_language;
    std::string system_location;
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
    REF_GETTER(game_paths, getGamePaths);
    REF_GETTER(loader_plugins, getLoaderPlugins);
    REF_GETTER(patch_paths, getPatchPaths);
    REF_GETTER(input_backend, getInputBackend);
    REF_GETTER(input_profiles, getInputProfiles);
    REF_GETTER(cpu_backend, getCpuBackend);
    REF_GETTER(gpu_renderer, getGpuRenderer);
    REF_GETTER(shader_backend, getShaderBackend);
    REF_GETTER(display_resolution, getDisplayResolution);
    REF_GETTER(custom_display_resolution, getCustomDisplayResolution);
    REF_GETTER(audio_backend, getAudioBackend);
    REF_GETTER(user_id, getUserId);
    REF_GETTER(device_nickname, getDeviceNickname);
    REF_GETTER(system_language, getSystemLanguage);
    REF_GETTER(system_location, getSystemLocation);
    REF_GETTER(firmware_path, getFirmwarePath);
    REF_GETTER(sd_card_path, getSdCardPath);
    REF_GETTER(save_path, getSavePath);
    REF_GETTER(sysmodules_path, getSysmodulesPath);
    REF_GETTER(handheld_mode, getHandheldMode);
    REF_GETTER(log_output, getLogOutput);
    REF_GETTER(log_fs_access, getLogFsAccess);
    REF_GETTER(debug_logging, getDebugLogging);
    REF_GETTER(process_args, getProcessArgs);
    REF_GETTER(recover_from_segfault, getRecoverFromSegfault);
    REF_GETTER(gdb_enabled, getGdbEnabled);
    REF_GETTER(gdb_port, getGdbPort);
    REF_GETTER(gdb_wait_for_client, getGdbWaitForClient);
};

} // namespace hydra

ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, InputBackend, AppleGameController,
                                   "Apple GameController", Sdl, "SDL")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, CpuBackend, AppleHypervisor,
                                   "Apple Hypervisor", Dynarmic, "dynarmic")
ENABLE_ENUM_FORMATTING_AND_CASTING(hydra, GpuRenderer, Null, "Null", Metal,
                                   "Metal")
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
