#include "core/horizon/loader/nca_loader.hpp"

#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/filesystem/host_file.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/loader/pfs0_loader.hpp"

namespace hydra::horizon::loader {

namespace {

enum class DistributionType : u8 {
    Download,
    GameCard,
};

enum class ContentType : u8 {
    Program,
    Meta,
    Control,
    Manual,
    Data,
    PublicData,
};

enum class KeyGenerationOld : u8 {
    _1_0_0,
    Unused,
    _3_0_0,
};

enum class KeyAreaEncryptionKeyIndex : u8 {
    Application,
    Ocean,
    System,
};

enum class KeyGeneration : u8 {
    _3_0_1 = 3,
    _4_0_0,
    _5_0_0,
    _6_0_0,
    _6_2_0,
    _7_0_0,
    _8_1_0,
    _9_0_0,
    _9_1_0,
    _12_1_0,
    _13_0_0,
    _14_0_0,
    _15_0_0,
    _16_0_0,
    _17_0_0,
    _18_0_0,
    _19_0_0,

    Invalid = 0xff,
};

constexpr usize FS_BLOCK_SIZE = 0x200;
constexpr usize FS_ENTRY_COUNT = 4;
constexpr u32 IVFC_MAX_LEVEL = 6;

struct NcaHeader {
    u8 signature0[0x100];
    u8 signature1[0x100];
    u32 magic;
    DistributionType distribution_type;
    ContentType content_type;
    KeyGenerationOld key_generation_old;
    KeyAreaEncryptionKeyIndex key_area_encryption_key_index;
    usize content_size;
    u64 program_id;
    u32 content_index;
    u32 sdk_addon_version;
    KeyGeneration key_generation;
    u8 signature_key_generation;
    u8 reserved[0xe];
    u8 rights_id[0x10];
    FsEntry fs_entries[FS_ENTRY_COUNT];
    // TODO: correct?
    u8 section_hashes[4][0x20];
    u8 encrypted_keys[4][0x10];
    u8 padding_0x340[0xC0];
    FsHeader fs_headers[FS_ENTRY_COUNT]; /* FS section headers. */

    SectionType get_section_type_from_index(const u32 index) const {
        if (content_type == ContentType::Program) {
            switch (index) {
            case 0:
                return SectionType::Code;
            case 1:
                return SectionType::Data;
            case 2:
                return SectionType::Logo;
            default:
                return SectionType::Invalid;
            }
        } else {
            return SectionType::Data;
        }
    }
};

} // namespace

} // namespace hydra::horizon::loader

ENABLE_ENUM_FORMATTING(hydra::horizon::loader::HashType, Auto, "auto", None,
                       "none", HierarchicalSha256Hash,
                       "hierarchical SHA 256 hash", HierarchicalIntegrityHash,
                       "hierarchical integrity hash", AutoSha3, "auto SHA3",
                       HierarchicalSha3256Hash, "hierarchical SHA3 256 hash",
                       HierarchicalIntegritySha3Hash,
                       "hierarchical integrity SHA3 hash")

namespace hydra::horizon::loader {

namespace {

void load_section(StreamReader reader, const std::string_view rom_filename,
                  SectionType type, const FsHeader& header,
                  kernel::Process*& out_process) {
    switch (type) {
    case SectionType::Code: {
        ASSERT(header.hash_type == HashType::HierarchicalSha256Hash, Loader,
               "Invalid hash type \"{}\" for Code section", header.hash_type);
        const auto& layer_region = header.hierarchical_sha_256_data.pfs0_region;

        reader.Seek(layer_region.offset);
        auto pfs0_reader = reader.CreateSubReader(layer_region.size);
        Pfs0Loader pfs0_loader(pfs0_reader);
        auto process_ = pfs0_loader.LoadProcess(pfs0_reader, rom_filename);
        CHECK_AND_SET_PROCESS(out_process, process_);
        break;
    }
    case SectionType::Data: {
        // TODO: can other hash types be used as well?
        ASSERT(header.hash_type == HashType::HierarchicalIntegrityHash, Loader,
               "Invalid hash type \"{}\" for Data section", header.hash_type);
        // TODO: correct?
        const auto& level = header.integrity_meta_info.info_level_hash
                                .levels[IVFC_MAX_LEVEL - 1];

        reader.Seek(level.logical_offset);
        auto romfs_reader = reader.CreateSubReader(level.hash_data_size);
        const auto res = FILESYSTEM_INSTANCE.AddEntry(
            FS_SD_MOUNT "/rom/romFS",
            new filesystem::HostFile(rom_filename, romfs_reader.GetSize(),
                                     romfs_reader.GetOffset(), false),
            true);
        ASSERT(res == filesystem::FsResult::Success, Loader,
               "Failed to add romFS entry: {}", res);
        break;
    }
    case SectionType::Logo:
        LOG_NOT_IMPLEMENTED(Loader, "Logo loading");
        break;
    case SectionType::Invalid:
        LOG_ERROR(Loader, "Invalid section type");
        break;
    }
}

} // namespace

NcaLoader::NcaLoader(StreamReader reader) {
    // Header
    const auto header = reader.Read<NcaHeader>();
    // TODO: allow other NCA versions as well
    ASSERT(header.magic == make_magic4('N', 'C', 'A', '3'), Loader,
           "Invalid NCA magic");

    title_id = header.program_id;

    // FS entries
    // TODO: don't iterate over all entries?
    for (u32 i = 0; i < FS_ENTRY_COUNT; i++) {
        sections.push_back({header.get_section_type_from_index(i),
                            header.fs_entries[i], header.fs_headers[i]});
    }
}

kernel::Process* NcaLoader::LoadProcess(StreamReader reader,
                                        const std::string_view rom_filename) {
    // Title ID
    KERNEL_INSTANCE.SetTitleId(title_id);

    kernel::Process* process = nullptr;

    // FS entries
    for (const auto& section : sections) {
        const auto& entry = section.fs_entry;

        reader.Seek(entry.start_offset * FS_BLOCK_SIZE);
        auto entry_reader = reader.CreateSubReader(
            (entry.end_offset - entry.start_offset) * FS_BLOCK_SIZE);

        load_section(entry_reader, rom_filename, section.type,
                     section.fs_header, process);
    }

    CHECK_AND_RETURN_PROCESS(process);
}

} // namespace hydra::horizon::loader
