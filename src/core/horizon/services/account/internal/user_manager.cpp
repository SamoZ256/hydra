#include "core/horizon/services/account/internal/user_manager.hpp"

#include <stb_image_write.h>

#define DEFAULT_USER_NAME "Hydra user"

namespace hydra::horizon::services::account::internal {

namespace {

constexpr magic4_t HUSR_MAGIC = make_magic4('H', 'U', 'S', 'R');
constexpr u32 CURRENT_HUSR_VERSION = 2;

struct HusrHeader {
    magic4_t magic{HUSR_MAGIC};
    u32 version{CURRENT_HUSR_VERSION};
    u32 header_size{sizeof(HusrHeader)};
};

constexpr u32 AVATAR_IMAGE_DIMENSION = 256;

static void jpg_to_memory(void* context, void* data, int len) {
    std::vector<u8>* jpg_image = static_cast<std::vector<u8>*>(context);
    u8* jpg = static_cast<u8*>(data);
    jpg_image->insert(jpg_image->end(), jpg, jpg + len);
}

} // namespace

UserManager::UserManager() {
    // Create user directory
    std::filesystem::create_directories(GetUserPath());

    // Deserialize all users
    if (std::filesystem::exists(GetUserPath())) {
        for (const auto& dir_entry :
             std::filesystem::directory_iterator{GetUserPath()}) {
            auto filename = dir_entry.path().filename();

            auto extension = filename.extension().string();
            if (extension != ".husr")
                continue;

            auto user_id_str = filename.stem().string();
            if (user_id_str.size() != 32) {
                LOG_WARN(Horizon, "Invalid user ID {}", user_id_str);
                return;
            }

            u64 user_id_hi =
                std::stoull(user_id_str.substr(0, 16), nullptr, 16);
            u64 user_id_lo =
                std::stoull(user_id_str.substr(16, 16), nullptr, 16);
            uuid_t user_id = (uuid_t)user_id_lo | ((uuid_t)user_id_hi << 64);

            Deserialize(user_id);
        }
    }

    if (users.empty()) {
        auto user_id = Create();
        Serialize(user_id);
    }
}

uuid_t UserManager::Create() {
    // First, find an available ID
    uuid_t user_id = random128();
    while (user_id == 0x0 || users.contains(user_id))
        user_id = random128();

    // Create
    User new_user(DEFAULT_USER_NAME, uchar3({18, 53, 110}), "00000001.szs");
    users.insert({user_id, {new_user, 0}});

    return user_id;
}

void UserManager::Flush() {
    for (auto& [user_id, user_pair] : users)
        Serialize(user_id);
}

void UserManager::LoadAvatarImage(uuid_t user_id, std::vector<u8>& out_data) {
    // Load image
    const auto& user = Get(user_id);

    std::vector<u8> decompressed;
    avatar_image_loader.LoadAvatarImage(user.avatar_path, decompressed);

    // Alpha blend with background color
    for (u32 i = 0; i < decompressed.size(); i += 4) {
        auto& r = decompressed[i + 0];
        auto& g = decompressed[i + 1];
        auto& b = decompressed[i + 2];
        auto& a = decompressed[i + 3];

        r = (user.avatar_bg_color.x() * (0xff - a) + r * a) / 0xff;
        g = (user.avatar_bg_color.y() * (0xff - a) + g * a) / 0xff;
        b = (user.avatar_bg_color.z() * (0xff - a) + b * a) / 0xff;
        a = 0xff;
    }

    // Convert to JPEG
    out_data.reserve(0x20000);
    stbi_write_jpg_to_func(jpg_to_memory, &out_data, AVATAR_IMAGE_DIMENSION,
                           AVATAR_IMAGE_DIMENSION, 4, decompressed.data(), 80);
    out_data.shrink_to_fit();
}

void UserManager::Serialize(uuid_t user_id) {
    LOG_INFO(Horizon, "Serializing user with ID {:032x}", user_id);

    auto& user_pair = GetPair(user_id);
    const auto& user = user_pair.first;
    if (!user.EditedSince(user_pair.second))
        return;

    // Serialize
    std::ofstream ofs{fmt::format("{}/{:032x}.husr", GetUserPath(), user_id)};
    {
        StreamWriter writer(ofs);

        // Header
        HusrHeader header{};
        writer.Write(header);

        // Data
        writer.Write(user.base);
        writer.Write(user.data);
        writer.Write(user.avatar_bg_color);
        writer.Write<u32>(user.avatar_path.size());
        writer.WritePtr(user.avatar_path.data(), user.avatar_path.size());
    }
    ofs.close();

    user_pair.second = get_absolute_time();
}

void UserManager::Deserialize(uuid_t user_id) {
    LOG_INFO(Horizon, "Deserializing user with ID {:032x}", user_id);

    auto it = users.find(user_id);
    if (it != users.end()) {
        auto& user_pair = it->second;
        if (!user_pair.first.EditedSince(user_pair.second))
            LOG_WARN(Horizon, "Overwriting user {:032x}", user_id);
    }

    // Deserialize
    std::ifstream ifs{fmt::format("{}/{:032x}.husr", GetUserPath(), user_id)};
    StreamReader reader(ifs);

    // Header
    const auto header = reader.Read<HusrHeader>();

    // Validate
    ASSERT(header.magic == HUSR_MAGIC, Horizon,
           "Invalid HUSR magic 0x{:08x} for user {:032x}", header.magic,
           user_id);
    if (header.version < 2) {
        LOG_WARN(Horizon, "Unsupported HUSR version {} for user {:032x}",
                 header.version, user_id);
        return;
    }
    ASSERT(header.version == CURRENT_HUSR_VERSION, Horizon,
           "Invalid HUSR version {} for user {:032x}", header.version, user_id);
    ASSERT(header.header_size == sizeof(HusrHeader), Horizon,
           "Invalid HUSR header size 0x{:x} for user {:032x}",
           header.header_size, user_id);

    // Data
    const auto base = reader.Read<ProfileBase>();
    const auto data = reader.Read<UserData>();
    const auto avatar_bg_color = reader.Read<uchar3>();
    std::string avatar_path;
    {
        const auto size = reader.Read<u32>();
        avatar_path.resize(size);
        reader.ReadPtr(avatar_path.data(), size);
    }

    User user(base, data, avatar_bg_color, avatar_path);
    users.insert({user_id, {user, user.GetLastEditTimestamp()}});
}

} // namespace hydra::horizon::services::account::internal
