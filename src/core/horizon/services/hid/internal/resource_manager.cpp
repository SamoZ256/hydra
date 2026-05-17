#include "core/horizon/services/hid/internal/resource_manager.hpp"

#include "core/system.hpp"

namespace hydra::horizon::services::hid::internal {

void ResourceManager::SetupNpads() {
    for (auto& resource : resource_pool) {
        if (!resource.has_value())
            continue;

        resource->SetupNpads();
    }
}

void ResourceManager::Update() {
    // Npads
    for (u32 i = 0; i < NPAD_COUNT; i++) {
        const auto index = static_cast<NpadIndex>(i);

        // Poll
        const auto state = system.GetInputDeviceManager().PollNpad(index);

        // Update
        for (auto& resource : resource_pool) {
            if (!resource.has_value())
                continue;

            resource->UpdateNpad(index, state);
        }
    }

    // Touch
    const auto touch_state = system.GetInputDeviceManager().PollTouch();
    for (auto& resource : resource_pool) {
        if (!resource.has_value())
            continue;

        resource->UpdateTouch(touch_state);
    }
}

} // namespace hydra::horizon::services::hid::internal
