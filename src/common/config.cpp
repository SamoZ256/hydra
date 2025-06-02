#include "common/config.hpp"

#include <toml.hpp>

#include "common/log.hpp"

#define TOML11_CONVERSION_TOML_TO_ENUM_CASE(e, val, n)                         \
    if (v.as_string() == n)                                                    \
        return e::val;

#define TOML11_CONVERSION_ENUM_TO_TOML_CASE(e, val, n)                         \
    case e::val:                                                               \
        return toml::value(n);

#define TOML11_DEFINE_CONVERSION_ENUM(e, ...)                                  \
    namespace toml {                                                           \
    template <> struct from<e> {                                               \
        template <typename TC> static e from_toml(const basic_value<TC>& v) {  \
            FOR_EACH_1_2(TOML11_CONVERSION_TOML_TO_ENUM_CASE, e, __VA_ARGS__)  \
            return e::Invalid;                                                 \
        }                                                                      \
    };                                                                         \
    template <> struct into<e> {                                               \
        template <typename TC>                                                 \
        static basic_value<TC> into_toml(const e& obj) {                       \
            switch (obj) {                                                     \
                FOR_EACH_1_2(TOML11_CONVERSION_ENUM_TO_TOML_CASE, e,           \
                             __VA_ARGS__)                                      \
            default:                                                           \
                return toml::value("invalid");                                 \
            }                                                                  \
        }                                                                      \
    };                                                                         \
    } /* toml */

#define ENABLE_ENUM_FORMATTING_CASTING_AND_TOML11(namespc, e, e_lower_case,    \
                                                  ...)                         \
    ENABLE_ENUM_FORMATTING_AND_CASTING(namespc, e, e_lower_case, __VA_ARGS__)  \
    TOML11_DEFINE_CONVERSION_ENUM(namespc::e, __VA_ARGS__)

#define ENABLE_STRUCT_FORMATTING_AND_TOML11(s, ...)                            \
    ENABLE_STRUCT_FORMATTING(s, __VA_ARGS__)                                   \
    TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(s, __VA_ARGS__)

TOML11_DEFINE_CONVERSION_ENUM(hydra::CpuBackend, AppleHypervisor,
                              "Apple Hypervisor", Dynarmic, "dynarmic")
TOML11_DEFINE_CONVERSION_ENUM(hydra::GpuRenderer, Metal, "Metal")
TOML11_DEFINE_CONVERSION_ENUM(hydra::ShaderBackend, Msl, "MSL", Air, "AIR")
TOML11_DEFINE_CONVERSION_ENUM(hydra::AudioBackend, Null, "Null", Cubeb, "Cubeb")
TOML11_DEFINE_CONVERSION_ENUM(hydra::Output, StdOut, "stdout", File, "file")

