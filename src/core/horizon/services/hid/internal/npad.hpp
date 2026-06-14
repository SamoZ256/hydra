#pragma once

#include "core/horizon/services/hid/const.hpp"
#include "core/horizon/services/hid/internal/shared_memory.hpp"
#include "core/input/state.hpp"

namespace hydra::horizon::kernel {
class Event;
}

namespace hydra::horizon::services::hid::internal {

class Npad {
  public:
    Npad(NpadInternalState& state_);
    ~Npad();

    void Setup(NpadStyleSet style_set);
    void Update(const input::NpadState& new_state);

  private:
    NpadInternalState& state;

    kernel::Event* style_set_update_event;

    // Helpers
    RingLifo<NpadCommonState>& GetLifo() {
        switch (state.style_set) {
        case NpadStyleSet::FullKey:
            return state.full_key_lifo;
        case NpadStyleSet::Handheld:
            return state.handheld_lifo;
        case NpadStyleSet::JoyDual:
            return state.joy_dual_lifo;
        case NpadStyleSet::JoyLeft:
            return state.joy_left_lifo;
        case NpadStyleSet::JoyRight:
            return state.joy_right_lifo;
        case NpadStyleSet::Palma:
            return state.palma_lifo;
        case NpadStyleSet::SystemExt: // TODO: correct?
            return state.system_ext_lifo;
        default:
            unreachable();
        }
    }

  public:
    GETTER(style_set_update_event, GetStyleSetUpdateEvent);
};

} // namespace hydra::horizon::services::hid::internal
