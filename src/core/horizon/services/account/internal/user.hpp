#pragma once

#include "core/horizon/services/account/const.hpp"

namespace hydra::horizon::services::account::internal {

constexpr uuid_t INVALID_USER_ID = 0x0;

inline u64 getTimestamp() {
    return static_cast<u64>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
}

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
        setNickname(nickname);
    }

    bool editedSince(u64 timestamp) const {
        return base.last_edit_timestamp > timestamp;
    }

    u64 getLastEditTimestamp() const { return base.last_edit_timestamp; }

    // Nickname
    std::string_view getNickname() const { return base.nickname; }

    enum class SetNicknameError {
        SizeTooLarge,
    };

    void setNickname(const std::string_view nickname) {
        ASSERT(nickname.size() < NICKNAME_SIZE, Services,
               "Nickname size ({}) too big", nickname.size());
        std::memcpy(base.nickname, nickname.data(), nickname.size());
        base.nickname[nickname.size()] = '\0';
        notifyEdit();
    }

    // Avatar background color
    uchar3 getAvatarBgColor() const { return avatar_bg_color; }

    void setAvatarBgColor(uchar3 avatar_bg_color_) {
        avatar_bg_color = avatar_bg_color_;
        notifyEdit();
    }

    // Avatar path
    std::string_view getAvatarPath() const { return avatar_path; }

    void setAvatarPath(std::string_view avatar_path_) {
        avatar_path = avatar_path_;
        notifyEdit();
    }

  private:
    ProfileBase base;
    UserData data;
    uchar3 avatar_bg_color;
    std::string avatar_path;

    // Helpers
    void notifyEdit() { base.last_edit_timestamp = getTimestamp(); }

  public:
    CONST_REF_GETTER(base, getBase);
    CONST_REF_GETTER(data, getData);
};

} // namespace hydra::horizon::services::account::internal
