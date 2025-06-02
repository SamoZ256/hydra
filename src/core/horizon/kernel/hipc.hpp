#pragma once

#include "core/horizon/kernel/const.hpp"

namespace hydra::hw::tegra_x1::cpu {
class MMUBase;
}

namespace hydra::horizon::kernel::hipc {

#define HIPC_AUTO_RECV_STATIC UINT8_MAX
#define HIPC_RESPONSE_NO_PID UINT32_MAX

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
    bool has_special_header : 1;
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
    handle_id_t* copy_handles;
    handle_id_t* move_handles;
};

// From https://github.com/switchbrew/libnx
struct Request {
    StaticDescriptor* send_statics;
    BufferDescriptor* send_buffers;
    BufferDescriptor* recv_buffers;
    BufferDescriptor* exch_buffers;
    u32* data_words;
    RecvListEntry* recv_list;
    handle_id_t* copy_handles;
    handle_id_t* move_handles;
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
    handle_id_t* copy_handles = NULL;
    if (meta.num_copy_handles) {
        copy_handles = (handle_id_t*)base;
        base = copy_handles + meta.num_copy_handles;
    }

    // Move handles
    handle_id_t* move_handles = NULL;
    if (meta.num_move_handles) {
        move_handles = (handle_id_t*)base;
        base = move_handles + meta.num_move_handles;
    }

    // Send statics
    StaticDescriptor* send_statics = NULL;
    if (meta.num_send_statics) {
        send_statics = (StaticDescriptor*)base;
        base = send_statics + meta.num_send_statics;
    }

    // Send buffers
    BufferDescriptor* send_buffers = NULL;
    if (meta.num_send_buffers) {
        send_buffers = (BufferDescriptor*)base;
        base = send_buffers + meta.num_send_buffers;
    }

    // Recv buffers
    BufferDescriptor* recv_buffers = NULL;
    if (meta.num_recv_buffers) {
        recv_buffers = (BufferDescriptor*)base;
        base = recv_buffers + meta.num_recv_buffers;
    }

    // Exch buffers
    BufferDescriptor* exch_buffers = NULL;
    if (meta.num_exch_buffers) {
        exch_buffers = (BufferDescriptor*)base;
        base = exch_buffers + meta.num_exch_buffers;
    }

    // Data words
    u32* data_words = NULL;
    if (meta.num_data_words) {
        data_words = (u32*)base;
        base = data_words + meta.num_data_words;
    }

    // Recv list
    RecvListEntry* recv_list = NULL;
    if (meta.num_recv_statics)
        recv_list = (RecvListEntry*)base;

