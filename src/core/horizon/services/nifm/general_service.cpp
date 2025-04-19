#include "core/horizon/services/nifm/general_service.hpp"

#include "core/horizon/services/nifm/request.hpp"

namespace Hydra::Horizon::Services::Nifm {

namespace {

#pragma pack(push, 1)
struct IpV4Address {
    u8 addr[4];
};

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
    u128 uuid;
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
                             GetCurrentNetworkProfile)

void IGeneralService::CreateRequest(REQUEST_COMMAND_PARAMS) {
    const auto requirement_preset = readers.reader.Read<i32>();
    LOG_DEBUG(HorizonServices, "Requirement preset: {}", requirement_preset);

    add_service(new IRequest());
}

void IGeneralService::GetCurrentNetworkProfile(REQUEST_COMMAND_PARAMS) {
    LOG_NOT_IMPLEMENTED(HorizonServices, "GetCurrentNetworkProfile");

    // HACK
    writers.recv_list_writers[0].Write<SfNetworkProfileData>(
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
         .network_name = "Hydra-network",
         .wireless_setting_data = {
             .ssid = {0x7},
             .passphrase = {0x13},
         }});
}

} // namespace Hydra::Horizon::Services::Nifm
