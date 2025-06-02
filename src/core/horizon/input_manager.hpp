#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/hid.hpp"
#include "core/horizon/kernel/kernel.hpp"

namespace hydra::horizon {

namespace HID {
struct SharedMemory;
}

class InputManager {
  public:
    InputManager();

    // Device connection
    void ConnectNpad(hid::NpadIdType type, hid::NpadStyleSet style_set,
                     hid::NpadAttributes attributes);

    // Events

    // Npad
    void UpdateAndSetNpadButtons(hid::NpadIdType type,
                                 hid::NpadButtons buttons);
    void UpdateAndSetNpadAnalogStickStateL(hid::NpadIdType type,
                                           hid::AnalogStickState analog_stick);
    void UpdateAndSetNpadAnalogStickStateR(hid::NpadIdType type,
                                           hid::AnalogStickState analog_stick);

    // Touch
    void UpdateTouchStates();
    void SetTouchState(hid::TouchState state);
    u32 BeginTouch();
    void EndTouch(u32 finger_id);

    // Getters
    const handle_id_t GetSharedMemoryId() const { return shared_memory.id; }

  private:
    kernel::HandleWithId<kernel::SharedMemory> shared_memory;

    // State
    usize touch_count{0};
    u16 available_finger_mask{0xffff};

    // Helpers
    hid::SharedMemory* GetHidSharedMemory() const;
};

} // namespace hydra::horizon
