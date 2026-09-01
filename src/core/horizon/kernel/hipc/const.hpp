#pragma once

#include "core/horizon/kernel/const.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::horizon::kernel::hipc {

#define HIPC_AUTO_RECV_STATIC std::numeric_limits<u8>::max()
#define HIPC_RESPONSE_NO_PID std::numeric_limits<u32>::max()

// From https://github.com/switchbrew/libnx
struct Metadata {
    u32 type;
    u32 num_send_statics;
    u32 num_send_buffers;
    u32 num_recv_buffers;
    u32 num_exch_buffers;
    u32 num_data_words;
    u32 num_recv_statics; // also accepts HIPC_AUTO_RECV_STATIC
    u32 send_pid;
    u32 num_copy_handles;
    u32 num_move_handles;
};

struct BufferDescriptor {
    u32 size_low;
    u32 address_low;
    u32 mode : 2;
    u32 address_high : 22;
    u32 size_high : 4;
    u32 address_mid : 4;
};

struct RecvListEntry {
    u32 address_low;
    u32 address_high : 16;
    u32 size : 16;
};

// From https://github.com/switchbrew/libnx
struct Header {
    u32 type : 16;
    u32 num_send_statics : 4;
    u32 num_send_buffers : 4;
    u32 num_recv_buffers : 4;
    u32 num_exch_buffers : 4;
    u32 num_data_words : 10;
    u32 recv_static_mode : 4;
    u32 padding : 6;
    u32 recv_list_offset : 11; // Unused.
    u32 has_special_header : 1;
};

// From https://github.com/switchbrew/libnx
struct StaticDescriptor {
    u32 index : 6;
    u32 address_high : 6;
    u32 address_mid : 4;
    u32 size : 16;
    u32 address_low;
};

// From https://github.com/switchbrew/libnx
struct Response {
    u64 pid;
    u32 num_statics;
    u32 num_data_words;
    u32 num_copy_handles;
    u32 num_move_handles;
    StaticDescriptor* statics;
    u32* data_words;
    Handle* copy_handles;
    Handle* move_handles;
};

// From https://github.com/switchbrew/libnx
struct Request {
    StaticDescriptor* send_statics;
    BufferDescriptor* send_buffers;
    BufferDescriptor* recv_buffers;
    BufferDescriptor* exch_buffers;
    u32* data_words;
    RecvListEntry* recv_list;
    Handle* copy_handles;
    Handle* move_handles;
};

// From https://github.com/switchbrew/libnx
struct ParsedRequest {
    Metadata meta;
    Request data;
    u64 pid;
};

// From https://github.com/switchbrew/libnx
struct SpecialHeader {
    u32 send_pid : 1;
    u32 num_copy_handles : 4;
    u32 num_move_handles : 4;
    u32 padding : 23;
};

// From https://github.com/switchbrew/libnx
inline Request calc_request_layout(Metadata meta, void* base) {
    // Copy handles
    Handle* copy_handles = nullptr;
    if (meta.num_copy_handles != 0) {
        copy_handles = reinterpret_cast<Handle*>(base);
        base = copy_handles + meta.num_copy_handles;
    }

    // Move handles
    Handle* move_handles = nullptr;
    if (meta.num_move_handles != 0) {
        move_handles = reinterpret_cast<Handle*>(base);
        base = move_handles + meta.num_move_handles;
    }

    // Send statics
    StaticDescriptor* send_statics = nullptr;
    if (meta.num_send_statics != 0) {
        send_statics = reinterpret_cast<StaticDescriptor*>(base);
        base = send_statics + meta.num_send_statics;
    }

    // Send buffers
    BufferDescriptor* send_buffers = nullptr;
    if (meta.num_send_buffers != 0) {
        send_buffers = reinterpret_cast<BufferDescriptor*>(base);
        base = send_buffers + meta.num_send_buffers;
    }

    // Recv buffers
    BufferDescriptor* recv_buffers = nullptr;
    if (meta.num_recv_buffers != 0) {
        recv_buffers = reinterpret_cast<BufferDescriptor*>(base);
        base = recv_buffers + meta.num_recv_buffers;
    }

    // Exch buffers
    BufferDescriptor* exch_buffers = nullptr;
    if (meta.num_exch_buffers != 0) {
        exch_buffers = reinterpret_cast<BufferDescriptor*>(base);
        base = exch_buffers + meta.num_exch_buffers;
    }

    // Data words
    u32* data_words = nullptr;
    if (meta.num_data_words != 0) {
        data_words = reinterpret_cast<u32*>(base);
        base = data_words + meta.num_data_words;
    }

    // Recv list
    RecvListEntry* recv_list = nullptr;
    if (meta.num_recv_statics != 0) {
        recv_list = reinterpret_cast<RecvListEntry*>(base);
    }

    return Request{
        .send_statics = send_statics,
        .send_buffers = send_buffers,
        .recv_buffers = recv_buffers,
        .exch_buffers = exch_buffers,
        .data_words = data_words,
        .recv_list = recv_list,
        .copy_handles = copy_handles,
        .move_handles = move_handles,
    };
}

