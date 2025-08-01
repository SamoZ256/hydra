#pragma once

#include "core/horizon/services/service.hpp"

#define SERVICE_COMMAND_CASE(service, id, func)                                \
    case id:                                                                   \
        LOG_DEBUG(Services, #func);                                            \
        return invoke_command(context, *this, &service::func);

#define DEFINE_SERVICE_COMMAND_TABLE(service, ...)                             \
    result_t service::RequestImpl(RequestContext& context, u32 id) {           \
        switch (id) {                                                          \
            FOR_EACH_1_2(SERVICE_COMMAND_CASE, service, __VA_ARGS__)           \
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

    Reader* reader;

    InBuffer() : reader{nullptr} {}
    InBuffer(Reader& reader_) : reader{&reader_} {}
};

template <BufferAttr attr_>
class OutBuffer {
  public:
    static constexpr BufferAttr attr = attr_;

    Writer* writer;

    OutBuffer() : writer{nullptr} {}
    OutBuffer(Writer& writer_) : writer{&writer_} {}
};

enum class HandleAttr {
    Copy,
    Move,
};

template <HandleAttr attr_>
class InHandle {
  public:
    static constexpr HandleAttr attr = attr_;

    InHandle() : handle_id{INVALID_HANDLE_ID} {}
    InHandle(handle_id_t handle_id_) : handle_id{handle_id_} {}

    operator handle_id_t() const { return handle_id; }

  private:
    handle_id_t handle_id;
};

template <HandleAttr attr_>
class OutHandle {
  public:
    static constexpr HandleAttr attr = attr_;

    OutHandle() : handle_id{nullptr} {}
    OutHandle(handle_id_t& handle_id_) : handle_id{&handle_id_} {}

    operator handle_id_t&() { return *handle_id; }

    void operator=(handle_id_t other) { *handle_id = other; }

  private:
    handle_id_t* handle_id;
};

enum class ArgumentType {
    Context,
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
        } else if constexpr (traits::type == ArgumentType::Process) {
            arg = context.process;

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::InData) {
            arg = context.readers.reader.Read<Arg>();

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::OutData) {
            arg = context.writers.writer.WritePtr<typename traits::BaseType>();

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::InBuffer) {
            Reader* reader;
            if constexpr (Arg::attr == BufferAttr::AutoSelect) {
                // TODO: correct?
                reader = &context.readers.send_buffers_readers[in_buffer_index];
                if (!reader->IsValid())
                    reader =
                        &context.readers.send_statics_readers[in_buffer_index];
            } else if constexpr (Arg::attr == BufferAttr::MapAlias) {
                reader = &context.readers.send_buffers_readers[in_buffer_index];
            } else if constexpr (Arg::attr == BufferAttr::HipcPointer) {
                reader = &context.readers.send_statics_readers[in_buffer_index];
            } else {
                LOG_FATAL(Services, "Invalid in buffer args");
            }

            arg = Arg(*reader);

            // Next
            read_arg<Class, CommandArguments, in_buffer_index + 1,
                     out_buffer_index, arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::OutBuffer) {
            Writer* writer;
            if constexpr (Arg::attr == BufferAttr::AutoSelect) {
                // TODO: correct?
                writer =
                    &context.writers.recv_buffers_writers[out_buffer_index];
                if (!writer->IsValid())
                    writer =
                        &context.writers.recv_list_writers[out_buffer_index];
            } else if constexpr (Arg::attr == BufferAttr::MapAlias) {
                writer =
                    &context.writers.recv_buffers_writers[out_buffer_index];
            } else if constexpr (Arg::attr == BufferAttr::HipcPointer) {
                writer = &context.writers.recv_list_writers[out_buffer_index];
            } else {
                LOG_FATAL(Services, "Invalid out buffer args");
            }

            arg = Arg(*writer);

            // Next
            read_arg<Class, CommandArguments, in_buffer_index,
                     out_buffer_index + 1, arg_index + 1>(context, instance,
                                                          args);
            return;
        } else if constexpr (traits::type == ArgumentType::InHandle) {
            handle_id_t handle_id;
            if constexpr (Arg::attr == HandleAttr::Copy) {
                handle_id =
                    context.readers.copy_handles_reader.Read<handle_id_t>();
            } else if constexpr (Arg::attr == HandleAttr::Move) {
                handle_id =
                    context.readers.move_handles_reader.Read<handle_id_t>();
            } else {
                LOG_FATAL(Services, "Invalid in handle args");
            }

            arg = Arg(handle_id);

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::OutHandle) {
            handle_id_t* handle_id;
            if constexpr (Arg::attr == HandleAttr::Copy) {
                handle_id =
                    context.writers.copy_handles_writer.WritePtr<handle_id_t>();
            } else if constexpr (Arg::attr == HandleAttr::Move) {
                handle_id =
                    context.writers.move_handles_writer.WritePtr<handle_id_t>();
            } else {
                LOG_FATAL(Services, "Invalid out handle args");
            }

            arg = Arg(*handle_id);

            // Next
            read_arg<Class, CommandArguments, in_buffer_index, out_buffer_index,
                     arg_index + 1>(context, instance, args);
            return;
        } else if constexpr (traits::type == ArgumentType::InService) {
            ASSERT_DEBUG(context.readers.objects_reader, Services,
                         "Objects reader is null");
            auto service_handle_id =
                context.readers.objects_reader->Read<handle_id_t>();
            arg = dynamic_cast<Arg>(
                instance.GetService(context, service_handle_id));
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

template <typename Class, typename MethodClass, typename... Args, size_t... Is>
result_t invoke_command_with_args(RequestContext& context, Class& instance,
                                  result_t (MethodClass::*func)(Args...),
                                  std::index_sequence<Is...>) {
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
