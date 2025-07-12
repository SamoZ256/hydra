#pragma once

#include <mutex>

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ostream.h>

#include "common/macros.hpp"
#include "common/type_aliases.hpp"

#define LOGGER_INSTANCE Logger::GetInstance()

#define LOG(level, c, ...)                                                     \
    LOGGER_INSTANCE.Log(LogLevel::level, LogClass::c,                          \
                        trim_source_path(__FILE__), __LINE__, __func__,        \
                        __VA_ARGS__)

#ifdef HYDRA_DEBUG
#define LOG_DEBUG(c, ...)                                                      \
    {                                                                          \
        if (CONFIG_INSTANCE.GetDebugLogging())                                 \
            LOG(Debug, c, __VA_ARGS__);                                        \
    }
#else
#define LOG_DEBUG(c, ...)
#endif

#define LOG_INFO(c, ...) LOG(Info, c, __VA_ARGS__)
#define LOG_STUBBED(c, f, ...)                                                 \
    LOG(Stub, c, f " stubbed" PASS_VA_ARGS(__VA_ARGS__))
#define LOG_WARN(c, ...) LOG(Warning, c, __VA_ARGS__)
#define LOG_ERROR(c, ...) LOG(Error, c, __VA_ARGS__)
#define LOG_FATAL(c, ...)                                                      \
    {                                                                          \
        LOG(Fatal, c, __VA_ARGS__);                                            \
        abort();                                                               \
    }

#define LOG_FUNC_STUBBED(c) LOG_STUBBED(c, "{}", __func__)
#define LOG_NOT_IMPLEMENTED(c, f, ...)                                         \
    LOG_WARN(c, f " not implemented" PASS_VA_ARGS(__VA_ARGS__))
#define LOG_FUNC_NOT_IMPLEMENTED(c) LOG_NOT_IMPLEMENTED(c, "{}", __func__)

#define ASSERT(condition, c, ...)                                              \
    if (!(condition)) {                                                        \
        LOG_FATAL(c, __VA_ARGS__);                                             \
    }
#define DEBUGGER_ASSERT(condition, c, f, ...)                                  \
    if (!(condition)) {                                                        \
        /* TODO: log class */                                                  \
        DEBUGGER_INSTANCE.BreakOnThisThread(                                   \
            fmt::format(f PASS_VA_ARGS(__VA_ARGS__)));                         \
    }
#define ASSERT_ALIGNMENT(value, alignment, c, name)                            \
    ASSERT(is_aligned<decltype(value)>(value, alignment), c,                   \
           name " must be {}-byte aligned", alignment)

#ifdef HYDRA_DEBUG
#define ASSERT_DEBUG(condition, c, ...) ASSERT(condition, c, __VA_ARGS__)
#define DEBUGGER_ASSERT_DEBUG(condition, c, ...)                               \
    DEBUGGER_ASSERT(condition, c, __VA_ARGS__)
#else
// TODO: should we evaluate the condition anyway?
#define ASSERT_DEBUG(condition, c, ...)                                        \
    if (condition) {                                                           \
    }
#define DEBUGGER_ASSERT_DEBUG(condition, c, ...)                               \
    if (condition) {                                                           \
    }
#endif
#define ASSERT_ALIGNMENT_DEBUG(value, alignment, c, name)                      \
    ASSERT_ALIGNMENT(value, alignment, c, name)

#define INDENT_FMT "{:{}}"
#define PASS_INDENT(indent) "", ((indent)*4)

namespace hydra {

// From yuzu
constexpr const char* trim_source_path(std::string_view source) {
    const auto rfind = [source](const std::string_view match) {
        return source.rfind(match) == source.npos
                   ? 0
                   : (source.rfind(match) + match.size());
    };
    auto idx =
        std::max({rfind("src/"), rfind("src\\"), rfind("../"), rfind("..\\")});
    return source.data() + idx;
}

// TODO: move this to config
enum class LogOutput {
    Invalid = 0,

