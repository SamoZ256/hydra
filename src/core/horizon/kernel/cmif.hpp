#pragma once

#include "core/horizon/kernel/hipc.hpp"

namespace hydra::horizon::kernel::cmif {

#define CMIF_IN_HEADER_MAGIC 0x49434653  // "SFCI"
#define CMIF_OUT_HEADER_MAGIC 0x4F434653 // "SFCO"

enum class DomainCommandType : u8 {
    Invalid,
    SendMessage,
    Close,
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
    ConvertCurrentObjectToDomain,
    CopyFromCurrentDomain,
    CloneCurrentObject,
    QueryPointerBufferSize,
    CloneCurrentObjectEx,
};

struct DomainInHeader {
    DomainCommandType type;
    u8 num_in_objects;
    u16 data_size;
    u32 object_id;
    u32 padding;
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
    u32 padding[3];
};

// From https://github.com/switchbrew/libnx
struct OutHeader {
    u32 magic;
    u32 version;
    result_t result;
    u32 token;
};

inline result_t* write_out_header(Writer& writer) {
    auto hdr = writer.Write(OutHeader{
        .magic = CMIF_OUT_HEADER_MAGIC,
        .version = 0,
        .result = RESULT_SUCCESS,
        .token = 0,
    });

    return &hdr->result;
}

inline void write_domain_out_header(Writer& writer) {
    writer.Write(DomainOutHeader{
        .num_out_objects = 0,
    });
}

} // namespace hydra::horizon::kernel::cmif

ENABLE_ENUM_FORMATTING(hydra::horizon::kernel::cmif::DomainCommandType, Invalid,
                       "invalid", SendMessage, "send message", Close, "close")

ENABLE_ENUM_FORMATTING(hydra::horizon::kernel::cmif::CommandType, Invalid,
                       "invalid", LegacyRequest, "legacy request", Close,
                       "close", LegacyControl, "legacy control", Request,
                       "request", Control, "control", RequestWithContext,
                       "request with context", ControlWithContext,
                       "control with context")

ENABLE_ENUM_FORMATTING(hydra::horizon::kernel::cmif::ControlCommandType,
                       ConvertCurrentObjectToDomain,
                       "convert current object to domain",
                       CopyFromCurrentDomain, "copy from current domain",
                       CloneCurrentObject, "clone current object",
                       QueryPointerBufferSize, "query pointer buffer size",
                       CloneCurrentObjectEx, "clone current object ex")
