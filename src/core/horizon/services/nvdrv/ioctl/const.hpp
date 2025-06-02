#pragma once

#include "core/horizon/services/nvdrv/const.hpp"

#define IOCTL_CASE(fd, nr, func)                                               \
    case nr: {                                                                 \
        LOG_DEBUG(Services, #func);                                            \
        return invoke_ioctl(context, *this, &fd::func);                        \
    }

#define DEFINE_IOCTL_TABLE_ENTRY(fd, type, ...)                                \
    case type:                                                                 \
        switch (nr) {                                                          \
            FOR_EACH_1_2(IOCTL_CASE, fd, __VA_ARGS__)                          \
        default:                                                               \
            LOG_WARN(Services, "Unknown ioctl nr 0x{:02x} for type 0x{:02x}",  \
                     nr, type);                                                \
            return NvResult::NotImplemented;                                   \
        }

#define DEFINE_IOCTL_TABLE(fd, ...)                                            \
    NvResult fd::Ioctl(IoctlContext& context, u32 type, u32 nr) {              \
        switch (type) {                                                        \
            __VA_ARGS__                                                        \
        default:                                                               \
            LOG_WARN(Services, "Unknown ioctl nr 0x{:02x} for type 0x{:02x}",  \
                     nr, type);                                                \
            return NvResult::NotImplemented;                                   \
        }                                                                      \
    }

namespace hydra::horizon::services::nvdrv::ioctl {

struct IoctlContext {
    Reader* reader;
    Reader* buffer_reader;
    Writer* writer;
    Writer* buffer_writer;
};

template <typename In, typename Out> struct InOut {
    static_assert(sizeof(In) == sizeof(Out));
    In in;
    Out* out;

    operator In() const { return in; }
    void operator=(const Out& other) { *out = other; }
};

template <typename T> struct InOutSingle {
    T* data;

    operator T() const { return *data; }
    void operator=(const T& other) { *data = other; }
};

enum class ArgumentType {
    In,
    Out,
    InOut,
    InOutSingle,
    InArray,
};

template <typename T> struct arg_traits;

template <typename T> struct arg_traits {
    static constexpr ArgumentType type = ArgumentType::In;
};

template <typename T> struct arg_traits<T*> {
    static constexpr ArgumentType type = ArgumentType::Out;
    using BaseType = T;
};

template <typename In_, typename Out_> struct arg_traits<InOut<In_, Out_>> {
    static constexpr ArgumentType type = ArgumentType::InOut;
    using In = In_;
    using Out = Out_;
};

template <typename T> struct arg_traits<InOutSingle<T>> {
    static constexpr ArgumentType type = ArgumentType::InOutSingle;
    using BaseType = T;
};

template <typename T> struct arg_traits<const T*> {
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

        if constexpr (traits::type == ArgumentType::In) {
            ASSERT_DEBUG(context.reader, Services, "No reader");
            arg = context.reader->Read<Arg>();
            if (context.writer)
                context.writer->Skip(sizeof(Arg));

            // Next
            read_arg<CommandArguments, arg_index + 1>(context, args);
            return;
        } else if constexpr (traits::type == ArgumentType::Out) {
            ASSERT_DEBUG(context.writer, Services, "No writer");
            arg = context.writer->WritePtr<typename traits::BaseType>();
            if (context.reader)
                context.reader->Skip(sizeof(typename traits::BaseType));

            // Next
            read_arg<CommandArguments, arg_index + 1>(context, args);
            return;
        } else if constexpr (traits::type == ArgumentType::InOut) {
            ASSERT_DEBUG(context.reader, Services, "No reader");
            ASSERT_DEBUG(context.writer, Services, "No writer");
            arg.in = context.reader->Read<typename traits::In>();
            arg.out = context.writer->WritePtr<typename traits::Out>();

            // Next
            read_arg<CommandArguments, arg_index + 1>(context, args);
            return;
        } else if constexpr (traits::type == ArgumentType::InOutSingle) {
            ASSERT_DEBUG(context.reader, Services, "No reader");
            ASSERT_DEBUG(context.writer, Services, "No writer");
            arg.data = context.writer->WritePtr<typename traits::BaseType>();
            *arg.data = context.reader->Read<typename traits::BaseType>();

            // Next
            read_arg<CommandArguments, arg_index + 1>(context, args);
            return;
        } else /*if constexpr (traits::type == ArgumentType::InArray)*/ {
            // TODO: correct?
            Reader* reader;
            if (context.buffer_reader)
                reader = context.buffer_reader;
            else
                reader = context.reader;

            ASSERT_DEBUG(reader, Services, "No reader");
            arg = reader->ReadPtr<typename traits::BaseType>();

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
