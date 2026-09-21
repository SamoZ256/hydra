#include "core/horizon/services/hid/internal/resource_manager.hpp"

#include "core/system.hpp"

namespace hydra::horizon::services::hid::internal {

void ResourceManager::setupNpads() {
    for (auto& resource : resource_pool) {
        if (!resource.has_value())
            continue;

        resource->setupNpads();
    }
}

void ResourceManager::update() {
    // Npads
    for (u32 i = 0; i < NPAD_COUNT; i++) {
        const auto index = static_cast<NpadIndex>(i);

        // Poll
        const auto state = system.getInputDeviceManager().pollNpad(index);

        // Update
        for (auto& resource : resource_pool) {
            if (!resource.has_value())
                continue;

            resource->updateNpad(index, state);
        }
    }

    // Touch
    const auto touch_state = system.getInputDeviceManager().pollTouch();
    for (auto& resource : resource_pool) {
        if (!resource.has_value())
            continue;

        resource->updateTouch(touch_state);
    }
}

} // namespace hydra::horizon::services::hid::internal
