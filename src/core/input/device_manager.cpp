#include "core/input/device_manager.hpp"

#include "core/horizon/os.hpp"
#include "core/input/apple_gc/device_list.hpp"

namespace hydra::input {

namespace {

i32 analog_stick_to_int(f32 value) {
    if (value < 0.0f)
        return std::max(i16(value * (-std::numeric_limits<i16>::min())),
                        std::numeric_limits<i16>::min());
    else
        return std::min(i16(value * std::numeric_limits<i16>::max()),
                        std::numeric_limits<i16>::max());
}

} // namespace

DeviceManager::DeviceManager() {
    // Profiles
    for (u32 i = 0; i < NPAD_COUNT; i++) {
        const auto& name = CONFIG_INSTANCE.GetInputProfiles()[i];
        if (name.empty())
            continue;

        profiles[i] = Profile(static_cast<horizon::hid::NpadIdType>(i), name);
    }

    // Device list
    device_list = new apple_gc::DeviceList();
}

DeviceManager::~DeviceManager() { delete device_list; }

void DeviceManager::ConnectNpads() {
    for (u32 i = 0; i < NPAD_COUNT; i++) {
        if (!profiles[i])
            continue;

        const auto type = horizon::hid::NpadIdType(i);
        INPUT_MANAGER_INSTANCE.ConnectNpad(
            type, horizon::hid::NpadStyleSet::Standard,
            horizon::hid::NpadAttributes::IsConnected);
    }
}

void DeviceManager::Poll() {
    // Npads
    for (u32 i = 0; i < NPAD_COUNT; i++) {
        const auto type = horizon::hid::NpadIdType(i);
        PollNpad(type, i);
    }

    // Touch screen
    {
        INPUT_MANAGER_INSTANCE.UpdateTouchStates();

        // TODO: get name from the config
        const std::string device_name = "cursor";

        auto device = GetDevice(device_name);
        if (!device)
            return;

        // Process touches
        u64 touch_id;
        while ((touch_id = device->GetNextBeganTouchID()) != invalid<u64>()) {
            active_touches.insert(
                {touch_id, INPUT_MANAGER_INSTANCE.BeginTouch()});
        }

        while ((touch_id = device->GetNextEndedTouchID()) != invalid<u64>()) {
            auto it = active_touches.find(touch_id);
            ASSERT(it != active_touches.end(), Input,
                   "Touch 0x{:016x} not active", touch_id);
            INPUT_MANAGER_INSTANCE.EndTouch(it->second);
            active_touches.erase(it);
        }

        // Set touch state
        for (const auto [touch_id, finger_id] : active_touches) {
            ASSERT_DEBUG(finger_id != invalid<u32>(), Input,
                         "Invalid finger ID");

            i32 x, y;
            device->GetTouchPosition(touch_id, x, y);
            // TODO: also clamp to guest screen size
            x = std::max(x, 0);
            y = std::max(y, 0);

            INPUT_MANAGER_INSTANCE.SetTouchState({
                .finger_id = finger_id,
                .x = static_cast<u32>(x),
                .y = static_cast<u32>(y),
                // TODO: other stuff
            });
        }
    }
}

void DeviceManager::PollNpad(horizon::hid::NpadIdType type, u32 index) {
    const auto& profile_opt = profiles[index];
    if (!profile_opt)
        return;

    const auto& profile = *profile_opt;

    horizon::hid::NpadButtons buttons = horizon::hid::NpadButtons::None;
    f32 analog_l_x = 0.0f;
    f32 analog_l_y = 0.0f;
    f32 analog_r_x = 0.0f;
    f32 analog_r_y = 0.0f;

    for (const auto& device_name : profile.GetDeviceNames()) {
        auto device = GetDevice(device_name);
        if (!device)
            continue;

        // Buttons
        for (const auto& mapping : profile.GetButtonMappings()) {
            if (device->IsPressed(mapping.code))
                buttons |= mapping.npad_buttons;
        }

        // Analog sticks
        for (const auto& mapping : profile.GetAnalogMappings()) {
            const auto value = device->GetAxisValue(mapping.code);
            // TODO: there are also dedicated buttons for this
            if (mapping.axis.is_left) {
                switch (mapping.axis.direction) {
                case AnalogStickDirection::Right:
                    analog_l_x += value;
                    break;
                case AnalogStickDirection::Left:
                    analog_l_x -= value;
                    break;
                case AnalogStickDirection::Up:
                    analog_l_y += value;
                    break;
                case AnalogStickDirection::Down:
                    analog_l_y -= value;
                    break;
                }
            } else {
                switch (mapping.axis.direction) {
                case AnalogStickDirection::Right:
                    analog_r_x += value;
                    break;
                case AnalogStickDirection::Left:
                    analog_r_x -= value;
                    break;
                case AnalogStickDirection::Up:
                    analog_r_y += value;
                    break;
                case AnalogStickDirection::Down:
                    analog_r_y -= value;
                    break;
                }
            }
        }
    }

    // Update
    INPUT_MANAGER_INSTANCE.UpdateNpad(type);

    // Set
    INPUT_MANAGER_INSTANCE.SetNpadButtons(type, buttons);
    INPUT_MANAGER_INSTANCE.SetNpadAnalogStickStateL(
        type,
        {analog_stick_to_int(analog_l_x), analog_stick_to_int(analog_l_y)});
    INPUT_MANAGER_INSTANCE.SetNpadAnalogStickStateR(
        type,
        {analog_stick_to_int(analog_r_x), analog_stick_to_int(analog_r_y)});
}

} // namespace hydra::input
