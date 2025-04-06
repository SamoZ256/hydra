#include "horizon/services/account/profile.hpp"

// TODO: make all of this configurable

namespace Hydra::Horizon::Services::Account {

namespace {

struct AccountUserData {
    u32 unk_x0;
    u32 icon_id;
    u8 icon_background_color_id;
    u8 unk_x9[0x7];
    u8 mii_id[0x10];
    u8 unk_x20[0x60];
};

struct AccountProfileBase {
    u128 uid;
    u64 last_edit_timestamp;
    char nickname[0x20];
};

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(IProfile, 0, Get)

void IProfile::Get(REQUEST_COMMAND_PARAMS) {
    // TODO: get this from state manager
    AccountUserData user_data{
        .unk_x0 = 0,
        .icon_id = 0,
        .icon_background_color_id = 0,
        .unk_x9 = {0},
        .mii_id = {0},
        .unk_x20 = {0},
    };
    writers.recv_list_writers[0].Write(user_data);

    // TODO: get this from state manager
    AccountProfileBase profile_base{
        .uid = account_uid,
        .last_edit_timestamp = 0,
        .nickname = "Hydra user",
    };
    writers.writer.Write(profile_base);
}

} // namespace Hydra::Horizon::Services::Account
