#include "common/config.hpp"

#include "common/toml_helper.hpp"

TOML11_DEFINE_CONVERSION_ENUM(hydra::InputBackend, AppleGameController,
                              "Apple GameController", Sdl, "SDL")
TOML11_DEFINE_CONVERSION_ENUM(hydra::CpuBackend, AppleHypervisor,
                              "Apple Hypervisor", Dynarmic, "dynarmic")
TOML11_DEFINE_CONVERSION_ENUM(hydra::GpuRenderer, Null, "null", Metal, "Metal")
TOML11_DEFINE_CONVERSION_ENUM(hydra::ShaderBackend, Msl, "MSL", Air, "AIR")
TOML11_DEFINE_CONVERSION_ENUM(hydra::Resolution, Auto, "auto", _720p, "720p",
                              _1080p, "1080p", _1440p, "1440p", _2160p, "2160p",
                              _4320p, "4320p", AutoExact, "Auto exact", Custom,
                              "custom")
TOML11_DEFINE_CONVERSION_ENUM(hydra::AudioBackend, Null, "null", Cubeb, "Cubeb")
TOML11_DEFINE_CONVERSION_ENUM(
    hydra::SystemLanguage, AmericanEnglish, "American English", BritishEnglish,
    "British English", Japanese, "Japanese", French, "French", German, "German",
    LatinAmericanSpanish, "Latin American Spanish", Spanish, "Spanish", Italian,
    "Italian", Dutch, "Dutch", CanadianFrench, "Canadian French", Portuguese,
    "Portuguese", Russian, "Russian", Korean, "Korean", TraditionalChinese,
    "Traditional Chinese", SimplifiedChinese, "Simplified Chinese",
    BrazilianPortuguese, "Brazilian Portuguese", Polish, "Polish", Thai, "Thai")
TOML11_DEFINE_CONVERSION_ENUM(hydra::LogOutput, None, "none", StdOut, "stdout",
                              File, "file")

ENABLE_STRUCT_FORMATTING_AND_TOML11(hydra::LoaderPlugin, path, options)