    None,
    StdOut,
    File,
};

enum class LogLevel {
    Debug,
    Info,
    Stub,
    Warning,
    Error,
    Fatal,
};

enum class LogClass {
    Common,
    Mmu,
    Cpu,
    Gpu,
    Engines,
    Macro,
    ShaderDecompiler,
    MetalRenderer,
    SDL3Window,
    Horizon,
    Kernel,
    Filesystem,
    Loader,
    Services,
    Applets,
    Cubeb,
    Hypervisor,
    Dynarmic,
    Input,
    Other,
};

} // namespace hydra

ENABLE_ENUM_FORMATTING(hydra::LogLevel, Debug, "D", Info, "I", Stub, "S",
                       Warning, "W", Error, "E", Fatal, "F")

ENABLE_ENUM_FORMATTING(hydra::LogClass, Common, "Common", Mmu, "MMU", Cpu,
                       "CPU", Gpu, "GPU", Engines, "Engines", Macro, "Macro",
                       ShaderDecompiler, "Shader Decompiler", MetalRenderer,
                       "Renderer::Metal", SDL3Window, "Window::SDL3", Horizon,
                       "Horizon", Kernel, "Kernel", Filesystem, "Filesystem",
                       Loader, "Loader", Services, "Services", Applets,
                       "Applets", Cubeb, "Cubeb", Hypervisor, "Hypervisor",
                       Dynarmic, "Dynarmic", Input, "input", Other, "")

namespace hydra {

struct LogMessage {
    LogLevel level;
    LogClass c;
    std::string file;
    u32 line;
    std::string function;
    std::string str;
};

typedef std::function<void(const LogMessage&)> log_callback_fn_t;

class Logger {
  public:
    static Logger& GetInstance() {
        static Logger instance;
        return instance;
    }

    ~Logger();

    void InstallCallback(log_callback_fn_t callback_) {
        std::lock_guard lock(mutex);
        callback = callback_;
    }

    void UninstallCallback() {
        std::lock_guard lock(mutex);
        callback = std::nullopt;
    }

    void SetOutput(const LogOutput output_) {
        std::lock_guard lock(mutex);
        output = output_;
    }

    template <typename... T>
    void Log(LogLevel level, LogClass c, const std::string_view file, u32 line,
             const std::string_view function, fmt::format_string<T...> f,
             T&&... args) {
        {
            std::lock_guard lock(mutex);

            switch (output) {
            case LogOutput::None:
                break;
            case LogOutput::StdOut:
                // Level
                fmt::terminal_color color;
                switch (level) {
                case LogLevel::Debug:
                    color = fmt::terminal_color::cyan;
                    break;
                case LogLevel::Info:
                    color = fmt::terminal_color::white;
                    break;
                case LogLevel::Stub:
                    color = fmt::terminal_color::magenta;
                    break;
                case LogLevel::Warning:
                    color = fmt::terminal_color::bright_yellow;
                    break;
                case LogLevel::Error:
                    color = fmt::terminal_color::bright_red;
                    break;
                case LogLevel::Fatal:
                    color = fmt::terminal_color::red;
                    break;
                }

                // Debug info
                fmt::print(fg(color), "{:016x} |{}| {:>17} {:>24} in {:>48}: ",
                           std::bit_cast<u64>(std::this_thread::get_id()),
                           level, c, function,
                           fmt::format("{}:{}", file, line));

                // Message
                fmt::print(f, std::forward<T>(args)...);
                fmt::print("\n");
                break;
            case LogOutput::File:
                EnsureOutputStream();

                fmt::print(*ofs, "TODO(TIME) |{}| {:>17}: ", level, c);
                fmt::print(*ofs, f, std::forward<T>(args)...);
                fmt::print(*ofs, "\n");
                break;
            default:
                throw std::runtime_error("Invalid logging output");
                break;
            }
        }

        if (callback)
            (*callback)(LogMessage{level, c, std::string(file), line,
                                   std::string(function),
                                   fmt::format(f, std::forward<T>(args)...)});
    }

  private:
    std::mutex mutex;
    std::ofstream* ofs{nullptr};

    std::optional<log_callback_fn_t> callback{};
    LogOutput output{LogOutput::StdOut};

    void EnsureOutputStream();
};

} // namespace hydra
