#pragma once

#include "core/horizon/loader/loader_base.hpp"

namespace hydra::horizon::loader {

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

struct FsSection {
    SectionType type;
    FsEntry fs_entry;
    FsHeader fs_header;
};

class NcaLoader : public LoaderBase {
  public:
    NcaLoader(StreamReader reader);

    kernel::Process* LoadProcess(StreamReader reader,
                                 const std::string_view rom_filename) override;

  private:
    u64 title_id;
    std::vector<FsSection> sections;
};

} // namespace hydra::horizon::loader