namespace toml {

using namespace hydra;

template <>
struct from<CustomResolution> {
    template <typename TC>
    static CustomResolution from_toml(const basic_value<TC>& v) {
        const auto& str = v.as_string();
        const auto x_pos = str.find('x');
        if (x_pos == std::string::npos)
            LOG_FATAL(Other, "Invalid custom display resolution {}", str);

        uint2 res;
        if (!strToNum(std::string_view(str).substr(0, x_pos), res.x()))
            LOG_FATAL(Other, "Invalid custom display resolution {}", str);
        if (!strToNum(std::string_view(str).substr(x_pos + 1), res.y()))
            LOG_FATAL(Other, "Invalid custom display resolution {}", str);

        return {res};
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
#ifdef ZTD_PLATFORM_APPLE
    if (const char* home = std::getenv("HOME")) {
        app_data_path =
            fmt::format("{}/Library/Application Support/" APP_NAME, home);
        logs_path = fmt::format("{}/Library/Logs/" APP_NAME, home);
        pictures_path = fmt::format("{}/Pictures/" APP_NAME, home);
    } else {
        LOG_FATAL(Other, "Failed to find HOME path");
    }
#elifdef ZTD_PLATFORM_WINDOWS
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
#elifdef ZTD_PLATFORM_LINUX
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
#ifndef ZTD_PLATFORM_IOS
    std::filesystem::create_directories(pictures_path);
#endif

    loadDefaults();
    deserialize();

    // Create directories
    std::filesystem::create_directories(sd_card_path);
    std::filesystem::create_directories(save_path);
    std::filesystem::create_directories(sysmodules_path);
}

void Config::loadDefaults() {
    game_paths = getDefaultGamePaths();
    loader_plugins = getDefaultLoaderPlugins();
    patch_paths = getDefaultPatchPaths();
    input_backend = getDefaultInputBackend();
    input_profiles = getDefaultInputProfiles();
    cpu_backend = getDefaultCpuBackend();
    gpu_renderer = getDefaultGpuRenderer();
    shader_backend = getDefaultShaderBackend();
    display_resolution = getDefaultDisplayResolution();
    custom_display_resolution = getDefaultCustomDisplayResolution();
    audio_backend = getDefaultAudioBackend();
    user_id = getDefaultUserId();
    device_nickname = getDefaultDeviceNickname();
    system_language = getDefaultSystemLanguage();
    system_location = getDefaultSystemLocation();
    firmware_path = getDefaultFirmwarePath();
    sd_card_path = getDefaultSdCardPath();
    save_path = getDefaultSavePath();
    sysmodules_path = getDefaultSysmodulesPath();
    handheld_mode = getDefaultHandheldMode();
    log_output = getDefaultLogOutput();
    log_fs_access = getDefaultLogFsAccess();
    debug_logging = getDefaultDebugLogging();
    process_args = getDefaultProcessArgs();
    recover_from_segfault = getDefaultRecoverFromSegfault();
    gdb_enabled = getDefaultGdbEnabled();
    gdb_port = getDefaultGdbPort();
    gdb_wait_for_client = getDefaultGdbWaitForClient();
}

void Config::serialize() {
    // TODO: check if changed?

    // TODO: why is the order of everything reversed in the saved config?

    std::ofstream config_file(getConfigPath());
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
        input["backend"] = input_backend;
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
        system["device_nickname"] = device_nickname;
        system["system_language"] = system_language;
        system["system_location"] = system_location;
        system["firmware_path"] = firmware_path;
        if (sd_card_path != getDefaultSdCardPath())
            system["sd_card_path"] = sd_card_path;
        if (save_path != getDefaultSavePath())
            system["save_path"] = save_path;
        if (sysmodules_path != getDefaultSysmodulesPath())
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

void Config::deserialize() {
    const std::string path = getConfigPath();

    // Check if exists
    bool exists = std::filesystem::exists(path);
    if (!exists) {
        loadDefaults();
        serialize();
        return;
    }

    auto data = toml::parse(path);

    if (data.contains("General")) {
        const auto& general = data.at("General");
        game_paths = toml::find_or<std::vector<std::string>>(
            general, "game_paths", getDefaultGamePaths());
        loader_plugins = toml::find_or<std::vector<LoaderPlugin>>(
            general, "loader_plugins", getDefaultLoaderPlugins());
        patch_paths = toml::find_or<std::vector<std::string>>(
            general, "patch_paths", getDefaultPatchPaths());
    }
    if (data.contains("Input")) {
        const auto& input = data.at("Input");
        input_backend = toml::find_or<std::optional<InputBackend>>(
                            input, "backend", getDefaultInputBackend())
                            .value_or(getDefaultInputBackend());
        input_profiles = toml::find_or<std::vector<std::string>>(
            input, "profiles", getDefaultInputProfiles());
    }
    if (data.contains("CPU")) {
        const auto& cpu = data.at("CPU");
        cpu_backend = toml::find_or<std::optional<CpuBackend>>(
                          cpu, "backend", getDefaultCpuBackend())
                          .value_or(getDefaultCpuBackend());
    }
    if (data.contains("Graphics")) {
        const auto& graphics = data.at("Graphics");
        gpu_renderer = toml::find_or<std::optional<GpuRenderer>>(
                           graphics, "renderer", getDefaultGpuRenderer())
                           .value_or(getDefaultGpuRenderer());
        shader_backend =
            toml::find_or<std::optional<ShaderBackend>>(
                graphics, "shader_backend", getDefaultShaderBackend())
                .value_or(getDefaultShaderBackend());
        display_resolution =
            toml::find_or<std::optional<Resolution>>(
                graphics, "display_resolution", getDefaultDisplayResolution())
                .value_or(getDefaultDisplayResolution());
        custom_display_resolution = toml::find_or<CustomResolution>(
            graphics, "custom_display_resolution",
            getDefaultCustomDisplayResolution());
    }
    if (data.contains("Audio")) {
        const auto& audio = data.at("Audio");
        audio_backend = toml::find_or<std::optional<AudioBackend>>(
                            audio, "backend", getDefaultAudioBackend())
                            .value_or(getDefaultAudioBackend());
    }
    if (data.contains("User")) {
        const auto& user = data.at("User");
        user_id = toml::find_or<uuid_t>(user, "user_id", getDefaultUserId());
    }
    if (data.contains("System")) {
        const auto& system = data.at("System");
        device_nickname = toml::find_or<std::string_view>(
            system, "device_nickname", getDefaultDeviceNickname());
        system_language =
            toml::find_or<std::optional<SystemLanguage>>(
                system, "system_language", getDefaultSystemLanguage())
                .value_or(getDefaultSystemLanguage());
        system_location = toml::find_or<std::string_view>(
            system, "system_location", getDefaultSystemLocation());
        firmware_path = toml::find_or<std::string_view>(
            system, "firmware_path", getDefaultFirmwarePath());
        sd_card_path = toml::find_or<std::string>(system, "sd_card_path",
                                                  getDefaultSdCardPath());
        save_path = toml::find_or<std::string>(system, "save_path",
                                               getDefaultSavePath());
        sysmodules_path = toml::find_or<std::string>(
            system, "sysmodules_path", getDefaultSysmodulesPath());
        handheld_mode = toml::find_or<bool>(system, "handheld_mode",
                                            getDefaultHandheldMode());
    }
    if (data.contains("Debug")) {
        const auto& debug = data.at("Debug");
        log_output = toml::find_or<std::optional<LogOutput>>(
                         debug, "log_output", getDefaultLogOutput())
                         .value_or(getDefaultLogOutput());
        log_fs_access = toml::find_or<bool>(debug, "log_fs_access",
                                            getDefaultLogFsAccess());
        debug_logging = toml::find_or<bool>(debug, "debug_logging",
                                            getDefaultDebugLogging());
        process_args = toml::find_or<std::vector<std::string>>(
            debug, "process_args", getDefaultProcessArgs());
        recover_from_segfault = toml::find_or<bool>(
            debug, "recover_from_segfault", getDefaultRecoverFromSegfault());
        gdb_enabled =
            toml::find_or<bool>(debug, "gdb_enabled", getDefaultGdbEnabled());
        gdb_port = toml::find_or<u16>(debug, "gdb_port", getDefaultGdbPort());
        gdb_wait_for_client = toml::find_or<bool>(debug, "gdb_wait_for_client",
                                                  getDefaultGdbWaitForClient());
    }
}

void Config::log() {
    LOG_INFO(Other, "Game paths: [{}]", fmt::join(game_paths, ", "));
    LOG_INFO(Other, "Loader plugins: [{}]", fmt::join(loader_plugins, ", "));
    LOG_INFO(Other, "Patch paths: [{}]", fmt::join(patch_paths, ", "));
    LOG_INFO(Other, "Input backend: {}", input_backend);
    LOG_INFO(Other, "Input profiles: [{}]", fmt::join(input_profiles, ", "));
    LOG_INFO(Other, "CPU backend: {}", cpu_backend);
    LOG_INFO(Other, "Gpu renderer: {}", gpu_renderer);
    LOG_INFO(Other, "Shader backend: {}", shader_backend);
    LOG_INFO(Other, "Display resolution: {}", display_resolution);
    LOG_INFO(Other, "Custom display resolution: {}x{}",
             custom_display_resolution.x(), custom_display_resolution.y());
    LOG_INFO(Other, "Audio backend: {}", audio_backend);
    LOG_INFO(Other, "User ID: {:032x}", user_id);
    LOG_INFO(Other, "Device nickname: {}", device_nickname);
    LOG_INFO(Other, "System language: {}", system_language);
    LOG_INFO(Other, "System location: {}", system_location);
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
