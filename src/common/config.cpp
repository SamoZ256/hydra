#include "common/config.hpp"

#include "common/log.hpp"
#include "common/platform.hpp"
#include "common/toml_helper.hpp"

TOML11_DEFINE_CONVERSION_ENUM(hydra::CpuBackend, AppleHypervisor,
                              "Apple Hypervisor", Dynarmic, "dynarmic")
TOML11_DEFINE_CONVERSION_ENUM(hydra::GpuRenderer, Metal, "Metal")
TOML11_DEFINE_CONVERSION_ENUM(hydra::ShaderBackend, Msl, "MSL", Air, "AIR")
TOML11_DEFINE_CONVERSION_ENUM(hydra::Resolution, Auto, "auto", _720p, "720p",
                              _1080p, "1080p", _1440p, "1440p", _2160p, "2160p",
                              _4320p, "4320p", AutoExact, "Auto exact", Custom,
                              "custom")
TOML11_DEFINE_CONVERSION_ENUM(hydra::AudioBackend, Null, "Null", Cubeb, "Cubeb")
TOML11_DEFINE_CONVERSION_ENUM(hydra::LogOutput, None, "none", StdOut, "stdout",
                              File, "file")

ENABLE_STRUCT_FORMATTING_AND_TOML11(hydra::LoaderPlugin, path, options)

namespace toml {

using namespace hydra;

template <>
struct from<CustomResolution> {
    template <typename TC>
    static CustomResolution from_toml(const basic_value<TC>& v) {
        const auto str = v.as_string();
        const auto x_pos = str.find('x');
        if (x_pos == std::string::npos)
            LOG_FATAL(Other, "Invalid custom display resolution {}", str);

        uint2 res;
        if (!str_to_num(std::string_view(str).substr(0, x_pos), res.x()))
            LOG_FATAL(Other, "Invalid custom display resolution {}", str);
        if (!str_to_num(std::string_view(str).substr(x_pos + 1), res.y()))
            LOG_FATAL(Other, "Invalid custom display resolution {}", str);

        return CustomResolution(res);
    }
};

template <>
struct into<CustomResolution> {
    template <typename TC>
    static basic_value<TC> into_toml(const CustomResolution& obj) {
        return toml::value(
            fmt::format("{}x{}", hydra::uint2(obj).x(), hydra::uint2(obj).y()));
    }
};

} // namespace toml

