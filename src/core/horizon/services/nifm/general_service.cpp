#include "core/horizon/services/nifm/general_service.hpp"

#include "core/horizon/services/nifm/request.hpp"

namespace hydra::horizon::services::nifm {

namespace {

#pragma pack(push, 1)
struct IpAddressSetting {
    u8 is_automatic;
    IpV4Address current_addr;
    IpV4Address subnet_mask;
    IpV4Address gateway;
};

struct DnsSetting {
    u8 is_automatic;
    IpV4Address primary_dns_server;
    IpV4Address secondary_dns_server;
};

struct ProxySetting {
    u8 enabled;
    u8 pad;
    u16 port;
    char server[0x64];
    u8 auto_auth_enabled;
    char user[0x20];
    char password[0x20];
    u8 pad2;
};

struct IpSettingData {
    IpAddressSetting ip_address_setting;
    DnsSetting dns_setting;
    ProxySetting proxy_setting;
    u16 mtu;
};

struct SfWirelessSettingData {
    u8 ssid_len;
    char ssid[0x20];
    u8 unk_x21;
    u8 unk_x22;
    u8 unk_x23;
    u8 passphrase[0x41];
};

struct SfNetworkProfileData {
    IpSettingData ip_setting_data;
    uuid_t uuid;
    char network_name[0x40];
    u8 unk_x112;
    u8 unk_x113;
    u8 unk_x114;
    u8 unk_x115;
    SfWirelessSettingData wireless_setting_data;
    u8 pad;
};
#pragma pack(pop)

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(IGeneralService, 4, CreateRequest, 5,
                             GetCurrentNetworkProfile, 12, GetCurrentIpAddress,
                             18, GetInternetConnectionStatus)

result_t IGeneralService::CreateRequest(add_service_fn_t add_service,
                                        i32 requirement_preset) {
    LOG_DEBUG(Services, "Requirement preset: {}", requirement_preset);

    add_service(new IRequest());
    return RESULT_SUCCESS;
}

result_t IGeneralService::GetCurrentNetworkProfile(
    OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    LOG_NOT_IMPLEMENTED(Services, "GetCurrentNetworkProfile");

    // HACK
    out_buffer.writer->Write<SfNetworkProfileData>(
        {.ip_setting_data =
             {
                 .ip_address_setting =
                     {
                         .is_automatic = 0x1,
                         .current_addr = {.addr = {0x7, 0x7, 0x7, 0x7}},
                         .subnet_mask = {.addr = {0xFF, 0xFF, 0xFF, 0xFF}},
                         .gateway = {.addr = {0x8, 0x8, 0x8, 0x8}},
                     },
                 .dns_setting =
                     {
                         .is_automatic = 0x1,
                         .primary_dns_server = {.addr = {0x1, 0x1, 0x1, 0x1}},
                         .secondary_dns_server = {.addr = {0x2, 0x2, 0x2, 0x2}},
                     },
                 .proxy_setting =
                     {
                         .enabled = 0x1,
                         .port = 0x1234,
                         .server = {0x3},
                         .auto_auth_enabled = 0x1,
                         .user = {0x4},
                         .password = {0x5},
                     },
             },
         .uuid = 0x76543210,
         .network_name = "hydra-network",
         .wireless_setting_data = {
             .ssid = {0x7},
             .passphrase = {0x13},
         }});
    return RESULT_SUCCESS;
}

result_t IGeneralService::GetCurrentIpAddress(IpV4Address* out_ip) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_ip = {0};
    return RESULT_SUCCESS;
}

result_t IGeneralService::GetInternetConnectionStatus(
    InternetConnectionType* out_type, u32* out_wifi_strength,
    InternetConnectionStatus* out_status) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_type = InternetConnectionType::WiFi;
    *out_wifi_strength = 0;
    *out_status = InternetConnectionStatus::Connected;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::nifm
