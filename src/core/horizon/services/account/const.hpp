#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::account {

constexpr usize NICKNAME_SIZE = 0x20;

struct ProfileBase {
    uuid_t user_id;
    u64 last_edit_timestamp;
    char nickname[0x20];
};

struct UserData {
    u32 unk_x0;
    u32 icon_id;
    u8 icon_background_color_id;
    u8 unk_x9[0x7];
    u8 mii_id[0x10];
    u8 unk_x20[0x60];
};

constexpr uuid_t INVALID_USER_ID = 0x0;

class User {
    friend class UserManager;

  public:
    User() {}
    User(const std::string_view nickname) {
        // TODO: don't use the setters?
        SetNickname(nickname);
    }

    bool EditedSince(u64 timestamp) const {
        return base.last_edit_timestamp > timestamp;
    }

    // Getters
    const ProfileBase& GetBase() const { return base; }
    const UserData& GetData() const { return data; }

    u64 GetLastEditTimestamp() const { return base.last_edit_timestamp; }

    std::string_view GetNickname() const { return base.nickname; }

    // Setters
    void SetNickname(const std::string_view nickname) {
        ASSERT(nickname.size() < NICKNAME_SIZE, Services,
               "Nickname size ({}) too big", nickname.size());
        std::memcpy(base.nickname, nickname.data(), nickname.size());
        NotifyEdit();
    }

  private:
    ProfileBase base;
    UserData data;

    // Helpers
    void NotifyEdit() { base.last_edit_timestamp = get_absolute_time(); }
};

} // namespace hydra::horizon::services::account