namespace hydra {

Config::Config() {
#ifdef PLATFORM_APPLE
    if (const char* home = std::getenv("HOME")) {
        app_data_path =
            fmt::format("{}/Library/Application Support/" APP_NAME, home);
        logs_path = fmt::format("{}/Library/Logs/" APP_NAME, home);
        pictures_path = fmt::format("{}/Pictures/" APP_NAME, home);
    } else {
        LOG_FATAL(Other, "Failed to find HOME path");
    }
#elif defined(PLATFORM_WINDOWS)
    if (const char* app_data = std::getenv("APPDATA")) {
        app_data_path = fmt::format("{}/" APP_NAME, app_data);
        logs_path = fmt::format("{}/logs", app_data_path); // TODO
    } else {
        LOG_FATAL(Other, "Failed to find APPDATA path");
    }

    if (const char* user_profile = std::getenv("USERPROFILE")) {
        pictures_path = fmt::format("{}/Pictures", user_profile);
    } else {
        LOG_FATAL(Other, "Failed to find USERPROFILE path");
    }
#elif defined(PLATFORM_LINUX)
    if (const char* xdg_config = std::getenv("XDG_CONFIG_HOME")) {
        app_data_path = fmt::format("{}/" APP_NAME, xdg_config);
        logs_path = fmt::format("{}/logs", app_data_path);
    } else if (const char* home = std::getenv("HOME")) {
        app_data_path = fmt::format("{}/.config/" APP_NAME, home);
        logs_path = fmt::format("{}/logs", app_data_path);
    } else {
        LOG_FATAL(Other, "Failed to find HOME path");
    }

    if (const char* xdg_pictures = std::getenv("XDG_PICTURES_DIR")) {
        pictures_path = fmt::format("{}/" APP_NAME, xdg_pictures);
    } else if (const char* home = std::getenv("HOME")) {
        pictures_path = fmt::format("{}/Pictures", home);
    } else {
        LOG_FATAL(Other, "Failed to find HOME path");
    }
#else
#error "Unsupported platform"
#endif

    // Create directories
    std::filesystem::create_directories(app_data_path);
    std::filesystem::create_directories(logs_path);
    // HACK
#ifndef PLATFORM_IOS
    std::filesystem::create_directories(pictures_path);
#endif

    LoadDefaults();
    Deserialize();

    // Create directories
    std::filesystem::create_directories(sd_card_path);
    std::filesystem::create_directories(save_path);
    std::filesystem::create_directories(sysmodules_path);
}

void Config::LoadDefaults() {
    game_paths = GetDefaultGamePaths();
    loader_plugins = GetDefaultLoaderPlugins();
    patch_paths = GetDefaultPatchPaths();
    input_profiles = GetDefaultInputProfiles();
    cpu_backend = GetDefaultCpuBackend();
    gpu_renderer = GetDefaultGpuRenderer();
    shader_backend = GetDefaultShaderBackend();
    display_resolution = GetDefaultDisplayResolution();
    custom_display_resolution = GetDefaultCustomDisplayResolution();
    audio_backend = GetDefaultAudioBackend();
    user_id = GetDefaultUserID();
    firmware_path = GetDefaultFirmwarePath();
    sd_card_path = GetDefaultSdCardPath();
    save_path = GetDefaultSavePath();
    sysmodules_path = GetDefaultSysmodulesPath();
    handheld_mode = GetDefaultHandheldMode();
    log_output = GetDefaultLogOutput();
    log_fs_access = GetDefaultLogFsAccess();
    debug_logging = GetDefaultDebugLogging();
    process_args = GetDefaultProcessArgs();
    recover_from_segfault = GetDefaultRecoverFromSegfault();
    gdb_enabled = GetDefaultGdbEnabled();
    gdb_port = GetDefaultGdbPort();
    gdb_wait_for_client = GetDefaultGdbWaitForClient();
}

void Config::Serialize() {
    // TODO: check if changed?

    // TODO: why is the order of everything reversed in the saved config?

    std::ofstream config_file(GetConfigPath());
    if (!config_file.is_open()) {
        LOG_ERROR(Common, "Failed to open config file");
        return;
    }

    toml::value data(toml::table{
        {"General", toml::table{}},
        {"Input", toml::table{}},
        {"CPU", toml::table{}},
        {"Graphics", toml::table{}},
        {"Audio", toml::table{}},
        {"User", toml::table{}},
        {"System", toml::table{}},
        {"Debug", toml::table{}},
    });

    {
        auto& general = data.at("General");
        general["game_paths"] = game_paths;
        general["loader_plugins"] = loader_plugins;
        general["patch_paths"] = patch_paths;
    }

    {
        auto& input = data.at("Input");
        input["profiles"] = input_profiles;
    }

    {
        auto& cpu = data.at("CPU");
        cpu["backend"] = cpu_backend;
    }

    {
        auto& graphics = data.at("Graphics");
        graphics["renderer"] = gpu_renderer;
        graphics["shader_backend"] = shader_backend;
        graphics["display_resolution"] = display_resolution;
        graphics["custom_display_resolution"] =
            CustomResolution(custom_display_resolution);
    }

    {
        auto& audio = data.at("Audio");
        audio["backend"] = audio_backend;
    }

    {
        auto& user = data.at("User");
        user["user_id"] = static_cast<u32>(user_id); // HACK
    }

    {
        auto& system = data.at("System");
        system["firmware_path"] = firmware_path;
        if (sd_card_path != GetDefaultSdCardPath())
            system["sd_card_path"] = sd_card_path;
        if (save_path != GetDefaultSavePath())
            system["save_path"] = save_path;
        if (sysmodules_path != GetDefaultSysmodulesPath())
            system["sysmodules_path"] = sysmodules_path;
        system["handheld_mode"] = handheld_mode;
    }

    {
        auto& debug = data.at("Debug");
        debug["log_output"] = log_output;
        debug["log_fs_access"] = log_fs_access;
        debug["debug_logging"] = debug_logging;
        debug["process_args"] = process_args;
        debug["recover_from_segfault"] = recover_from_segfault;
        debug["gdb_enabled"] = gdb_enabled;
        debug["gdb_port"] = gdb_port;
        debug["gdb_wait_for_client"] = gdb_wait_for_client;
    }

    config_file << toml::format(data);
    config_file.close();
}

void Config::Deserialize() {
    const std::string path = GetConfigPath();

    // Check if exists
    bool exists = std::filesystem::exists(path);
    if (!exists) {
        LoadDefaults();
        Serialize();
        return;
    }

    auto data = toml::parse(path);

    if (data.contains("General")) {
        const auto& general = data.at("General");
        game_paths = toml::find_or<std::vector<std::string>>(
            general, "game_paths", GetDefaultGamePaths());
        loader_plugins = toml::find_or<std::vector<LoaderPlugin>>(
            general, "loader_plugins", GetDefaultLoaderPlugins());
        patch_paths = toml::find_or<std::vector<std::string>>(
            general, "patch_paths", GetDefaultPatchPaths());
    }
    if (data.contains("Input")) {
        const auto& input = data.at("Input");
        input_profiles = toml::find_or<std::vector<std::string>>(
            input, "profiles", GetDefaultInputProfiles());
    }
    if (data.contains("CPU")) {
        const auto& cpu = data.at("CPU");
        cpu_backend =
            toml::find_or<CpuBackend>(cpu, "backend", GetDefaultCpuBackend());
    }
    if (data.contains("Graphics")) {
        const auto& graphics = data.at("Graphics");
        gpu_renderer = toml::find_or<GpuRenderer>(graphics, "renderer",
                                                  GetDefaultGpuRenderer());
        shader_backend = toml::find_or<ShaderBackend>(
            graphics, "shader_backend", GetDefaultShaderBackend());
        display_resolution = toml::find_or<Resolution>(
            graphics, "display_resolution", GetDefaultDisplayResolution());
        custom_display_resolution = toml::find_or<CustomResolution>(
            graphics, "custom_display_resolution",
            GetDefaultCustomDisplayResolution());
    }
    if (data.contains("Audio")) {
        const auto& audio = data.at("Audio");
        audio_backend = toml::find_or<AudioBackend>(audio, "backend",
                                                    GetDefaultAudioBackend());
    }
    if (data.contains("User")) {
        const auto& user = data.at("User");
        user_id = toml::find_or<uuid_t>(user, "user_id", GetDefaultUserID());
    }
    if (data.contains("System")) {
        const auto& system = data.at("System");
        firmware_path = toml::find_or<std::string>(system, "firmware_path",
                                                   GetDefaultFirmwarePath());
        sd_card_path = toml::find_or<std::string>(system, "sd_card_path",
                                                  GetDefaultSdCardPath());
        save_path = toml::find_or<std::string>(system, "save_path",
                                               GetDefaultSavePath());
        sysmodules_path = toml::find_or<std::string>(
            system, "sysmodules_path", GetDefaultSysmodulesPath());
        handheld_mode = toml::find_or<bool>(system, "handheld_mode",
                                            GetDefaultHandheldMode());
    }
    if (data.contains("Debug")) {
        const auto& debug = data.at("Debug");
        log_output = toml::find_or<LogOutput>(debug, "log_output",
                                              GetDefaultLogOutput());
        log_fs_access = toml::find_or<bool>(debug, "log_fs_access",
                                            GetDefaultLogFsAccess());
        debug_logging = toml::find_or<bool>(debug, "debug_logging",
                                            GetDefaultDebugLogging());
        process_args = toml::find_or<std::vector<std::string>>(
            debug, "process_args", GetDefaultProcessArgs());
        recover_from_segfault = toml::find_or<bool>(
            debug, "recover_from_segfault", GetDefaultRecoverFromSegfault());
        gdb_enabled =
            toml::find_or<bool>(debug, "gdb_enabled", GetDefaultGdbEnabled());
        gdb_port = toml::find_or<u16>(debug, "gdb_port", GetDefaultGdbPort());
        gdb_wait_for_client = toml::find_or<bool>(debug, "gdb_wait_for_client",
                                                  GetDefaultGdbWaitForClient());
    }

    // Validate
    if (cpu_backend == CpuBackend::Invalid) {
        cpu_backend = GetDefaultCpuBackend();
        LOG_WARN(Other, "Invalid CPU backend, falling back to {}", cpu_backend);
    }

    if (gpu_renderer == GpuRenderer::Invalid) {
        gpu_renderer = GetDefaultGpuRenderer();
        LOG_WARN(Other, "Invalid Gpu renderer, falling back to {}",
                 gpu_renderer);
    }

    if (shader_backend == ShaderBackend::Invalid) {
        shader_backend = GetDefaultShaderBackend();
        LOG_WARN(Other, "Invalid shader backend, falling back to {}",
                 shader_backend);
    } else if (shader_backend == ShaderBackend::Air) {
        LOG_ERROR(Other, "AIR shader backend is not functional");
    }

    if (display_resolution == Resolution::Invalid) {
        display_resolution = GetDefaultDisplayResolution();
        LOG_WARN(Other, "Invalid display resolution, falling back to {}",
                 display_resolution);
    }

    if (audio_backend == AudioBackend::Invalid) {
        audio_backend = GetDefaultAudioBackend();
        LOG_WARN(Other, "Invalid audio backend, falling back to {}",
                 audio_backend);
    }
}

void Config::Log() {
    LOG_INFO(Other, "Game paths: [{}]", fmt::join(game_paths, ", "));
    LOG_INFO(Other, "Loader plugins: [{}]", fmt::join(loader_plugins, ", "));
    LOG_INFO(Other, "Patch paths: [{}]", fmt::join(patch_paths, ", "));
    LOG_INFO(Other, "Input profiles: [{}]", fmt::join(input_profiles, ", "));
    LOG_INFO(Other, "CPU backend: {}", cpu_backend);
    LOG_INFO(Other, "Gpu renderer: {}", gpu_renderer);
    LOG_INFO(Other, "Shader backend: {}", shader_backend);
    LOG_INFO(Other, "Display resolution: {}", display_resolution);
    LOG_INFO(Other, "Custom display resolution: {}x{}",
             custom_display_resolution.x(), custom_display_resolution.y());
    LOG_INFO(Other, "Audio backend: {}", audio_backend);
    LOG_INFO(Other, "User ID: {:032x}", user_id);
    LOG_INFO(Other, "Firmware path: {}", firmware_path);
    LOG_INFO(Other, "SD card path: {}", sd_card_path);
    LOG_INFO(Other, "Save path: {}", save_path);
    LOG_INFO(Other, "Sysmodules path: {}", sysmodules_path);
    LOG_INFO(Other, "Handheld mode: {}", handheld_mode);
    LOG_INFO(Other, "Log output: {}", log_output);
    LOG_INFO(Other, "Log FS access: {}", log_fs_access);
    LOG_INFO(Other, "Debug logging: {}", debug_logging);
    LOG_INFO(Other, "Process arguments: {}", process_args);
    LOG_INFO(Other, "Recover from segfault: {}", recover_from_segfault);
    LOG_INFO(Other, "GDB enabled: {}", gdb_enabled);
    LOG_INFO(Other, "GDB port: {}", gdb_port);
    LOG_INFO(Other, "GDB wait for client: {}", gdb_wait_for_client);
}

} // namespace hydra