namespace hydra {

void Config::Initialize() {
#ifdef __APPLE__
    // macOS
    if (const char* home = std::getenv("HOME")) {
        app_data_path =
            fmt::format("{}/Library/Application Support/" APP_NAME, home);
        logs_path = fmt::format("{}/Library/Logs/" APP_NAME, home);
    } else {
        LOG_FATAL(Other, "Failed to find HOME path");
    }
#elif defined(_WIN32)
    // Windows
    if (const char* app_data = std::getenv("APPDATA")) {
        app_data_path = fmt::format("{}/" APP_NAME, app_data);
        logs_path = app_data_path; // TODO
    } else {
        LOG_FATAL(Other, "Failed to find APPDATA path");
    }
#else
    // Linux and other Unix-like systems
    if (const char* xdg_config = std::getenv("XDG_CONFIG_HOME")) {
        app_data_path = fmt::format("{}/" APP_NAME, xdg_config);
        logs_path = app_data_path; // TODO
    } else if (const char* home = std::getenv("HOME")) {
        app_data_path = fmt::format("{}/.config/" APP_NAME, home);
        logs_path = app_data_path; // TODO
    } else {
        LOG_FATAL(Other, "Failed to find HOME path");
    }
#endif

    // Create directories
    std::filesystem::create_directories(app_data_path);
    std::filesystem::create_directories(logs_path);

    LoadDefaults();
    Deserialize();
}

void Config::LoadDefaults() {
    game_paths = GetDefaultGamePaths();
    patch_paths = GetDefaultPatchPaths();
    sd_card_path = GetDefaultSdCardPath();
    save_path = GetDefaultSavePath();
    cpu_backend = GetDefaultCpuBackend();
    gpu_renderer = GetDefaultGpuRenderer();
    shader_backend = GetDefaultShaderBackend();
    audio_backend = GetDefaultAudioBackend();
    user_id = GetDefaultUserID();
    logging_output = GetDefaultLoggingOutput();
    log_fs_access = GetDefaultLogFsAccess();
    debug_logging = GetDefaultDebugLogging();
    stack_trace_logging = GetDefaultStackTraceLogging();
    process_args = GetDefaultProcessArgs();
}

void Config::Serialize() {
    // TODO: check if changed?

    // TODO: why is the order of everything reversed in the saved config?

    std::ofstream config_file(GetConfigPath());
    if (config_file.is_open()) {
        toml::value data(toml::table{
            {"General", toml::table{}},
            {"CPU", toml::table{}},
            {"Graphics", toml::table{}},
            {"User", toml::table{}},
            {"Debug", toml::table{}},
        });

        {
            auto& general = data.at("General");

            auto& game_paths_arr = general["game_paths"];
            game_paths_arr = toml::array{};
            game_paths_arr.as_array().assign(game_paths.Get().begin(),
                                             game_paths.Get().end());

            auto& patch_paths_arr = general["patch_paths"];
            patch_paths_arr = toml::array{};
            patch_paths_arr.as_array().assign(patch_paths.Get().begin(),
                                              patch_paths.Get().end());

            general["sd_card_path"] = sd_card_path;
            general["save_path"] = save_path;
        }

        {
            auto& cpu = data.at("CPU");
            cpu["backend"] = cpu_backend.Get();
        }

        {
            auto& graphics = data.at("Graphics");
            graphics["renderer"] = gpu_renderer.Get();
            graphics["shader_backend"] = shader_backend.Get();
        }

        {
            auto& audio = data.at("Audio");
            audio["backend"] = audio_backend.Get();
        }

        {
            auto& user = data.at("User");
            user["user_id"] = user_id.Get();
        }

        {
            auto& debug = data.at("Debug");
            debug["logging_output"] = logging_output.Get();
            debug["log_fs_access"] = log_fs_access.Get();
            debug["debug_logging"] = debug_logging.Get();
            debug["stack_trace_logging"] = stack_trace_logging.Get();
            debug["process_args"] = process_args.Get();
        }

        config_file << toml::format(data);
        config_file.close();
    } else {
        LOG_FATAL(Other, "Failed to create config file");
    }
}

void Config::Deserialize() {
    const std::string path = GetConfigPath();
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
        patch_paths = toml::find_or<std::vector<std::string>>(
            general, "patch_paths", GetDefaultPatchPaths());
        sd_card_path = toml::find_or<std::string>(general, "sd_card_path",
                                                  GetDefaultSdCardPath());
        save_path = toml::find_or<std::string>(general, "save_path",
                                               GetDefaultSavePath());
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
    if (data.contains("Debug")) {
        const auto& debug = data.at("Debug");
        logging_output = toml::find_or<Output>(debug, "logging_output",
                                               GetDefaultLoggingOutput());
        log_fs_access = toml::find_or<bool>(debug, "log_fs_access",
                                            GetDefaultLogFsAccess());
        debug_logging = toml::find_or<bool>(debug, "debug_logging",
                                            GetDefaultDebugLogging());
        stack_trace_logging = toml::find_or<bool>(
            debug, "stack_trace_logging", GetDefaultStackTraceLogging());
        process_args = toml::find_or<std::vector<std::string>>(
            debug, "process_args", GetDefaultProcessArgs());
    }

    // Validate
    if (cpu_backend == CpuBackend::Invalid) {
        cpu_backend = GetDefaultCpuBackend();
        LOG_WARN(Other, "Invalid CPU backend, falling back to {}", cpu_backend);
    }

    if (gpu_renderer == GpuRenderer::Invalid) {
        gpu_renderer = GetDefaultGpuRenderer();
        LOG_WARN(Other, "Invalid GPU renderer, falling back to {}",
                 gpu_renderer);
    }

    if (shader_backend == ShaderBackend::Invalid) {
        shader_backend = GetDefaultShaderBackend();
        LOG_WARN(Other, "Invalid shader backend, falling back to {}",
                 shader_backend);
    } else if (shader_backend == ShaderBackend::Air) {
        LOG_ERROR(Other, "AIR shader backend is not functional");
    }

    if (audio_backend == AudioBackend::Invalid) {
        audio_backend = GetDefaultAudioBackend();
        LOG_WARN(Other, "Invalid audio backend, falling back to {}",
                 audio_backend);
    }
}

void Config::Log() {
    LOG_INFO(Other, "Game paths: [{}]", game_paths);
    LOG_INFO(Other, "Patch paths: [{}]", patch_paths);
    LOG_INFO(Other, "SD card path: {}", sd_card_path);
    LOG_INFO(Other, "Save path: {}", save_path);
    LOG_INFO(Other, "CPU backend: {}", cpu_backend);
    LOG_INFO(Other, "GPU renderer: {}", gpu_renderer);
    LOG_INFO(Other, "Shader backend: {}", shader_backend);
    LOG_INFO(Other, "Audio backend: {}", audio_backend);
    LOG_INFO(Other, "User ID: {:032x}", user_id.Get());
    LOG_INFO(Other, "Logging output: {}", logging_output);
    LOG_INFO(Other, "Log FS access: {}", log_fs_access);
    LOG_INFO(Other, "Debug logging: {}", debug_logging);
    LOG_INFO(Other, "Log stack trace: {}", stack_trace_logging);
    LOG_INFO(Other, "Process arguments: {}", process_args);
}

} // namespace hydra
