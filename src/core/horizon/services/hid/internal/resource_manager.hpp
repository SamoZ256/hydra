#pragma once

#include "core/horizon/services/hid/internal/applet_resource.hpp"

namespace hydra::horizon::services::hid::internal {

class ResourceManager {
  public:
    enum class Error {
        InvalidAruid,
        AruidAlreadyTaken,
    };

    explicit ResourceManager(System& system_)
        : system{system_}, resource_pool(system) {}

    void setupNpads();
    void update();

    AppletResource& createResource(kernel::AppletResourceUserId aruid) {
        return resource_pool.createResource(aruid);
    }

    void destroyResource(kernel::AppletResourceUserId aruid) {
        resource_pool.destroyResource(aruid);
    }

    AppletResource& getResource(kernel::AppletResourceUserId aruid) {
        return resource_pool.getResource(aruid);
    }

  private:
    System& system;

    kernel::AppletResourcePool<AppletResource> resource_pool;
};

} // namespace hydra::horizon::services::hid::internal