    return (Request){
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
    base = (u8*)base + sizeof(hdr);
    u32 num_recv_statics = 0;
    u64 pid = 0;

    // Parse recv static mode
    if (hdr.recv_static_mode) {
        if (hdr.recv_static_mode == 2u)
            num_recv_statics = HIPC_AUTO_RECV_STATIC;
        else if (hdr.recv_static_mode > 2u)
            num_recv_statics = hdr.recv_static_mode - 2u;
    }

    // Parse special header
    SpecialHeader sphdr = {};
    if (hdr.has_special_header) {
        memcpy(&sphdr, base, sizeof(sphdr));
        base = (u8*)base + sizeof(sphdr);

        // Read PID descriptor
        if (sphdr.send_pid) {
            pid = *(u64*)base;
            base = (u8*)base + sizeof(u64);
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
    bool has_special_header =
        meta.send_pid || meta.num_copy_handles || meta.num_move_handles;
    Header* hdr = (Header*)base;
    base = hdr + 1;
    *hdr = (Header){
        .type = meta.type,
        .num_send_statics = meta.num_send_statics,
        .num_send_buffers = meta.num_send_buffers,
        .num_recv_buffers = meta.num_recv_buffers,
        .num_exch_buffers = meta.num_exch_buffers,
        .num_data_words = meta.num_data_words,
        .recv_static_mode =
            meta.num_recv_statics
                ? (meta.num_recv_statics != HIPC_AUTO_RECV_STATIC
                       ? 2u + meta.num_recv_statics
                       : 2u)
                : 0u,
        .padding = 0,
        .recv_list_offset = 0,
        .has_special_header = has_special_header,
    };

    // Write special header
    if (has_special_header) {
        SpecialHeader* sphdr = (SpecialHeader*)base;
        base = sphdr + 1;
        *sphdr = (SpecialHeader){
            .send_pid = meta.send_pid,
            .num_copy_handles = meta.num_copy_handles,
            .num_move_handles = meta.num_move_handles,
        };
        if (meta.send_pid)
            base = (u8*)base + sizeof(u64);
    }

    // Calculate layout
    return calc_request_layout(meta, base);
}

u8* get_buffer_ptr(const hw::tegra_x1::cpu::MMUBase* mmu,
                   const BufferDescriptor& descriptor, usize& size);

u8* get_static_ptr(const hw::tegra_x1::cpu::MMUBase* mmu,
                   const StaticDescriptor& descriptor, usize& size);

u8* get_list_entry_ptr(const hw::tegra_x1::cpu::MMUBase* mmu,
                       const RecvListEntry& descriptor, usize& size);

#define CREATE_READERS_OR_WRITERS(buffer_or_static, reader_or_writer, type)    \
    type##_##buffer_or_static##s_##reader_or_writer##s.reserve(                \
        hipc_in.meta.num_##type##_##buffer_or_static##s);                      \
    for (u32 i = 0; i < hipc_in.meta.num_##type##_##buffer_or_static##s;       \
         i++) {                                                                \
        usize size;                                                            \
        u8* ptr = get_##buffer_or_static##_ptr(                                \
            mmu, hipc_in.data.type##_##buffer_or_static##s[i], size);          \
        type##_##buffer_or_static##s_##reader_or_writer##s.emplace_back(ptr,   \
                                                                        size); \
    }

#define CREATE_STATIC_READERS_OR_WRITERS(reader_or_writer, type)               \
    CREATE_READERS_OR_WRITERS(static, reader_or_writer, type)
#define CREATE_BUFFER_READERS_OR_WRITERS(reader_or_writer, type)               \
    CREATE_READERS_OR_WRITERS(buffer, reader_or_writer, type)

struct Readers {
    Reader reader;
    Reader* objects_reader;
    Reader copy_handles_reader;
    Reader move_handles_reader;
    std::vector<Reader> send_statics_readers;
    std::vector<Reader> send_buffers_readers;
    std::vector<Reader> exch_buffers_readers;

    Readers(const hw::tegra_x1::cpu::MMUBase* mmu, ParsedRequest hipc_in)
        : reader(align_ptr((u8*)hipc_in.data.data_words, 0x10),
                 hipc_in.meta.num_data_words * sizeof(u32)),
          objects_reader{nullptr},
          copy_handles_reader((u8*)hipc_in.data.copy_handles,
                              hipc_in.meta.num_copy_handles *
                                  sizeof(handle_id_t)),
          move_handles_reader((u8*)hipc_in.data.move_handles,
                              hipc_in.meta.num_move_handles *
                                  sizeof(handle_id_t)) {
        CREATE_STATIC_READERS_OR_WRITERS(reader, send);
        CREATE_BUFFER_READERS_OR_WRITERS(reader, send);
        CREATE_BUFFER_READERS_OR_WRITERS(reader, exch);
    }

    ~Readers() {
        if (objects_reader)
            delete objects_reader;
    }
};

struct Writers {
    Writer writer;
    Writer objects_writer;
    Writer copy_handles_writer;
    Writer move_handles_writer;
    std::vector<Writer> recv_list_writers;
    std::vector<Writer> recv_buffers_writers;
    std::vector<Writer> exch_buffers_writers;

    Writers(const hw::tegra_x1::cpu::MMUBase* mmu, ParsedRequest hipc_in,
            u8* scratch_buffer, u8* scratch_buffer_objects,
            u8* scratch_buffer_copy_handles, u8* scratch_buffer_move_handles)
        : writer(scratch_buffer, 0x1000),
          objects_writer(scratch_buffer_objects, 0x1000),
          copy_handles_writer(scratch_buffer_copy_handles, 0x1000),
          move_handles_writer(scratch_buffer_move_handles, 0x1000) {
        recv_list_writers.reserve(hipc_in.meta.num_recv_statics);
        for (u32 i = 0; i < hipc_in.meta.num_recv_statics; i++) {
            usize size;
            u8* ptr = get_list_entry_ptr(mmu, hipc_in.data.recv_list[i], size);
            if (!ptr)
                continue;
            recv_list_writers.emplace_back(ptr, size);
        }
        CREATE_BUFFER_READERS_OR_WRITERS(writer, recv);
        CREATE_BUFFER_READERS_OR_WRITERS(writer, exch);
    }
};

#undef CREATE_READERS_OR_WRITERS

} // namespace hydra::horizon::kernel::hipc
