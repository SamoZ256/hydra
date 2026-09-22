#pragma once

#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/services/account/internal/user.hpp"

namespace hydra::horizon::services::account::internal {

struct Avatar {
    filesystem::IFile* file;
    std::vector<uchar4> data;
    u32 dimensions{0};
};

class UserManager {
  public:
    UserManager();
    ~UserManager() { flush(); }

    void flush();

    uuid_t createUser();

    std::vector<uuid_t> getUserIDs() const {
        std::vector<uuid_t> ids;
        ids.reserve(users.size());
        for (const auto& [id, _] : users)
            ids.push_back(id);

        return ids;
    }

    usize getUserCount() const { return users.size(); }
    User& getUser(uuid_t user_id) { return getPair(user_id).first; }
    bool userExists(uuid_t user_id) const { return users.contains(user_id); }

    // Avatar
    void loadSystemAvatars(filesystem::Filesystem& fs);
    const std::vector<uchar4>& loadAvatarImage(std::string_view path,
                                               u32& out_dimensions);
    void loadAvatarImageAsJpeg(std::string_view path, uchar3 bg_color,
                               std::vector<u8>& out_data);

    std::string_view getAvatarPath(u32 index) const {
        // TODO: not the best way to index into a map
        auto it = avatars.begin();
        std::advance(it, index);
        return it->first;
    }

  private:
    std::map<uuid_t, std::pair<User, u64>> users;
    std::map<std::string, Avatar> avatars;

    // Helpers
    std::pair<User, u64>& getPair(uuid_t user_id) {
        auto it = users.find(user_id);
        ASSERT(it != users.end(), Horizon, "Invalid user {:032x}", user_id);

        return it->second;
    }

    static std::string getUsersPath() {
        return fmt::format("{}/user", CONFIG_INSTANCE.getAppDataPath());
    }

    static std::string getUserPath(uuid_t user_id) {
        return fmt::format("{}/{:032x}.husr", getUsersPath(), user_id);
    }

    void serialize(uuid_t user_id);
    void deserialize(uuid_t user_id);

    enum class PreloadAvatarError {
        LoadImageFailed,
        ImageNotASquare,
    };
    static void preloadAvatar(Avatar& avatar, bool is_compressed);

  public:
    CONST_REF_GETTER(avatars, getAvatars);
};

} // namespace hydra::horizon::services::account::internal
