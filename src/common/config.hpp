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

  private:
    std::string app_data_path;
    std::string logs_path;
    std::string pictures_path; // TODO: remove this

    // Config
    std::vector<std::string> game_paths;
    std::vector<LoaderPlugin> loader_plugins;
    std::vector<std::string> patch_paths;
    std::vector<std::string> input_profiles;
    CpuBackend cpu_backend;
    GpuRenderer gpu_renderer;
    ShaderBackend shader_backend;
    Resolution display_resolution;
    uint2 custom_display_resolution;
    AudioBackend audio_backend;
    uuid_t user_id;
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

    // Default values
    std::vector<std::string> GetDefaultGamePaths() const { return {}; }
    std::vector<LoaderPlugin> GetDefaultLoaderPlugins() const { return {}; }
    std::vector<std::string> GetDefaultPatchPaths() const { return {}; }
    std::vector<std::string> GetDefaultInputProfiles() const {
        return {"Default", "", "", "", "", "", "", "", ""};
    }
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
    bool GetDefaultRecoverFromSegfault() const { return false; }
    bool GetDefaultGdbEnabled() const { return false; }
    u16 GetDefaultGdbPort() const { return 1234; }
    bool GetDefaultGdbWaitForClient() const { return false; }

  public:
    REF_GETTER(game_paths, GetGamePaths);
    REF_GETTER(loader_plugins, GetLoaderPlugins);
    REF_GETTER(patch_paths, GetPatchPaths);
    REF_GETTER(input_profiles, GetInputProfiles);
    REF_GETTER(cpu_backend, GetCpuBackend);
    REF_GETTER(gpu_renderer, GetGpuRenderer);
    REF_GETTER(shader_backend, GetShaderBackend);
    REF_GETTER(display_resolution, GetDisplayResolution);
    REF_GETTER(custom_display_resolution, GetCustomDisplayResolution);
    REF_GETTER(audio_backend, GetAudioBackend);
    REF_GETTER(user_id, GetUserId);
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
