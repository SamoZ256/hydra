#pragma once

#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/services/account/internal/user.hpp"

namespace hydra::horizon::services::account::internal {

struct Avatar {
    filesystem::IFile* file;
    std::vector<uchar4> data{};
    u32 dimensions{0};
};

class UserManager {
  public:
    enum class Error {
        InvalidHusrMagic,
        InvalidHusrVersion,
        InvalidHusrHeaderSize,
    };

    UserManager();
    ~UserManager() { Flush(); }

    void Flush();

    uuid_t CreateUser();

    std::vector<uuid_t> GetUserIDs() const {
        std::vector<uuid_t> ids;
        ids.reserve(users.size());
        for (const auto& [id, _] : users)
            ids.push_back(id);

        return ids;
    }

    usize GetUserCount() const { return users.size(); }
    User& GetUser(uuid_t user_id) { return GetPair(user_id).first; }
    bool UserExists(uuid_t user_id) const { return users.contains(user_id); }

    // Avatar
    void LoadSystemAvatars(filesystem::Filesystem& fs);
    const std::vector<uchar4>& LoadAvatarImage(std::string_view path,
                                               u32& out_dimensions);
    void LoadAvatarImageAsJpeg(std::string_view path, uchar3 bg_color,
                               std::vector<u8>& out_data);

    std::string_view GetAvatarPath(u32 index) const {
        // TODO: not the best way to index into a map
        auto it = avatars.begin();
        std::advance(it, index);
        return it->first;
    }

  private:
    std::map<uuid_t, std::pair<User, u64>> users;
    std::map<std::string, Avatar> avatars;

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

    enum class PreloadAvatarError {
        LoadImageFailed,
        ImageNotASquare,
    };
    void PreloadAvatar(Avatar& avatar, bool is_compressed);

  public:
    CONST_REF_GETTER(avatars, GetAvatars);
};

} // namespace hydra::horizon::services::account::internal