inline ParsedRequest parse_request(void* base) {
    // Parse message header
    Header hdr = {};
    memcpy(&hdr, base, sizeof(hdr));
    base = reinterpret_cast<u8*>(base) + sizeof(hdr);
    u32 num_recv_statics = 0;
    u64 pid = 0;

    // Parse recv static mode
    if (hdr.recv_static_mode != 0u) {
        if (hdr.recv_static_mode == 2u)
            num_recv_statics = HIPC_AUTO_RECV_STATIC;
        else if (hdr.recv_static_mode > 2u)
            num_recv_statics = hdr.recv_static_mode - 2u;
    }

    // Parse special header
    SpecialHeader sphdr = {};
    if (hdr.has_special_header) {
        memcpy(&sphdr, base, sizeof(sphdr));
        base = reinterpret_cast<u8*>(base) + sizeof(sphdr);

        // Read PID descriptor
        if (sphdr.send_pid) {
            pid = *reinterpret_cast<u64*>(base);
            base = reinterpret_cast<u8*>(base) + sizeof(u64);
        }
    }

    const Metadata meta = {
        .type = hdr.type,
        .num_send_statics = hdr.num_send_statics,
        .num_send_buffers = hdr.num_send_buffers,
        .num_recv_buffers = hdr.num_recv_buffers,
        .num_exch_buffers = hdr.num_exch_buffers,
        .num_data_words = hdr.num_data_words,
        .num_recv_statics = num_recv_statics,
        .send_pid = sphdr.send_pid,
        .num_copy_handles = sphdr.num_copy_handles,
        .num_move_handles = sphdr.num_move_handles,
    };

    return {
        .meta = meta,
        .data = calc_request_layout(meta, base),
        .pid = pid,
    };
}

inline Request make_request(void* base, Metadata meta) {
    // Write message header
    bool has_special_header = (meta.send_pid != 0u) ||
                              (meta.num_copy_handles != 0u) ||
                              (meta.num_move_handles != 0u);
    auto* hdr = reinterpret_cast<Header*>(base);
    base = hdr + 1;
    *hdr = Header{
        .type = meta.type,
        .num_send_statics = meta.num_send_statics,
        .num_send_buffers = meta.num_send_buffers,
        .num_recv_buffers = meta.num_recv_buffers,
        .num_exch_buffers = meta.num_exch_buffers,
        .num_data_words = meta.num_data_words,
        .recv_static_mode =
            (meta.num_recv_statics != 0u)
                ? (meta.num_recv_statics != HIPC_AUTO_RECV_STATIC
                       ? 2u + meta.num_recv_statics
                       : 2u)
                : 0u,
        .padding = 0,
        .recv_list_offset = 0,
        .has_special_header = static_cast<u32>(has_special_header),
    };

    // Write special header
    if (has_special_header) {
        auto sphdr = reinterpret_cast<SpecialHeader*>(base);
        base = sphdr + 1;
        *sphdr = SpecialHeader{
            .send_pid = meta.send_pid,
            .num_copy_handles = meta.num_copy_handles,
            .num_move_handles = meta.num_move_handles,
        };
        if (meta.send_pid != 0u)
            base = reinterpret_cast<u8*>(base) + sizeof(u64);
    }

    // Calculate layout
    return calc_request_layout(meta, base);
}

u8* get_buffer_ptr(const hw::tegra_x1::cpu::IMmu* mmu,
                   const BufferDescriptor& descriptor, u64& size);

