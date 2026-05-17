#pragma once

#include "core/horizon/services/nvdrv/const.hpp"

#define IOCTL_CASE(fd, ioctl_suffix, nr, func)                                 \
    case nr: {                                                                 \
        LOG_DEBUG(Services, #func #ioctl_suffix);                              \
        return invoke_ioctl(context, *this, &fd::func##ioctl_suffix);          \
    }

#define DEFINE_IOCTL_TABLE_ENTRY_IMPL(fd, ioctl_suffix, type, ...)             \
    case type:                                                                 \
        switch (nr) {                                                          \
            FOR_EACH_2_2(IOCTL_CASE, fd, ioctl_suffix, __VA_ARGS__)            \
        default:                                                               \
            LOG_WARN(Services, "Unknown ioctl nr 0x{:02x} for type 0x{:02x}",  \
                     nr, type);                                                \
            return NvResult::NotImplemented;                                   \
        }

#define DEFINE_IOCTL_TABLE_ENTRY(fd, type, ...)                                \
    DEFINE_IOCTL_TABLE_ENTRY_IMPL(fd, , type, __VA_ARGS__)
#define DEFINE_IOCTL2_TABLE_ENTRY(fd, type, ...)                               \
    DEFINE_IOCTL_TABLE_ENTRY_IMPL(fd, 2, type, __VA_ARGS__)
#define DEFINE_IOCTL3_TABLE_ENTRY(fd, type, ...)                               \
    DEFINE_IOCTL_TABLE_ENTRY_IMPL(fd, 3, type, __VA_ARGS__)

#define DEFINE_IOCTL_TABLE_IMPL(fd, ioctl_suffix, ...)                         \
    NvResult fd::Ioctl##ioctl_suffix([[maybe_unused]] IoctlContext& context,   \
                                     u32 type, u32 nr) {                       \
        switch (type) {                                                        \
            __VA_ARGS__                                                        \
        default:                                                               \
            LOG_WARN(Services, "Unknown ioctl nr 0x{:02x} for type 0x{:02x}",  \
                     nr, type);                                                \
            return NvResult::NotImplemented;                                   \
        }                                                                      \
    }

#define DEFINE_IOCTL_TABLE(fd, ...) DEFINE_IOCTL_TABLE_IMPL(fd, , __VA_ARGS__)
#define DEFINE_IOCTL2_TABLE(fd, ...) DEFINE_IOCTL_TABLE_IMPL(fd, 2, __VA_ARGS__)
#define DEFINE_IOCTL3_TABLE(fd, ...) DEFINE_IOCTL_TABLE_IMPL(fd, 3, __VA_ARGS__)

namespace hydra {
class System;
}

namespace hydra::horizon::kernel {
class Process;
}

namespace hydra::horizon::services::nvdrv::ioctl {

struct IoctlContext {
    System& system;
    kernel::Process* process;
    io::MemoryStream* in_stream;
    io::MemoryStream* in_buffer_stream;
    io::MemoryStream* out_stream;
    io::MemoryStream* out_buffer_stream;
};

template <typename In, typename Out>
struct InOut {
    static_assert(sizeof(In) == sizeof(Out));
    In in;
    Out* out;

    operator In() const { return in; }
    void operator=(const Out& other) { *out = other; }

    In Get() const { return in; }
};

template <typename T>
struct InOutSingle {
    T* data;

    operator T() const { return *data; }
    void operator=(const T& other) { *data = other; }

    T Get() const { return *data; }
};

enum class ArgumentType {
    Context,
    System,
    Process,
    In,
    Out,
    InOut,
    InOutSingle,
    InArray,
};

template <typename T>
struct arg_traits;

template <>
struct arg_traits<IoctlContext*> {
    static constexpr ArgumentType type = ArgumentType::Context;
};

template <>
struct arg_traits<System*> {
    static constexpr ArgumentType type = ArgumentType::System;
};

template <>
struct arg_traits<kernel::Process*> {
    static constexpr ArgumentType type = ArgumentType::Process;
};

template <typename T>
struct arg_traits {
    static constexpr ArgumentType type = ArgumentType::In;
};

template <typename T>
struct arg_traits<T*> {
    static constexpr ArgumentType type = ArgumentType::Out;
    using BaseType = T;
};

template <typename In_, typename Out_>
struct arg_traits<InOut<In_, Out_>> {
    static constexpr ArgumentType type = ArgumentType::InOut;
    using In = In_;
    using Out = Out_;
};

template <typename T>
struct arg_traits<InOutSingle<T>> {
    static constexpr ArgumentType type = ArgumentType::InOutSingle;
    using BaseType = T;
};

template <typename T>
struct arg_traits<const T*> {
    static constexpr ArgumentType type = ArgumentType::InArray;
    using BaseType = T;
};

template <typename CommandArguments, u32 arg_index = 0>
void read_arg(IoctlContext& context, CommandArguments& args) {
    if constexpr (arg_index >= std::tuple_size_v<CommandArguments>) {
        return;
    } else {
        using Arg = std::tuple_element_t<arg_index, CommandArguments>;
        using traits = arg_traits<Arg>;

        auto& arg = std::get<arg_index>(args);

        if constexpr (traits::type == ArgumentType::Context) {
            arg = &context;

            // Next
            read_arg<CommandArguments, arg_index + 1>(context, args);
            return;
        } else if constexpr (traits::type == ArgumentType::System) {
            arg = &context.system;

            // Next
            read_arg<CommandArguments, arg_index + 1>(context, args);
            return;
        } else if constexpr (traits::type == ArgumentType::Process) {
            arg = context.process;

            // Next
            read_arg<CommandArguments, arg_index + 1>(context, args);
            return;
        } else if constexpr (traits::type == ArgumentType::In) {
            ASSERT_DEBUG(context.in_stream, Services, "No input stream");
            arg = context.in_stream->Read<Arg>();
            if (context.out_stream)
                context.out_stream->SeekBy(sizeof(Arg));

            // Next
            read_arg<CommandArguments, arg_index + 1>(context, args);
            return;
        } else if constexpr (traits::type == ArgumentType::Out) {
            ASSERT_DEBUG(context.out_stream, Services, "No output stream");
            arg = context.out_stream
                      ->WriteReturningPtr<typename traits::BaseType>();
            if (context.in_stream)
                context.in_stream->SeekBy(sizeof(typename traits::BaseType));

            // Next
            read_arg<CommandArguments, arg_index + 1>(context, args);
            return;
        } else if constexpr (traits::type == ArgumentType::InOut) {
            ASSERT_DEBUG(context.in_stream, Services, "No input stream");
            ASSERT_DEBUG(context.out_stream, Services, "No output stream");
            arg.in = context.in_stream->Read<typename traits::In>();
            arg.out =
                context.out_stream->WriteReturningPtr<typename traits::Out>();

            // Next
            read_arg<CommandArguments, arg_index + 1>(context, args);
            return;
        } else if constexpr (traits::type == ArgumentType::InOutSingle) {
            ASSERT_DEBUG(context.in_stream, Services, "No input stream");
            ASSERT_DEBUG(context.out_stream, Services, "No output stream");
            arg.data = context.out_stream
                           ->WriteReturningPtr<typename traits::BaseType>();
            *arg.data = context.in_stream->Read<typename traits::BaseType>();

            // Next
            read_arg<CommandArguments, arg_index + 1>(context, args);
            return;
        } else /*if constexpr (traits::type == ArgumentType::InArray)*/ {
            ASSERT_DEBUG(context.in_stream, Services, "No input stream");
            arg = context.in_stream->ReadPtr<typename traits::BaseType>();

            // Next
            static_assert(arg_index == std::tuple_size_v<CommandArguments> - 1,
                          "InArray must be the last argument");
            return;
        }
    }
}

template <typename Class, typename... Args, size_t... Is>
NvResult invoke_command_with_args(IoctlContext& context, Class& instance,
                                  NvResult (Class::*func)(Args...),
                                  std::index_sequence<Is...>) {
    using traits = function_traits<decltype(func)>;

    auto args = std::tuple<typename traits::template arg<Is>::type...>();
    read_arg(context, args);

    auto callable = [&]<typename... CallArgs>(CallArgs&... args) {
        return (instance.*func)(args...);
    };

    return std::apply(callable, args);
}

template <typename Class, typename... Args>
NvResult invoke_ioctl(IoctlContext& context, Class& instance,
                      NvResult (Class::*func)(Args...)) {
    using traits = function_traits<decltype(func)>;

    constexpr auto indices = std::make_index_sequence<traits::arg_count>{};

    return invoke_command_with_args(context, instance, func, indices);
}

} // namespace hydra::horizon::services::nvdrv::ioctl

template <typename In, typename Out>
struct fmt::formatter<hydra::horizon::services::nvdrv::ioctl::InOut<In, Out>>
    : formatter<string_view> {
    fmt::formatter<In> value_formatter;

    constexpr auto parse(fmt::format_parse_context& ctx) {
        return value_formatter.parse(ctx);
    }

    template <typename FormatContext>
    auto
    format(const hydra::horizon::services::nvdrv::ioctl::InOut<In, Out>& value,
           FormatContext& ctx) const {
        return value_formatter.format(value.Get(), ctx);
    }
};

template <typename T>
struct fmt::formatter<hydra::horizon::services::nvdrv::ioctl::InOutSingle<T>>
    : formatter<string_view> {
    fmt::formatter<T> value_formatter;

    constexpr auto parse(fmt::format_parse_context& ctx) {
        return value_formatter.parse(ctx);
    }

    template <typename FormatContext>
    auto
    format(const hydra::horizon::services::nvdrv::ioctl::InOutSingle<T>& value,
           FormatContext& ctx) const {
        return value_formatter.format(value.Get(), ctx);
    }
};
