#include "core/horizon/filesystem/content_archive.hpp"

#include "core/horizon/filesystem/file_view.hpp"
#include "core/horizon/filesystem/partition_filesystem.hpp"

namespace hydra::horizon::filesystem {

namespace {

struct FsEntry {
    u32 start_offset;
    u32 end_offset;
    u64 reserved;
};

enum class FsType : u8 {
    RomFS,
    PartitionFS,
};

enum class HashType : u8 {
    Auto,
    None,
    HierarchicalSha256Hash,
    HierarchicalIntegrityHash,
    AutoSha3,
    HierarchicalSha3256Hash,
    HierarchicalIntegritySha3Hash,
};

enum class EncryptionType : u8 {
    Auto,
    None,
    AesXts,
    AesCtr,
    AesCtrEx,
    AesCtrSkipLayerHash,
    AesCtrExSkipLayerHash,
};

enum class MetaDataHashType : u8 {
    None,
    HierarchicalIntegrity,
};

#pragma pack(push, 1)
struct FsHeader {
    u16 version;
    FsType type;
    HashType hash_type;
    EncryptionType encryption_type;
    MetaDataHashType meta_data_hash_type;
    u8 reserved1[2];
    union {
        struct {
            u8 master_hash[0x20];
            u32 block_size;
            u32 layer_count; // Always 2
            struct {
                u64 offset;
                u64 size;
            } hash_table_region, pfs0_region;
            u8 reserved[0x80];
        } hierarchical_sha_256_data;

        struct {
            u32 magic;
            u32 version;
            u32 master_hash_size;
            struct {
                u32 max_layers;
                struct {
                    u64 logical_offset;
                    u64 hash_data_size;
                    u32 block_size_log2;
                    u32 reserved;
                } levels[6];
                u8 signature_salt[0x20];
            } info_level_hash;
        } integrity_meta_info;

        // TODO: more

        u8 hash_data_raw[0xf8];
    };
    u8 patch_info[0x40]; // TODO: struct
    u32 generation;
    u32 secure_value;
    u8 sparse_info[0x30];              // TODO: struct
    u8 compression_info[0x28];         // TODO: struct
    u8 meta_data_hash_data_info[0x30]; // TODO: struct
    u8 reserved2[0x30];
};
#pragma pack(pop)

enum class SectionType {
    Invalid,

    Code,
    Data,
    Logo,
};

enum class DistributionType : u8 {
    Download,
    GameCard,
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

struct Header {
    u8 signature0[0x100];
    u8 signature1[0x100];
    u32 magic;
    DistributionType distribution_type;
    ContentArchiveContentType content_type;
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
    u8 padding_0x340[0xc0];
    FsHeader fs_headers[FS_ENTRY_COUNT];

    SectionType get_section_type_from_index(const u32 index) const {
        if (content_type == ContentArchiveContentType::Program) {
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

} // namespace hydra::horizon::filesystem

ENABLE_ENUM_FORMATTING(hydra::horizon::filesystem::HashType, Auto, "auto", None,
                       "none", HierarchicalSha256Hash,
                       "hierarchical SHA 256 hash", HierarchicalIntegrityHash,
                       "hierarchical integrity hash", AutoSha3, "auto SHA3",
                       HierarchicalSha3256Hash, "hierarchical SHA3 256 hash",
                       HierarchicalIntegritySha3Hash,
                       "hierarchical integrity SHA3 hash")

namespace hydra::horizon::filesystem {

ContentArchive::ContentArchive(IFile* file) {
    auto stream = file->Open(FileOpenFlags::Read);

    // Header
    const auto header = stream->Read<Header>();
    // TODO: allow other NCA versions as well
    ASSERT(header.magic == make_magic4('N', 'C', 'A', '3'), Filesystem,
           "Invalid NCA magic 0x{:08x}", header.magic);

    content_type = header.content_type;
    title_id = header.program_id;

    // FS entries
    // TODO: don't iterate over all entries?
    for (u32 i = 0; i < FS_ENTRY_COUNT; i++) {
        const auto& entry = header.fs_entries[i];
        if (entry.start_offset == 0x0)
            continue;

        const auto type = header.get_section_type_from_index(i);
        if (type == SectionType::Invalid) {
            LOG_ERROR(Filesystem, "Invalid section type");
            continue;
        }

        const auto& fs_header = header.fs_headers[i];

        const u64 entry_offset = entry.start_offset * FS_BLOCK_SIZE;
        // const u64 entry_size = (entry.end_offset - entry.start_offset) *
        // FS_BLOCK_SIZE;

        switch (type) {
        case SectionType::Code:
        case SectionType::Logo: {
            ASSERT(fs_header.hash_type == HashType::HierarchicalSha256Hash,
                   Filesystem, "Invalid hash type \"{}\" for PFS0",
                   fs_header.hash_type);
            const auto& layer_region =
                fs_header.hierarchical_sha_256_data.pfs0_region;

            auto partition_file =
                new FileView(file, entry_offset + layer_region.offset,
                             layer_region.size); // TODO: free
            auto pfs = new PartitionFilesystem();
            pfs->Initialize<false>(partition_file);
            entries.insert(
                {(type == SectionType::Code ? "code" : "logo"), pfs});
            break;
        }
        case SectionType::Data: {
            // TODO: can other hash types be used as well?
            // ASSERT(fs_header.hash_type ==
            // HashType::HierarchicalIntegrityHash,
            //       Filesystem, "Invalid hash type \"{}\" for Data section",
            //       fs_header.hash_type);
            // TODO: correct?
            const auto& level = fs_header.integrity_meta_info.info_level_hash
                                    .levels[IVFC_MAX_LEVEL - 1];

            entries.insert(
                {"data", new FileView(file, entry_offset + level.logical_offset,
                                      level.hash_data_size)});
            break;
        }
        default:
            break;
        }
    }

    delete stream;
}

} // namespace hydra::horizon::filesystem
