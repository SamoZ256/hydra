#include "core/horizon/services/hid/internal/applet_resource.hpp"

#include "core/horizon/kernel/shared_memory.hpp"
#include "core/system.hpp"

#define SHARED_MEMORY (*reinterpret_cast<SharedMemory*>(shared_mem->GetPtr()))

namespace hydra::horizon::services::hid::internal {

#define NPAD_INTERNAL_STATE(i) SHARED_MEMORY.npad.entries[i].internal_state

AppletResource::AppletResource(System& system)
    : shared_mem{new kernel::SharedMemory(system.GetCpu(), sizeof(SharedMemory),
                                          "HID shared memory")},
      npads{{NPAD_INTERNAL_STATE(0), NPAD_INTERNAL_STATE(1),
             NPAD_INTERNAL_STATE(2), NPAD_INTERNAL_STATE(3),
             NPAD_INTERNAL_STATE(4), NPAD_INTERNAL_STATE(5),
             NPAD_INTERNAL_STATE(6), NPAD_INTERNAL_STATE(7),
             NPAD_INTERNAL_STATE(8), NPAD_INTERNAL_STATE(9)}} {}

#undef NPAD_INTERNAL_STATE

AppletResource::~AppletResource() { delete shared_mem; }

void AppletResource::ActivateNpads(NpadRevision revision) {
    active = true;

    // TODO: revision affects style sets?
    LOG_NOT_IMPLEMENTED(Services, "{}", revision);

    SetupNpads();
}

void AppletResource::SetupNpads() {
    for (u32 i = 0; i < NPAD_COUNT; i++) {
        // TODO: get this from the config
        NpadStyleSet style_set;
        switch (i) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            style_set = NpadStyleSet::FullKey;
            break;
        case 8:
            style_set = NpadStyleSet::Handheld;
            break;
        case 9:
            style_set = NpadStyleSet::Palma;
            break;
        }

        // Disable handheld npads in docked mode
        if (style_set == NpadStyleSet::Handheld &&
            !CONFIG_INSTANCE.GetHandheldMode())
            style_set = NpadStyleSet::None;

        // TODO: auto-remap ProController to JoyDual and vice-versa

        // Check if supported
        if (supported_npads[i] && !any(supported_style_sets & style_set))
            style_set = NpadStyleSet::None;

        npads[i].Setup(style_set);
    }
}

void AppletResource::UpdateTouch(
    const std::map<u32, input::TouchState>& new_state) {
    if (!ShouldAcceptInput())
        return;

    auto& lifo = SHARED_MEMORY.touch_screen.lifo;

    // State
    TouchScreenState state{
        .count = static_cast<i32>(new_state.size()),
    };
    u32 index = 0;
    for (const auto& [finger_id, touch_state] : new_state) {
        state.touches[index++] = {
            .finger_id = finger_id, .x = touch_state.x, .y = touch_state.y,
            // TODO: more
        };
    }
    lifo.WriteNext(state);
}

} // namespace hydra::horizon::services::hid::internal
