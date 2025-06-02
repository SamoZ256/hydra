#pragma once

#include <mutex>

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ostream.h>

#include "common/macros.hpp"
#include "common/type_aliases.hpp"

#define LOG(level, c, ...)                                                     \
    g_logger.Log(Level::level, Class::c, trim_source_path(__FILE__), __LINE__, \
                 __func__, __VA_ARGS__)

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
#define LOG_STUBBED(c, fmt, ...)                                               \
    LOG(Stubbed, c, fmt " stubbed" PASS_VA_ARGS(__VA_ARGS__))
#define LOG_WARN(c, ...) LOG(Warning, c, __VA_ARGS__)
#define LOG_ERROR(c, ...) LOG(Error, c, __VA_ARGS__)
#define LOG_FATAL(c, ...)                                                      \
    {                                                                          \
        LOG(Fatal, c, __VA_ARGS__);                                            \
        throw;                                                                 \
    }

#define LOG_FUNC_STUBBED(c) LOG_STUBBED(c, "{}", __func__)
#define LOG_NOT_IMPLEMENTED(c, fmt, ...)                                       \
    LOG_WARN(c, fmt " not implemented" PASS_VA_ARGS(__VA_ARGS__))
#define LOG_FUNC_NOT_IMPLEMENTED(c) LOG_NOT_IMPLEMENTED(c, "{}", __func__)

#define ASSERT(condition, c, ...)                                              \
    if (!(condition)) {                                                        \
        LOG_ERROR(c, __VA_ARGS__);                                             \
    }
#define ASSERT_ALIGNMENT(value, alignment, c, name)                            \
    ASSERT(is_aligned<decltype(value)>(value, alignment), c,                   \
           name " must be {}-byte aligned", alignment)

#ifdef HYDRA_DEBUG
#define ASSERT_DEBUG(condition, c, ...) ASSERT(condition, c, __VA_ARGS__)
#else
// TODO: should we evaluate the condition anyway?
#define ASSERT_DEBUG(condition, c, ...)                                        \
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
enum class Output {
    Invalid = 0,

    StdOut,
    File,
};

enum class Level {
    Debug,
    Info,
    Stubbed,
    Warning,
    Error,
    Fatal,
};

enum class Class {
    Common,
    MMU,
    CPU,
    GPU,
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

ENABLE_ENUM_FORMATTING(hydra::Level, Debug, "D", Info, "I", Stubbed, "S",
                       Warning, "W", Error, "E", Fatal, "F")

ENABLE_ENUM_FORMATTING(hydra::Class, Common, "Common", MMU, "MMU", CPU, "CPU",
                       GPU, "GPU", Engines, "Engines", Macro, "Macro",
                       ShaderDecompiler, "Shader Decompiler", MetalRenderer,
                       "Renderer::Metal", SDL3Window, "Window::SDL3", Horizon,
                       "Horizon", Kernel, "Kernel", Filesystem, "Filesystem",
                       Loader, "Loader", Services, "Services", Applets,
                       "Applets", Cubeb, "Cubeb", Hypervisor, "Hypervisor",
                       Dynarmic, "Dynarmic", Input, "input", Other, "")

namespace hydra {

class Logger {
  public:
    ~Logger();

    template <typename... T>
    void Log(Level level, Class c, const std::string_view file, u32 line,
             const std::string_view function, fmt::format_string<T...> fmt,
             T&&... args) {
        mutex.lock();

        switch (GetOutput()) {
        case Output::StdOut:
            // Level
            fmt::terminal_color color;
            switch (level) {
            case Level::Debug:
                color = fmt::terminal_color::cyan;
                break;
            case Level::Info:
                color = fmt::terminal_color::white;
                break;
            case Level::Stubbed:
                color = fmt::terminal_color::magenta;
                break;
            case Level::Warning:
                color = fmt::terminal_color::bright_yellow;
                break;
            case Level::Error:
                color = fmt::terminal_color::bright_red;
                break;
            case Level::Fatal:
                color = fmt::terminal_color::red;
                break;
            }

            // Debug info
            fmt::print(fg(color), "{:016x} |{}| {:>17} {:>24} in {:>48}: ",
                       std::bit_cast<u64>(std::this_thread::get_id()), level, c,
                       function, fmt::format("{}:{}", file, line));

            // Message
            fmt::print(fmt, std::forward<T>(args)...);
            fmt::print("\n");
            break;
        case Output::File:
            EnsureOutputStream();

            fmt::print(*ofs, "TODO(TIME) |{}| {:>17}: ", level, c);
            fmt::print(*ofs, fmt, std::forward<T>(args)...);
            fmt::print(*ofs, "\n");
            break;
        default:
            throw std::runtime_error("Invalid logging output");
            break;
        }

        mutex.unlock();
    }

  private:
    std::mutex mutex;
    std::ofstream* ofs{nullptr};

    void EnsureOutputStream();

    // HACK
    static Output GetOutput();
};

extern Logger g_logger;

} // namespace hydra
