#include "horizon/loader/nca_loader.hpp"

#include "horizon/filesystem/file.hpp"
#include "horizon/filesystem/filesystem.hpp"
#include "horizon/kernel.hpp"
#include "horizon/loader/nso_loader.hpp"

namespace Hydra::Horizon::Loader {

namespace {

enum class PartitionType {
    RomFS,
    PartitionFS,
};

struct PFS0Header {
    char magic[4];
    u32 entry_count;
    u32 string_table_size;
    u32 reserved;
};

struct PartitionEntry {
    u64 offset;
    u64 size;
    u32 string_offset;
    u32 reserved;
};

void load_pfs0(FileReader& reader, const std::string& rom_filename) {
    // Header
    const auto header = reader.Read<PFS0Header>();
    ASSERT(std::memcmp(header.magic, "PFS0", 4) == 0, HorizonLoader,
           "Invalid PFS0 magic");

    // Entries
    PartitionEntry entries[header.entry_count];
    reader.Read(entries, header.entry_count);

    // String table
    char string_table[header.string_table_size];
    reader.Read(string_table, header.string_table_size);

    // NSOs
    u64 nso_offset = reader.Tell();
    for (u32 i = 0; i < header.entry_count; i++) {
        const auto& entry = entries[i];
        const std::string entry_name(string_table + entry.string_offset);
        LOG_DEBUG(HorizonLoader, "{} -> offset: 0x{:08x}, size: 0x{:08x}",
                  entry_name, entry.offset, entry.size);

        // TODO: it doesn't always need to be ExeFS

        // HACK: main.npdm has some weird ro section
        if (entry_name == "main.npdm")
            continue;

        reader.Seek(nso_offset + entry.offset);
        NSOLoader loader(entry_name == "rtld");
        FileReader nso_reader = reader.CreateSubReader(entry.size);
        loader.LoadROM(nso_reader, rom_filename);
    }
}

void load_section(FileReader& reader, const std::string& rom_filename,
                  PartitionType type) {
    switch (type) {
    case PartitionType::PartitionFS: {
        reader.Seek(0x00004000);
        auto pfs0_reader = reader.CreateSubReader(0x0000008d0e92);
        load_pfs0(pfs0_reader, rom_filename);
        break;
    }
    case PartitionType::RomFS: {
        reader.Seek(0x00068000);
        auto romfs_reader = reader.CreateSubReader(0x0aed12c8);
        Filesystem::Filesystem::GetInstance().AddEntry(
            new Filesystem::File(rom_filename, romfs_reader.GetOffset(),
                                 romfs_reader.GetSize()),
            "/rom/romFS");
        break;
    }
    }
}

} // namespace

// TODO: don't hardcode stuff
void NCALoader::LoadROM(FileReader& reader, const std::string& rom_filename) {
    {
        reader.Seek(0x00000af58000);
        auto partition_reader = reader.CreateSubReader(0x0000008d8000);
        load_section(partition_reader, rom_filename,
                     PartitionType::PartitionFS);
    }
    {
        reader.Seek(0x00000001c000);
        auto partition_reader = reader.CreateSubReader(0x00000af3c000);
        load_section(partition_reader, rom_filename, PartitionType::RomFS);
    }
}

} // namespace Hydra::Horizon::Loader
