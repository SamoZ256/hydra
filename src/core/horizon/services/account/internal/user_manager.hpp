#pragma once

#include "core/horizon/avatar_image_loader.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/services/account/internal/user.hpp"

namespace hydra::horizon::services::account::internal {

class UserManager {
  public:
    UserManager();

    uuid_t Create();

    void Flush();

    std::vector<uuid_t> GetUserIDs() const {
        std::vector<uuid_t> ids;
        ids.reserve(users.size());
        for (const auto& [id, _] : users)
            ids.push_back(id);

        return ids;
    }

    User& Get(uuid_t user_id) { return GetPair(user_id).first; }
    bool Exists(uuid_t user_id) const { return users.contains(user_id); }
    usize GetCount() const { return users.size(); }

    // Avatar
    void LoadSystemAvatars() {
        avatar_image_loader.LoadSystemAvatars(FILESYSTEM_INSTANCE);
    }
    void LoadAvatarImage(uuid_t user_id, std::vector<u8>& out_data);
    // TODO: isn't there a better way?
    usize GetAvatarImageSize(uuid_t user_id) {
        std::vector<u8> data;
        LoadAvatarImage(user_id, data);
        return data.size();
    }

  private:
    std::map<uuid_t, std::pair<User, u64>> users;
    AvatarImageLoader avatar_image_loader;

    // Helpers
    std::pair<User, u64>& GetPair(uuid_t user_id) {
        auto it = users.find(user_id);
        ASSERT(it != users.end(), Horizon, "Invalid user {:032x}", user_id);

        return it->second;
    }

    static std::string GetUserPath() {
        return fmt::format("{}/user", CONFIG_INSTANCE.GetAppDataPath());
    }

    void Serialize(uuid_t user_id);
    void Deserialize(uuid_t user_id);
};

} // namespace hydra::horizon::services::account::internal
