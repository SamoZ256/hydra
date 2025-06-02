#include "core/horizon/loader/nso_loader.hpp"

#include "common/lz4.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::loader {

namespace {

enum class NsoFlags : u32 {
    None = 0,

    TextCompressed = BIT(0),
    RoCompressed = BIT(1),
    DataCompressed = BIT(2),
    TextHash = BIT(3),
    RoHash = BIT(4),
    DataHash = BIT(5),
};
ENABLE_ENUM_BITMASK_OPERATORS(NsoFlags)

struct NsoHeader {
    u32 magic;
    u32 version;
    u32 reserved1;
    NsoFlags flags;
    Segment text;
    u32 module_name_offset;
    Segment ro;
    u32 module_name_size;
    Segment data;
    u32 bss_size;
    u32 module_id[0x8];
    u32 text_file_size;
    u32 ro_file_size;
    u32 data_file_size;
    u8 reserved2[0x1c];
    u32 embedded_offset;
    u32 embedded_size;
    u32 dyn_str_offset;
    u32 dyn_str_size;
    u32 dyn_sym_offset;
    u32 dyn_sym_size;
    u32 text_hash[0x8];
    u32 ro_hash[0x8];
    u32 data_hash[0x8];
};

void read_segment(StreamReader reader, uptr executable_mem_ptr,
                  const Segment& segment, const usize segment_file_size,
                  bool is_compressed) {
    // Skip
    reader.Seek(segment.file_offset);

    usize file_size = (is_compressed ? segment_file_size : segment.size);

    if (is_compressed) {
        // Decompress
        auto file = new u8[file_size];
        reader.ReadPtr(file, file_size);
        decompress_lz4(
            file, file_size,
            reinterpret_cast<u8*>(executable_mem_ptr + segment.memory_offset),
            segment.size);
        delete[] file;
    } else {
        reader.ReadPtr(
            reinterpret_cast<u8*>(executable_mem_ptr + segment.memory_offset),
            file_size);
    }
}

struct ArgData {
    u32 allocated_size;
    u32 string_size;
    u8 unused[0x18];
    char str[];
};

// TODO: what should this be?
constexpr usize ARG_DATA_SIZE = 0x9000;

} // namespace

NsoLoader::NsoLoader(StreamReader reader, const bool is_entry_point_)
    : is_entry_point{is_entry_point_} {
    // Header
    const auto header = reader.Read<NsoHeader>();
    ASSERT(header.magic == make_magic4('N', 'S', 'O', '0'), Loader,
           "Invalid NSO magic");

    text_offset = header.text.memory_offset;

    // Determine executable memory size
    executable_size =
        std::max(executable_size, static_cast<usize>(header.text.memory_offset +
                                                     header.text.size));
    executable_size =
        std::max(executable_size,
                 static_cast<usize>(header.ro.memory_offset + header.ro.size));
    executable_size =
        std::max(executable_size, static_cast<usize>(header.data.memory_offset +
                                                     header.data.size));
    executable_size += header.bss_size;
    LOG_DEBUG(Loader,
              "NSO: 0x{:08x} + 0x{:08x}, 0x{:08x} + 0x{:08x}, 0x{:08x} + "
              "0x{:08x}, 0x{:08x}",
              header.text.memory_offset, header.text.size,
              header.ro.memory_offset, header.ro.size,
              header.data.memory_offset, header.data.size, header.bss_size);

    // Segments
    segments[0] = {header.text, header.text_file_size,
                   any(header.flags & NsoFlags::TextCompressed)};
    segments[1] = {header.ro, header.ro_file_size,
                   any(header.flags & NsoFlags::RoCompressed)};
    segments[2] = {header.data, header.data_file_size,
                   any(header.flags & NsoFlags::DataCompressed)};
}

kernel::Process* NsoLoader::LoadProcess(StreamReader reader,
                                        const std::string_view rom_filename) {
    // Create executable memory
    vaddr_t base;
    auto ptr = KERNEL_INSTANCE.CreateExecutableMemory(
        executable_size, kernel::MemoryPermission::ReadExecute, false, base);
    LOG_DEBUG(Loader, "Base: 0x{:08x}, size: 0x{:08x}", base, executable_size);

    // Segments
    for (u32 i = 0; i < 3; i++) {
        const auto& segment = segments[i];
        read_segment(reader, ptr, segment.seg, segment.file_size,
                     segment.compressed);
    }

    // Arg data
    // TODO: don't hardcode
    std::string arg_data_str = "";

    vaddr_t arg_data_base;
    // TODO: memory type
    auto arg_data_ptr =
        reinterpret_cast<ArgData*>(KERNEL_INSTANCE.CreateRomMemory(
            ARG_DATA_SIZE, static_cast<kernel::MemoryType>(4),
            kernel::MemoryPermission::ReadWrite, true, arg_data_base));
    arg_data_ptr->allocated_size = ARG_DATA_SIZE;
    arg_data_ptr->string_size = arg_data_str.size() + 1;
    std::memcpy(arg_data_ptr->str, arg_data_str.c_str(), arg_data_str.size());

    // Debug
#define DUMP 0
#if DUMP
    std::ofstream out(
        fmt::format("/Users/samuliak/Downloads/extracted/0x{:08x}.bin", base),
        std::ios::binary);
    out.write(reinterpret_cast<const char*>(ptr), executable_size);
    out.close();
#endif

    // Process
    if (is_entry_point) {
        kernel::Process* process = new kernel::Process();
        auto& main_thread = process->GetMainThread();
        main_thread.handle->SetEntryPoint(base + text_offset);
        main_thread.handle->SetArg(0, 0x0);
        main_thread.handle->SetArg(1, main_thread.id);

        return process;
    } else {
        return nullptr;
    }
}

} // namespace hydra::horizon::loader
