#pragma once

#include "core/horizon/kernel/applet_resource.hpp"
#include "core/horizon/services/hid/internal/npad.hpp"
#include "core/horizon/services/hid/internal/npad_index.hpp"

namespace hydra {
class System;
}

namespace hydra::horizon::kernel {
class SharedMemory;
}

namespace hydra::horizon::services::hid::internal {

class AppletResource {
  public:
    AppletResource(System& system);
    ~AppletResource();

    // Npad setup
    void ActivateNpads(NpadRevision revision);
    void SetupNpads();

    void DisconnectNpad(NpadIndex index) {
        npads[static_cast<usize>(index)].Setup(NpadStyleSet::None);
        // TODO: ensure that it doesn't get connected in the future?
    }

    // Npad support
    void ClearSupportedNpads() { supported_npads = {false}; }
    void SetNpadSupported(NpadIndex index, bool supported) {
        supported_npads[static_cast<usize>(index)] = supported;
        // TODO: reevaluate npad?
    }

    // Update
    void UpdateNpad(NpadIndex index, const input::NpadState& new_state) {
        if (!ShouldAcceptInput())
            return;

        npads[static_cast<usize>(index)].Update(new_state);
    }
    void UpdateTouch(const std::map<u32, input::TouchState>& new_state);

    kernel::Event* GetNpadStyleSetUpdateEvent(NpadIndex index) {
        return npads[static_cast<usize>(index)].GetStyleSetUpdateEvent();
    }

  private:
    kernel::SharedMemory* shared_mem;

    bool active{false};
    bool input_enabled{true};
    NpadStyleSet supported_style_sets{
        NpadStyleSet::Standard}; // TODO: what should this be?
    NpadJoyHoldType joy_hold_type{NpadJoyHoldType::Vertical};
    std::array<bool, NPAD_COUNT> supported_npads = {true};

    std::array<Npad, NPAD_COUNT> npads;

    // Helpers
    inline bool ShouldAcceptInput() const { return active && input_enabled; }

  public:
    GETTER(shared_mem, GetSharedMemory);
    SETTER(input_enabled, EnableInput);
    GETTER_AND_SETTER(supported_style_sets, GetSupportedStyleSet,
                      SetSupportedStyleSet);
    GETTER_AND_SETTER(joy_hold_type, GetJoyHoldType, SetJoyHoldType);
};

} // namespace hydra::horizon::services::hid::internal
