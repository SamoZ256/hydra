#include "core/horizon/services/account/user_manager.hpp"

#define DEFAULT_USER_NAME "Hydra user"

namespace hydra::horizon::services::account {

namespace {

constexpr magic4_t HUSR_MAGIC = make_magic4('H', 'U', 'S', 'R');
constexpr u32 CURRENT_HUSR_VERSION = 1;

struct HusrHeader {
    magic4_t magic{HUSR_MAGIC};
    u32 version{CURRENT_HUSR_VERSION};
    u32 header_size{sizeof(HusrHeader)};
    User user;
};

} // namespace

SINGLETON_DEFINE_GET_INSTANCE(UserManager, Horizon)

UserManager::UserManager() {
    SINGLETON_SET_INSTANCE(UserManager, Horizon);

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

UserManager::~UserManager() { SINGLETON_UNSET_INSTANCE(); }

uuid_t UserManager::Create() {
    // First, find an available ID
    uuid_t user_id = random128();
    while (user_id == 0x0 || users.contains(user_id))
        user_id = random128();

    // Create
    User new_user(DEFAULT_USER_NAME);
    users.insert({user_id, {new_user, 0}});

    return user_id;
}

void UserManager::Flush() {
    for (auto& [user_id, user_pair] : users)
        Serialize(user_id);
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
        HusrHeader header{.user = user};
        writer.Write(header);
    }
    ofs.close();

    user_pair.second = get_absolute_time();
}

void UserManager::Deserialize(uuid_t user_id) {
    LOG_INFO(Horizon, "Deserializing user with ID {:032x}", user_id);

    {
        auto it = users.find(user_id);
        if (it != users.end()) {
            auto& user_pair = it->second;
            if (!user_pair.first.EditedSince(user_pair.second))
                LOG_WARN(Horizon, "Overwriting user {:032x}", user_id);
        }
    }

    std::string nickname;

    // Deserialize
    std::ifstream ifs{fmt::format("{}/{:032x}.husr", GetUserPath(), user_id)};
    {
        StreamReader reader(ifs);
        const auto header = reader.Read<HusrHeader>();

        // Validate
        ASSERT(header.magic == HUSR_MAGIC, Horizon,
               "Invalid HUSR magic 0x{:08x}", header.magic);
        ASSERT(header.version == CURRENT_HUSR_VERSION, Horizon,
               "Invalid HUSR version {}", header.version);
        ASSERT(header.header_size == sizeof(HusrHeader), Horizon,
               "Invalid HUSR header size 0x{:x}", header.header_size);

        users.insert(
            {user_id, {header.user, header.user.GetLastEditTimestamp()}});
    }
    ifs.close();
}

} // namespace hydra::horizon::services::account
