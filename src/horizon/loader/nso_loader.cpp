#include "horizon/loader/nso_loader.hpp"

#include "common/lz4.hpp"
#include "horizon/kernel.hpp"

namespace Hydra::Horizon::Loader {

namespace {

struct Segment {
    u32 file_offset;
    u32 memory_offset;
    u32 size;
};

struct NSOHeader {
    char magic[4];
    u32 version;
    u32 reserved1;
    u32 flags;
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

void read_segment(FileReader& reader, uptr executable_mem_ptr,
                  const Segment& segment, const usize segment_file_size,
                  bool is_compressed) {
    // Skip
    reader.Seek(segment.file_offset);

    usize file_size = (is_compressed ? segment_file_size : segment.size);

    if (is_compressed) {
        // Decompress
        u8 file[file_size];
        reader.Read(file, file_size);
        decompress_lz4(
            file, file_size,
            reinterpret_cast<u8*>(executable_mem_ptr + segment.memory_offset),
            segment.size);
    } else {
        reader.Read(
            reinterpret_cast<u8*>(executable_mem_ptr + segment.memory_offset),
            file_size);
    }
}

} // namespace

void NSOLoader::LoadROM(FileReader& reader, const std::string& rom_filename) {
    // Header
    const auto header = reader.Read<NSOHeader>();
    ASSERT(std::memcmp(header.magic, "NSO0", 4) == 0, HorizonLoader,
           "Invalid NSO magic");

    // Determine executable memory size
    usize executable_size = 0;
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
    LOG_DEBUG(HorizonLoader,
              "NSO: 0x{:08x} + 0x{:08x}, 0x{:08x} + 0x{:08x}, 0x{:08x} + "
              "0x{:08x}, 0x{:08x}",
              header.text.memory_offset, header.text.size,
              header.ro.memory_offset, header.ro.size,
              header.data.memory_offset, header.data.size, header.bss_size);

    // Create executable memory
    uptr base;
    auto ptr = Kernel::GetInstance().CreateExecutableMemory(
        executable_size, base, MemoryPermission::ReadExecute);
    LOG_DEBUG(HorizonLoader, "Base: 0x{:08x}", base);

    // Segments
    read_segment(reader, ptr, header.text, header.text_file_size,
                 (header.flags & (1u << 0)));
    read_segment(reader, ptr, header.ro, header.ro_file_size,
                 (header.flags & (1u << 1)));
    read_segment(reader, ptr, header.data, header.data_file_size,
                 (header.flags & (1u << 2)));

    if (is_entry_point) {
        // Set entrypoint
        Kernel::GetInstance().SetMainThreadEntryPoint(
            base + header.text.memory_offset);

        // Args
        Kernel::GetInstance().SetMainThreadArg(0, 0x0);
        Kernel::GetInstance().SetMainThreadArg(
            1, 0x0000000f); // TODO: what thread handle should be used?
    }

    // Debug
#define DUMP 0
#if DUMP
    std::ofstream out(
        fmt::format("/Users/samuliak/Downloads/extracted/0x{:08x}.bin", base),
        std::ios::binary);
    out.write(reinterpret_cast<const char*>(mem->GetPtrU8()), executable_size);
    out.close();
#endif
}

} // namespace Hydra::Horizon::Loader
