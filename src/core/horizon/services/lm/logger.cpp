#include "core/horizon/services/lm/logger.hpp"

namespace hydra::horizon::services::lm {

namespace {

enum class PacketFlags : u8 {
    None = 0,

    Head = ZTD_BIT(0),
    Tail = ZTD_BIT(1),
    LittleEndian = ZTD_BIT(2),
};

ZTD_ENABLE_ENUM_BITWISE_OPERATORS(hydra::horizon::services::lm::PacketFlags)

enum class Severity : u8 {
    Trace,
    Info,
    Warn,
    Error,
    Fatal,
};

struct LogPacketHeader {
    u64 process_id;
    u64 thread_id;
    PacketFlags flags;
    u8 reserved;
    Severity severity;
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
bool TryReadUleb128(io::MemoryStream& stream, u32& result) {
    result = 0;
    int count = 0;
    u8 encoded;

    do {
        // TODO: check if enough space
        encoded = stream.Read<u8>();

        result += static_cast<u32>(encoded & 0x7F) << (7 * count);

        count++;
    } while ((encoded & 0x80) != 0);

    return true;
}

} // namespace

} // namespace hydra::horizon::services::lm

ENABLE_ENUM_FORMATTING(hydra::horizon::services::lm::Severity, Trace, "trace",
                       Info, "info", Warn, "warn", Error, "error", Fatal,
                       "fatal")

ENABLE_ENUM_FLAGS_FORMATTING(hydra::horizon::services::lm::PacketFlags, Head,
                             "head", Tail, "tail", LittleEndian,
                             "little endian")

namespace hydra::horizon::services::lm {

DEFINE_SERVICE_COMMAND_TABLE(ILogger, 0, Log)

result_t ILogger::Log(InBuffer<BufferAttr::AutoSelect> buffer) {
    ZTD_ASSIGN_OR_RETURN_VALUE(
        auto stream, buffer.stream,
        RESULT_SUCCESS); // TODO: return error on failure?
    const auto header = stream.Read<LogPacketHeader>();

    // From Ryujinx
    [[maybe_unused]] bool is_head_packet =
        any(header.flags & PacketFlags::Head);
    bool is_tail_packet = any(header.flags & PacketFlags::Tail);

    while (stream.GetSeek() - sizeof(LogPacketHeader) <
           header.payload_size) { // TODO: correct?
        u32 key;
        u32 size;
        if (!TryReadUleb128(stream, key) || !TryReadUleb128(stream, size))
            return MAKE_RESULT(
                Svc, kernel::Error::InvalidCombination); // TODO: module

        const auto data = stream.ReadSpan<u8>(size);

#define GET_DATA(type) *reinterpret_cast<const type*>(data.data())
#define GET_STRING()                                                           \
    std::string(reinterpret_cast<const char*>(data.data()), data.size())

        switch (static_cast<LogDataChunkKey>(key)) {
        case LogDataChunkKey::Begin:
            stream.SeekBy(size);
            continue;
        case LogDataChunkKey::End:
            break;
        case LogDataChunkKey::TextLog: {
            const auto text = GET_STRING();
            packet.message += text;
            // TODO: limit the size?
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
            packet.process = GET_STRING();
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
        if (!packet.process.empty()) {
            msg += fmt::format("- Process: {}\n", packet.process);
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
        LOG_INFO(Services, "Log:\n{}", msg);
        packet = {};
    }

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::lm
