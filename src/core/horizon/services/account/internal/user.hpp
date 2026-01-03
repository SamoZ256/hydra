#pragma once

#include "core/horizon/services/account/const.hpp"

namespace hydra::horizon::services::account::internal {

constexpr uuid_t INVALID_USER_ID = 0x0;

class User {
    friend class UserManager;

  public:
    User(const ProfileBase& base_, const UserData& data_,
         uchar3 avatar_bg_color_, std::string_view avatar_path_)
        : base{base_}, data{data_}, avatar_bg_color{avatar_bg_color_},
          avatar_path{avatar_path_} {}
    User(const std::string_view nickname, uchar3 avatar_bg_color_,
         std::string_view avatar_path_)
        : avatar_bg_color{avatar_bg_color_}, avatar_path{avatar_path_} {
        // TODO: don't use the setters?
        SetNickname(nickname);
    }

    bool EditedSince(u64 timestamp) const {
        return base.last_edit_timestamp > timestamp;
    }

    u64 GetLastEditTimestamp() const { return base.last_edit_timestamp; }

    // Nickname
    std::string_view GetNickname() const { return base.nickname; }

    enum class SetNicknameError {
        SizeTooLarge,
    };

    void SetNickname(const std::string_view nickname) {
        ASSERT_THROWING(nickname.size() < NICKNAME_SIZE, Services,
                        SetNicknameError::SizeTooLarge,
                        "Nickname size ({}) too big", nickname.size());
        std::memcpy(base.nickname, nickname.data(), nickname.size());
        base.nickname[nickname.size()] = '\0';
        NotifyEdit();
    }

    // Avatar background color
    uchar3 GetAvatarBgColor() const { return avatar_bg_color; }

    void SetAvatarBgColor(uchar3 avatar_bg_color_) {
        avatar_bg_color = avatar_bg_color_;
        NotifyEdit();
    }

    // Avatar path
    std::string_view GetAvatarPath() const { return avatar_path; }

    void SetAvatarPath(std::string_view avatar_path_) {
        avatar_path = avatar_path_;
        NotifyEdit();
    }

  private:
    ProfileBase base;
    UserData data;
    uchar3 avatar_bg_color;
    std::string avatar_path;

    // Helpers
    void NotifyEdit() { base.last_edit_timestamp = get_absolute_time(); }

  public:
    CONST_REF_GETTER(base, GetBase);
    CONST_REF_GETTER(data, GetData);
};

} // namespace hydra::horizon::services::account::internal
