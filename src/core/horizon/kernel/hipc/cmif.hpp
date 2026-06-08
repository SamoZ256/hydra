#pragma once

#include "core/horizon/kernel/hipc/const.hpp"

namespace hydra::horizon::kernel::hipc::cmif {

constexpr u32 IN_HEADER_MAGIC = make_magic4('S', 'F', 'C', 'I');
constexpr u32 OUT_HEADER_MAGIC = make_magic4('S', 'F', 'C', 'O');

enum class DomainCommandType : u8 {
    Invalid = 0,
    SendMessage = 1,
    Close = 2,
};

enum class CommandType {
    Invalid = 0,
    LegacyRequest = 1,
    Close = 2,
    LegacyControl = 3,
    Request = 4,
    Control = 5,
    RequestWithContext = 6,
    ControlWithContext = 7,

    TipcClose = 15,
    TipcCommandRegion = 16,
};

enum class ControlCommandType {
    ConvertCurrentObjectToDomain = 0,
    CopyFromCurrentDomain = 1,
    CloneCurrentObject = 2,
    QueryPointerBufferSize = 3,
    CloneCurrentObjectEx = 4,
};

struct DomainInHeader {
    DomainCommandType type;
    u8 num_in_objects;
    u16 data_size;
    u32 object_id;
    u32 _padding_x8;
    u32 token;
};

struct InHeader {
    u32 magic;
    u32 version;
    u32 command_id;
    u32 token;
};

// From https://github.com/switchbrew/libnx
struct DomainOutHeader {
    u32 num_out_objects;
    u32 _padding_x4[3];
};

// From https://github.com/switchbrew/libnx
struct OutHeader {
    u32 magic;
    u32 version;
    result_t result;
    u32 token;
};

inline result_t* write_out_header(io::MemoryStream& stream) {
    auto& hdr = stream.WriteReturningRef<OutHeader>({
        .magic = OUT_HEADER_MAGIC,
        .version = 0,
        .result = RESULT_SUCCESS,
        .token = 0,
    });

    return &hdr.result;
}

inline void write_domain_out_header(io::MemoryStream& stream) {
    stream.Write<DomainOutHeader>({
        .num_out_objects = 0,
    });
}

template <typename T>
inline T* align_data_start(T* data_start) {
    return align_ptr(data_start, 0x10); // align to 16 bytes
}

} // namespace hydra::horizon::kernel::hipc::cmif

ENABLE_ENUM_FORMATTING(hydra::horizon::kernel::hipc::cmif::DomainCommandType,
                       Invalid, "invalid", SendMessage, "send message", Close,
                       "close")

ENABLE_ENUM_FORMATTING(hydra::horizon::kernel::hipc::cmif::CommandType, Invalid,
                       "invalid", LegacyRequest, "legacy request", Close,
                       "close", LegacyControl, "legacy control", Request,
                       "request", Control, "control", RequestWithContext,
                       "request with context", ControlWithContext,
                       "control with context")

ENABLE_ENUM_FORMATTING(hydra::horizon::kernel::hipc::cmif::ControlCommandType,
                       ConvertCurrentObjectToDomain,
                       "convert current object to domain",
                       CopyFromCurrentDomain, "copy from current domain",
                       CloneCurrentObject, "clone current object",
                       QueryPointerBufferSize, "query pointer buffer size",
                       CloneCurrentObjectEx, "clone current object ex")
