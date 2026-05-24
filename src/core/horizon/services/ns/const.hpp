#pragma once

namespace hydra::horizon::services::ns {

struct ApplicationTitle {
    char name[0x200];
    char author[0x100];

    bool IsValid() const { return name[0] != '\0'; }
};

enum class VideoCapture : u8 {
    Disabled = 0,
    Enabled = 1,
    Automatic = 2,
};

enum class PlayLogPolicy : u8 {
    All = 0,
    LogOnly = 1,
    None = 2,
    AllNew = 3, // 10.0.0+ (TODO: correct?)
};

struct NeighborDetectionGroupConfiguration {
    u64 group_id;
    u8 key[0x10];
};

struct NeighborDetectionClientConfiguration {
    NeighborDetectionGroupConfiguration send_group_configuration;
    NeighborDetectionGroupConfiguration receivable_group_configurations[0x10];
};

typedef struct {
    u64 flags;
    u64 memory_size;
} JitConfiguration;

// TODO: adjust this according to switchbrew
struct ApplicationControlProperty {
    ApplicationTitle titles[16];
    u8 isbn[0x25];
    u8 startup_user_account;
    u8 user_account_switch_lock;
    u8 add_on_content_registration_type;
    u32 attribute_flag;
    u32 supported_language_flag;
    u32 parental_control_flag;
    u8 screenshot;
    VideoCapture video_capture;
    u8 data_loss_confirmation;
    PlayLogPolicy play_log_policy;
    u64 presence_group_id;
    i8 rating_age[0x20];
    char display_version[0x10];
    u64 add_on_content_base_id;
    u64 save_data_owner_id;
    u64 user_account_save_data_size;
    u64 user_account_save_data_journal_size;
    u64 device_save_data_size;
    u64 device_save_data_journal_size;
    u64 bcat_delivery_cache_storage_size;
    u64 application_error_code_category;
    u64 local_communication_id[0x8];
    u8 logo_type;
    u8 logo_handling;
    u8 runtime_add_on_content_install;
    u8 runtime_parameter_delivery;
    u8 reserved_x30f4[0x2];
    u8 crash_report;
    u8 hdcp;
    u64 pseudo_device_id_seed;
    char bcat_passphrase[0x41];
    u8 startup_user_account_option;
    u8 reserved_for_user_account_save_data_operation[0x6];
    u64 user_account_save_data_size_max;
    u64 user_account_save_data_journal_size_max;
    u64 device_save_data_size_max;
    u64 device_save_data_journal_size_max;
    u64 temporary_storage_size;
    u64 cache_storage_size;
    u64 cache_storage_journal_size;
    u64 cache_storage_data_and_journal_size_max;
    u16 cache_storage_index_max;
    u8 reserved_x318a[0x6];
    u64 play_log_queryable_application_id[0x10];
    u8 play_log_query_capability;
    u8 repair_flag;
    u8 program_index;
    u8 required_network_service_license_on_launch;
    u8 application_error_code_prefix; // 20.0.0+
    u8 reserved_x3215;
    u8 acd_index;         // 20.0.0+
    u8 apparent_platform; // 20.0.0+
    NeighborDetectionClientConfiguration
        neighbor_detection_client_configuration;
    JitConfiguration jit_configuration;
    u8 reserved_x33c0[0xc40]; // TODO: there is more stuff here

    const ApplicationTitle& GetApplicationTitle(SystemLanguage lang) const;
};

} // namespace hydra::horizon::services::ns