u8* get_static_ptr(const hw::tegra_x1::cpu::IMmu* mmu,
                   const StaticDescriptor& descriptor, u64& size);

u8* get_list_entry_ptr(const hw::tegra_x1::cpu::IMmu* mmu,
                       const RecvListEntry& descriptor, u64& size);

#define CREATE_STREAMS(buffer_or_static, type)                                 \
    type##_##buffer_or_static##s_streams.reserve(                              \
        hipc_in.meta.num_##type##_##buffer_or_static##s);                      \
    for (u32 i = 0; i < hipc_in.meta.num_##type##_##buffer_or_static##s;       \
         i++) {                                                                \
        u64 size;                                                              \
        u8* ptr = get_##buffer_or_static##_ptr(                                \
            mmu, hipc_in.data.type##_##buffer_or_static##s[i], size);          \
        type##_##buffer_or_static##s_streams.push_back(                        \
            ptr != nullptr ? std::make_optional<ztd::io::MemoryStream>(        \
                                 std::span(ptr, size))                         \
                           : std::nullopt);                                    \
    }

#define CREATE_STATIC_STREAMS(type) CREATE_STREAMS(static, type)
#define CREATE_BUFFER_STREAMS(type) CREATE_STREAMS(buffer, type)

struct Streams {
    ztd::io::MemoryStream in_stream;
    std::optional<ztd::io::MemoryStream> in_objects_stream{std::nullopt};
    ztd::io::MemoryStream in_copy_handles_stream;
    ztd::io::MemoryStream in_move_handles_stream;
    ztd::io::MemoryStream out_stream;
    ztd::io::MemoryStream out_objects_stream;
    ztd::io::MemoryStream out_copy_handles_stream;
    ztd::io::MemoryStream out_move_handles_stream;
    std::vector<std::optional<ztd::io::MemoryStream>> send_statics_streams;
    std::vector<std::optional<ztd::io::MemoryStream>> send_buffers_streams;
    std::vector<std::optional<ztd::io::MemoryStream>> recv_list_streams;
    std::vector<std::optional<ztd::io::MemoryStream>> recv_buffers_streams;
    std::vector<std::optional<ztd::io::MemoryStream>> exch_buffers_streams;

    Streams(const hw::tegra_x1::cpu::IMmu* mmu, ParsedRequest hipc_in,
            u8* scratch_buffer, u8* scratch_buffer_objects,
            u8* scratch_buffer_copy_handles, u8* scratch_buffer_move_handles)
        : in_stream(std::span(reinterpret_cast<u8*>(hipc_in.data.data_words),
                              hipc_in.meta.num_data_words * sizeof(u32))),
          in_copy_handles_stream(
              std::span(reinterpret_cast<u8*>(hipc_in.data.copy_handles),
                        hipc_in.meta.num_copy_handles * sizeof(Handle))),
          in_move_handles_stream(
              std::span(reinterpret_cast<u8*>(hipc_in.data.move_handles),
                        hipc_in.meta.num_move_handles * sizeof(Handle))),
          out_stream(std::span(scratch_buffer, 0x1000)),
          out_objects_stream(std::span(scratch_buffer_objects, 0x1000)),
          out_copy_handles_stream(
              std::span(scratch_buffer_copy_handles, 0x1000)),
          out_move_handles_stream(
              std::span(scratch_buffer_move_handles, 0x1000)) {
        CREATE_STATIC_STREAMS(send);
        CREATE_BUFFER_STREAMS(send);
        recv_list_streams.reserve(hipc_in.meta.num_recv_statics);
        for (u32 i = 0; i < hipc_in.meta.num_recv_statics; i++) {
            u64 size;
            u8* ptr = get_list_entry_ptr(mmu, hipc_in.data.recv_list[i], size);
            // TODO: should we continue or push std::nullopt in case of nullptr?
            recv_list_streams.push_back(
                ptr != nullptr ? std::make_optional<ztd::io::MemoryStream>(
                                     std::span(ptr, size))
                               : std::nullopt);
        }
        CREATE_BUFFER_STREAMS(recv);
        CREATE_BUFFER_STREAMS(exch);
    }
};

#undef CREATE_READERS_OR_WRITERS

} // namespace hydra::horizon::kernel::hipc
