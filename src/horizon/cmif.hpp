#pragma once

#include "common/common.hpp"
#include "horizon/const.hpp"
#include "horizon/hipc.hpp"

namespace Hydra::Horizon::Cmif {

#define CMIF_IN_HEADER_MAGIC 0x49434653  // "SFCI"
#define CMIF_OUT_HEADER_MAGIC 0x4F434653 // "SFCO"

enum class CommandType {
    Invalid = 0,
    LegacyRequest = 1,
    Close = 2,
    LegacyControl = 3,
    Request = 4,
    Control = 5,
    RequestWithContext = 6,
    ControlWithContext = 7,
};

struct DomainInHeader {
    u8 type;
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
    Result result;
    u32 token;
};

inline Result* write_out_header(Writer& writer) {
    auto hdr = writer.Write(OutHeader{
        .magic = CMIF_OUT_HEADER_MAGIC,
        .version = 0,
        .result = MAKE_KERNEL_RESULT(NotImplemented),
        .token = 0,
    });

    return &hdr->result;
}

inline void write_domain_out_header(Writer& writer) {
    writer.Write(DomainOutHeader{
        .num_out_objects = 0,
    });
}

} // namespace Hydra::Horizon::Cmif
