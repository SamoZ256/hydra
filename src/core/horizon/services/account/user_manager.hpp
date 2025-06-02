#pragma once

#include "core/horizon/services/account/const.hpp"

#define USER_MANAGER_INSTANCE                                                  \
    horizon::services::account::UserManager::GetInstance()

namespace hydra::horizon::services::account {

class UserManager {
  public:
    static UserManager& GetInstance();

    UserManager();
    ~UserManager();

    uuid_t Create();

    void Flush();

    const uuid_t* Begin() { return &users.begin()->first; }
    const uuid_t* End() { return &users.end()->first; }

    User& Get(uuid_t user_id) { return GetPair(user_id).first; }
    bool Exists(uuid_t user_id) const { return users.contains(user_id); }
    usize GetCount() const { return users.size(); }

  private:
    std::map<uuid_t, std::pair<User, u64>> users;

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

} // namespace hydra::horizon::services::account
