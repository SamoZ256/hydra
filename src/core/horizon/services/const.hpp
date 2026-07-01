#pragma once

#include "core/horizon/kernel/hipc/client_session.hpp"
#include "core/horizon/services/service.hpp"

#define SERVICE_COMMAND_CASE(service, id, func)                                \
    case id:                                                                   \
        LOG_DEBUG(Services, #func);                                            \
        return invoke_command(context, *this, &service::func);

#define DEFINE_SERVICE_COMMAND_TABLE(service, ...)                             \
    result_t service::RequestImpl([[maybe_unused]] RequestContext& context,    \
                                  u32 id) {                                    \
        switch (id) {                                                          \
            ZTD_FOR_EACH_1_2(SERVICE_COMMAND_CASE, service, __VA_ARGS__)       \
        default:                                                               \
            LOG_WARN(Services, "Unknown request {}", id);                      \
            return MAKE_RESULT(Svc, 0); /* TODO */                             \
        }                                                                      \
    }

#define STUB_REQUEST_COMMAND(name)                                             \
    result_t name() {                                                          \
        ONCE(LOG_FUNC_STUBBED(Services));                                      \
        return RESULT_SUCCESS;                                                 \
    }

namespace hydra {
class System;
}

// TODO: remove?
namespace hydra::horizon::kernel {
class Event;
class SharedMemory;
} // namespace hydra::horizon::kernel

namespace hydra::horizon::services {

using result_t = kernel::result_t;

enum class BufferAttr {
    AutoSelect,
    MapAlias,
    HipcPointer,
};

template <BufferAttr attr_>
class InBuffer {
  public:
    static constexpr BufferAttr attr = attr_;

    std::optional<io::MemoryStream> stream;

    InBuffer() : stream{std::nullopt} {}
    InBuffer(std::optional<io::MemoryStream> stream_)
        : stream{std::move(stream_)} {}

    bool IsValid() const { return stream.has_value(); }
};

template <BufferAttr attr_>
class OutBuffer {
  public:
    static constexpr BufferAttr attr = attr_;

    std::optional<io::MemoryStream> stream;

    OutBuffer() : stream{std::nullopt} {}
    OutBuffer(std::optional<io::MemoryStream> stream_)
        : stream{std::move(stream_)} {}

    bool IsValid() const { return stream.has_value(); }
};

enum class HandleAttr {
    Copy,
    Move,
};

template <HandleAttr attr_>
class InHandle {
  public:
    static constexpr HandleAttr attr = attr_;

    InHandle() : handle{INVALID_HANDLE} {}
    InHandle(Handle handle_) : handle{handle_} {}

    operator Handle() const { return handle; }

  private:
    Handle handle;
};

template <HandleAttr attr_>
class OutHandle {
  public:
    static constexpr HandleAttr attr = attr_;

    OutHandle() : handle{nullptr} {}
    OutHandle(Handle* handle_) : handle{handle_} {}

    operator Handle&() { return *handle; }

    OutHandle& operator=(Handle other) {
        *handle = other;
        return *this;
    }

  private:
    Handle* handle;
};

enum class ArgumentType {
    Context,
    System,
    Process,
    InData,
    OutData,
    InBuffer,
    OutBuffer,
    InHandle,
    OutHandle,
    InService,
    OutService,
};

template <typename T>
struct arg_traits;

template <>
struct arg_traits<RequestContext*> {
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
    static constexpr ArgumentType type = ArgumentType::InData;
};

template <typename T>
struct arg_traits<T*> {
    static constexpr ArgumentType type = ArgumentType::OutData;
    using BaseType = T;
};

template <BufferAttr attr>
struct arg_traits<InBuffer<attr>> {
    static constexpr ArgumentType type = ArgumentType::InBuffer;
};

template <BufferAttr attr>
struct arg_traits<OutBuffer<attr>> {
    static constexpr ArgumentType type = ArgumentType::OutBuffer;
};

template <HandleAttr attr>
struct arg_traits<InHandle<attr>> {
    static constexpr ArgumentType type = ArgumentType::InHandle;
};

template <HandleAttr attr>
struct arg_traits<OutHandle<attr>> {
    static constexpr ArgumentType type = ArgumentType::OutHandle;
};

template <>
struct arg_traits<IService*> {
    static constexpr ArgumentType type = ArgumentType::InService;
};

template <>
struct arg_traits<IService**> {
    static constexpr ArgumentType type = ArgumentType::OutService;
};

template <typename Class, typename CommandArguments, u32 in_buffer_index = 0,
          u32 out_buffer_index = 0, u32 arg_index = 0>
void read_arg(RequestContext& context, Class& instance,
              CommandArguments& args) {
    if constexpr (arg_index >= std::tuple_size_v<CommandArguments>) {
        return;
    } else {
        using Arg = std::tuple_element_t<arg_index, CommandArguments>;
        using traits = arg_traits<Arg>;

        auto& arg = std::get<arg_index>(args);

        if constexpr (traits::type == ArgumentType::Context) {
            arg = &context;

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::System) {
            arg = &context.system;

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::Process) {
            arg = context.process;

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::InData) {
            arg = context.streams.in_stream.Read<Arg>();

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::OutData) {
            arg = context.streams.out_stream
                      .WriteReturningPtr<typename traits::BaseType>();

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::InBuffer) {
            std::optional<io::MemoryStream> stream;
            if constexpr (Arg::attr == BufferAttr::AutoSelect) {
                if (in_buffer_index <
                    context.streams.send_buffers_streams.size())
                    stream =
                        context.streams.send_buffers_streams[in_buffer_index];
                if (!stream && in_buffer_index <
                                   context.streams.send_statics_streams.size())
                    stream =
                        context.streams.send_statics_streams[in_buffer_index];
            } else if constexpr (Arg::attr == BufferAttr::MapAlias) {
                stream = context.streams.send_buffers_streams[in_buffer_index];
            } else if constexpr (Arg::attr == BufferAttr::HipcPointer) {
                stream = context.streams.send_statics_streams[in_buffer_index];
            } else {
                LOG_FATAL(Services, "Invalid in buffer args");
            }

            arg = Arg(stream);

            // Next
            read_arg<Class, CommandArguments, in_buffer_index + 1,
                     out_buffer_index, arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::OutBuffer) {
            std::optional<io::MemoryStream> stream;
            if constexpr (Arg::attr == BufferAttr::AutoSelect) {
                if (out_buffer_index <
                    context.streams.recv_buffers_streams.size())
                    stream =
                        context.streams.recv_buffers_streams[out_buffer_index];
                if (!stream &&
                    out_buffer_index < context.streams.recv_list_streams.size())
                    stream =
                        context.streams.recv_list_streams[out_buffer_index];
            } else if constexpr (Arg::attr == BufferAttr::MapAlias) {
                stream = context.streams.recv_buffers_streams[out_buffer_index];
            } else if constexpr (Arg::attr == BufferAttr::HipcPointer) {
                stream = context.streams.recv_list_streams[out_buffer_index];
            } else {
                LOG_FATAL(Services, "Invalid out buffer args");
            }

            arg = Arg(stream);

            // Next
            read_arg<Class, CommandArguments, in_buffer_index,
                     out_buffer_index + 1, arg_index + 1>(context, instance,
                                                          args);
            return;
        } else if constexpr (traits::type == ArgumentType::InHandle) {
            Handle handle;
            if constexpr (Arg::attr == HandleAttr::Copy) {
                handle = context.streams.in_copy_handles_stream.Read<Handle>();
            } else if constexpr (Arg::attr == HandleAttr::Move) {
                handle = context.streams.in_move_handles_stream.Read<Handle>();
            } else {
                LOG_FATAL(Services, "Invalid in handle args");
            }

            arg = Arg(handle);

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::OutHandle) {
            Handle* handle;
            if constexpr (Arg::attr == HandleAttr::Copy) {
                handle = context.streams.out_copy_handles_stream
                             .WriteReturningPtr<Handle>();
            } else if constexpr (Arg::attr == HandleAttr::Move) {
                handle = context.streams.out_move_handles_stream
                             .WriteReturningPtr<Handle>();
            } else {
                LOG_FATAL(Services, "Invalid out handle args");
            }

            arg = Arg(handle);

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::InService) {
            ASSERT_DEBUG(context.streams.in_objects_stream, Services,
                         "Objects stream is null");
            auto service_handle =
                context.streams.in_objects_stream->Read<Handle>();
            arg = instance.GetService(context, service_handle);
            ASSERT_DEBUG(arg, Services, "Invalid service");

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::OutService) {
            // TODO: implement
            LOG_FATAL(Services, "OutService");

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else {
            LOG_FATAL(Services, "Invalid argument type");
        }
    }
}

template <typename Class, typename MethodClass, typename... Args, usize... Is>
result_t invoke_command_with_args(RequestContext& context, Class& instance,
                                  result_t (MethodClass::*func)(Args...),
                                  std::index_sequence<Is...> /*unused*/) {
    using traits = function_traits<decltype(func)>;

    auto args = std::tuple<typename traits::template arg<Is>::type...>();
    read_arg(context, instance, args);

    auto callable = [&]<typename... CallArgs>(CallArgs&... args) {
        return (instance.*func)(args...);
    };

    return std::apply(callable, args);
}

template <typename Class, typename MethodClass, typename... Args>
result_t invoke_command(RequestContext& context, Class& instance,
                        result_t (MethodClass::*func)(Args...)) {
    using traits = function_traits<decltype(func)>;

    constexpr auto indices = std::make_index_sequence<traits::arg_count>{};

    return invoke_command_with_args(context, instance, func, indices);
}

} // namespace hydra::horizon::services
