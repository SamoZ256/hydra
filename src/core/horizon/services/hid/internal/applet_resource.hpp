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
    explicit AppletResource(System& system);
    ~AppletResource();

    // Npad setup
    void activateNpads(NpadRevision revision);
    void setupNpads();

    void disconnectNpad(NpadIndex index) {
        npads[static_cast<usize>(index)].setup(NpadStyleSet::None);
        // TODO: ensure that it doesn't get connected in the future?
    }

    // Npad support
    void clearSupportedNpads() { supported_npads = {false}; }
    void setNpadSupported(NpadIndex index, bool supported) {
        supported_npads[static_cast<usize>(index)] = supported;
        // TODO: reevaluate npad?
    }

    // Update
    void updateNpad(NpadIndex index, const input::NpadState& new_state) {
        if (!shouldAcceptInput())
            return;

        npads[static_cast<usize>(index)].update(new_state);
    }
    void updateTouch(const std::map<u32, input::TouchState>& new_state);

    kernel::Event* getNpadStyleSetUpdateEvent(NpadIndex index) {
        return npads[static_cast<usize>(index)].getStyleSetUpdateEvent();
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
    bool shouldAcceptInput() const { return active && input_enabled; }

  public:
    GETTER(shared_mem, getSharedMemory);
    SETTER(input_enabled, enableInput);
    GETTER_AND_SETTER(supported_style_sets, getSupportedStyleSet,
                      setSupportedStyleSet);
    GETTER_AND_SETTER(joy_hold_type, getJoyHoldType, setJoyHoldType);
};

} // namespace hydra::horizon::services::hid::internal
