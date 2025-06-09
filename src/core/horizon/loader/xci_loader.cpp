#include "core/horizon/loader/xci_loader.hpp"

#include "core/horizon/loader/nca_loader.hpp"

namespace hydra::horizon::loader {

namespace {

constexpr usize PAGE_SIZE = 0x200;

enum class RomSize : u8 {
    _1GB = 0xfa,
    _2GB = 0xf8,
    _4GB = 0xf0,
    _8GB = 0xe0,
    _16GB = 0xe1,
    _32GB = 0xe2,
};

enum class Version : u8 {
    Default = 0,
    Unknown1 = 1,
    Unknown2 = 2,
    T2Supported = 3, // 20.0.0+
};

enum class Flags : u8 {
    None = 0,
    AutoBoot = BIT(0),
    HistoryErase = BIT(1),
    RepairTool = BIT(2),                       // 4.0.0+
    DifferentRegionCupToTerraDevice = BIT(3),  // 9.0.0+
    DifferentRegionCupToGlobalDevice = BIT(4), // 9.0.0+
    CardHeaderSignKey = BIT(7),                // 11.0.0+
};
ENABLE_ENUM_BITMASK_OPERATORS(Flags)

enum class SelSec : u32 {
    T1 = 1,
    T2 = 2,
};

struct XciHeader {
    u8 signature[0x100];
    u32 magic;
    u32 rom_area_start_page_address; // in pages
    u32 backup_area_start_page_address;
    // TODO: correct?
    u8 kek_index : 4;
    u8 title_key_dec_index : 4;
    RomSize rom_size;
    Version version;
    Flags flags;
    u64 package_id;
    u32 valid_data_end_address; // in pages
    u32 _reserved_x11c;
    u8 _reserved_x120[0x10]; // TODO: iv?
    u64 partition_fs_header_address;
    u64 partition_fs_header_size;
    u8 partition_fs_header_hash[0x20];
    u8 initial_data_hash[0x20];
    SelSec sel_sec;
    u32 sel_t1_key; // always 2
    u32 sel_key;    // always 0
    u32 lim_area;   // in pages
} PACKED;

} // namespace

} // namespace hydra::horizon::loader

ENABLE_ENUM_FORMATTING(hydra::horizon::loader::RomSize, _1GB, "1 GB", _2GB,
                       "2 GB", _4GB, "4 GB", _8GB, "8 GB", _16GB, "16 GB",
                       _32GB, "32 GB")

namespace hydra::horizon::loader {

namespace {

usize get_rom_size(const RomSize size) {
    // TODO: use GB, not GiB
    switch (size) {
    case RomSize::_1GB:
        return 1_GiB;
    case RomSize::_2GB:
        return 2_GiB;
    case RomSize::_4GB:
        return 4_GiB;
    case RomSize::_8GB:
        return 8_GiB;
    case RomSize::_16GB:
        return 16_GiB;
    case RomSize::_32GB:
        return 32_GiB;
    default:
        LOG_ERROR(Loader, "Unknown ROM size {}", size);
        return 0;
    }
}

} // namespace

XciLoader::XciLoader(StreamReader reader) {
    // Header
    const auto header = reader.Read<XciHeader>();
    ASSERT(header.magic == make_magic4('H', 'E', 'A', 'D'), Loader,
           "Invalid XCI magic");

    {
        // TODO
        // LoadNormalArea(reader);
    }

    {
        reader.Seek(header.rom_area_start_page_address * PAGE_SIZE);
        auto rom_reader = reader.CreateSubReader(get_rom_size(header.rom_size));
        LoadRomArea(rom_reader);
    }
}

kernel::Process* XciLoader::LoadProcess(StreamReader reader,
                                        const std::string_view rom_filename) {
    kernel::Process* process = nullptr;

    // NCAs
    // HACK: find the largest one
    FileEntry largest_entry{.size = 0};
    for (const auto& entry : entries) {
        const std::string entry_name(string_table.data() + entry.string_offset);
        LOG_DEBUG(Loader, "{} -> offset: 0x{:08x}, size: 0x{:08x}", entry_name,
                  entry.offset, entry.size);

        if (entry.size > largest_entry.size)
            largest_entry = entry;
    }

    reader.Seek(entries_offset + largest_entry.offset);
    auto nca_reader = reader.CreateSubReader(largest_entry.size);
    NcaLoader loader(nca_reader);
    auto process_ = loader.LoadProcess(nca_reader, rom_filename);
    if (process_)
        CHECK_AND_SET_PROCESS(process, process_);

    for (u32 i = 0; i < 256; i++) {
        LOG_INFO(Loader, "DATA: 0x{:08x}", nca_reader.Read<u32>());
    }

    CHECK_AND_RETURN_PROCESS(process);
}

void XciLoader::LoadNormalArea(StreamReader reader) {
    LOG_FUNC_NOT_IMPLEMENTED(Loader);
}

// TODO: move this elsewhere
namespace {

struct PartitionFsHeader {
    u32 magic;
    u32 file_count;
    u32 string_table_size;
    u32 _reserved_xc;
};

} // namespace

void XciLoader::LoadRomArea(StreamReader reader) {
    const auto header = reader.Read<PartitionFsHeader>();
    ASSERT(header.magic == make_magic4('H', 'F', 'S', '0'), Loader,
           "Invalid HFS0 magic");

    // Entries
    entries.resize(header.file_count);
    reader.ReadPtr(entries.data(), header.file_count);

    // String table
    string_table.resize(header.string_table_size);
    reader.ReadPtr(string_table.data(), header.string_table_size);

    entries_offset = reader.Tell();
}

} // namespace hydra::horizon::loader
