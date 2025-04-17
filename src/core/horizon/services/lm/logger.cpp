#include "core/horizon/services/lm/logger.hpp"

namespace Hydra::Horizon::Services::Lm {

namespace {

enum class LogPacketFlags : u8 {
    None = 0,

    Head = BIT(0),
    Tail = BIT(1),
    LittleEndian = BIT(2),
};

ENABLE_ENUM_BITMASK_OPERATORS(Hydra::Horizon::Services::Lm::LogPacketFlags)

enum class LogSeverity : u8 {
    Trace,
    Info,
    Warn,
    Error,
    Fatal,
};

struct LogPacketHeader {
    u64 process_id;
    u64 thread_id;
    LogPacketFlags flags;
    u8 reserved;
    LogSeverity severity;
    u8 verbosity;
    u32 payload_size;
};

enum class LogDataChunkKey {
    Begin,
    End,
    TextLog,
    Line,
    Filename,
    Function,
    Module,
    Thread,
    DropCount,
    Time,
    ProcessName,
};

// From Ryujinx
bool try_read_uleb128(Reader& reader, u32& result) {
    result = 0;
    int count = 0;
    u8 encoded;

    do {
        // TODO: check if enough space
        encoded = reader.Read<u8>();

        result += (encoded & 0x7F) << (7 * count);

        count++;
    } while ((encoded & 0x80) != 0);

    return true;
}

} // namespace

} // namespace Hydra::Horizon::Services::Lm

ENABLE_ENUM_FORMATTING(Hydra::Horizon::Services::Lm::LogSeverity, Trace,
                       "trace", Info, "info", Warn, "warn", Error, "error",
                       Fatal, "fatal")

ENABLE_ENUM_FLAGS_FORMATTING(Hydra::Horizon::Services::Lm::LogPacketFlags, Head,
                             "head", Tail, "tail", LittleEndian,
                             "little endian")

namespace Hydra::Horizon::Services::Lm {

DEFINE_SERVICE_COMMAND_TABLE(ILogger, 0, Log)

void ILogger::Log(REQUEST_COMMAND_PARAMS) {
    auto& reader = readers.send_buffers_readers[0];
    const auto header = reader.Read<LogPacketHeader>();

    // From Ryujinx
    bool is_head_packet = any(header.flags & LogPacketFlags::Head);
    bool is_tail_packet = any(header.flags & LogPacketFlags::Tail);

    while (reader.GetReadSize() - sizeof(LogPacketHeader) <
           header.payload_size) { // TODO: correct?
        u32 key;
        u32 size;
        if (!try_read_uleb128(reader, key) || !try_read_uleb128(reader, size)) {
            result = MAKE_KERNEL_RESULT(Error::InvalidCombination); // TODO
            return;
        }

        const auto data = reader.Read<u8>(size);

#define GET_DATA(type) *reinterpret_cast<type*>(data)
#define GET_STRING() std::string(reinterpret_cast<const char*>(data), size)

        switch (static_cast<LogDataChunkKey>(key)) {
        case LogDataChunkKey::Begin:
            reader.Skip(size);
            continue;
        case LogDataChunkKey::End:
            break;
        case LogDataChunkKey::TextLog: {
            const auto text = GET_STRING();

            if (is_head_packet && is_tail_packet) {
                packet.message = text;
            } else {
                packet.message += text;
                // TODO: limit the size?
            }

            break;
        }
        case LogDataChunkKey::Line:
            packet.line = GET_DATA(u32);
            break;
        case LogDataChunkKey::Filename:
            packet.filename = GET_STRING();
            break;
        case LogDataChunkKey::Function:
            packet.function = GET_STRING();
            break;
        case LogDataChunkKey::Module:
            packet.module = GET_STRING();
            break;
        case LogDataChunkKey::Thread:
            packet.thread = GET_STRING();
            break;
        case LogDataChunkKey::DropCount:
            packet.drop_count = GET_DATA(u64);
            break;
        case LogDataChunkKey::Time:
            packet.time = GET_DATA(u64);
            break;
        case LogDataChunkKey::ProcessName:
            LOG_NOT_IMPLEMENTED(HorizonServices, "ProcessName");
            break;
        }

#undef GET_STRING
#undef GET_DATA
    }

    if (is_tail_packet) {
        std::string msg;
        if (!packet.program_name.empty() || !packet.filename.empty() ||
            packet.line != 0 || !packet.function.empty() ||
            !packet.module.empty()) {
            msg += fmt::format("- {} -> {}:{}, function {} in {}\n",
                               packet.program_name, packet.filename,
                               packet.line, packet.function, packet.module);
        }
        if (!packet.thread.empty()) {
            msg += fmt::format("- Thread: {}\n", packet.thread);
        }
        if (packet.drop_count != 0) {
            msg += fmt::format("- Drop count: {}\n", packet.drop_count);
        }
        if (packet.time != 0) {
            msg += fmt::format("- Time: {}\n", packet.time);
        }
        if (!packet.message.empty()) {
            msg += fmt::format("- Message: \"{}\"\n", packet.message);
        }
        LOG_INFO(HorizonServices, "Log:\n{}", msg);
        packet = {};
    }
}

} // namespace Hydra::Horizon::Services::Lm
