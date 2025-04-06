#include "horizon/input_manager.hpp"

#include "horizon/hid.hpp"
#include "horizon/kernel.hpp"

#define GET_LIFO(state) auto& lifo = GetHidSharedMemory()->state##_lifo;

#define GET_NPAD_LIFO(type, style_lower)                                       \
    GET_LIFO(npad.entries[u32(type)].internal_state.style_lower)

#define SET_NPAD_ENTRY(type, style_upper, style_lower, entry_dst, entry_src)   \
    if (any(GetHidSharedMemory()                                               \
                ->npad.entries[u32(type)]                                      \
                .internal_state.style_set &                                    \
            HID::NpadStyleSet::style_upper)) {                                 \
        GET_NPAD_LIFO(type, style_lower);                                      \
        /* TODO: set the tail properly */                                      \
        lifo.header.tail = 0; /*(lifo.header.tail + 1) % 17;*/                 \
        lifo.storage[lifo.header.tail].state.entry_dst = entry_src;            \
        lifo.header.count = 1;                                                 \
    }

#define SET_NPAD_ENTRIES_SEPARATE(type, entry_dst, entry_src)                  \
    SET_NPAD_ENTRY(type, FullKey, full_key, entry_dst, entry_src);             \
    SET_NPAD_ENTRY(type, Handheld, handheld, entry_dst, entry_src);            \
    SET_NPAD_ENTRY(type, JoyDual, joy_dual, entry_dst, entry_src);             \
    SET_NPAD_ENTRY(type, JoyLeft, joy_left, entry_dst, entry_src);             \
    SET_NPAD_ENTRY(type, JoyRight, joy_right, entry_dst, entry_src);

#define SET_NPAD_ENTRIES(type, entry)                                          \
    SET_NPAD_ENTRIES_SEPARATE(type, entry, entry)

namespace Hydra::Horizon {

InputManager::InputManager() {
    shared_memory_id = Kernel::GetInstance().CreateSharedMemory(0x40000);
}

void InputManager::ConnectNpad(HID::NpadIdType type,
                               HID::NpadStyleSet style_set,
                               HID::NpadAttributes attributes) {
    GetHidSharedMemory()->npad.entries[u32(type)].internal_state.style_set =
        style_set;
    SET_NPAD_ENTRIES(type, attributes);
}

void InputManager::SetNpadButtons(HID::NpadIdType type,
                                  HID::NpadButtons buttons) {
    SET_NPAD_ENTRIES(type, buttons);
}

void InputManager::SetNpadAnalogStickStateL(
    HID::NpadIdType type, HID::AnalogStickState analog_stick) {
    SET_NPAD_ENTRIES_SEPARATE(type, analog_stick_l, analog_stick);
}

void InputManager::SetNpadAnalogStickStateR(
    HID::NpadIdType type, HID::AnalogStickState analog_stick) {
    SET_NPAD_ENTRIES_SEPARATE(type, analog_stick_r, analog_stick);
}

HID::SharedMemory* InputManager::GetHidSharedMemory() const {
    return reinterpret_cast<HID::SharedMemory*>(
        Kernel::GetInstance().GetSharedMemory(shared_memory_id).GetPtr());
}

} // namespace Hydra::Horizon
