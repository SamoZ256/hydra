#pragma once

#include "core/horizon/services/hid/internal/applet_resource.hpp"

namespace hydra::horizon::services::hid::internal {

class ResourceManager {
  public:
    enum class Error {
        InvalidAruid,
        AruidAlreadyTaken,
    };

    ResourceManager(System& system_) : system{system_}, resource_pool(system) {}

    void SetupNpads();
    void Update();

    AppletResource& CreateResource(kernel::AppletResourceUserId aruid) {
        return resource_pool.CreateResource(aruid);
    }

    void DestroyResource(kernel::AppletResourceUserId aruid) {
        resource_pool.DestroyResource(aruid);
    }

    AppletResource& GetResource(kernel::AppletResourceUserId aruid) {
        return resource_pool.GetResource(aruid);
    }

  private:
    System& system;

    kernel::AppletResourcePool<AppletResource> resource_pool;
};

} // namespace hydra::horizon::services::hid::internal
