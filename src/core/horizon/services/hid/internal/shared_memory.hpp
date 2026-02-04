#pragma once

#include "core/horizon/services/hid/const.hpp"

namespace hydra::horizon::services::hid::internal {

template <typename T, usize max_entries = 17>
struct RingLifo {
  public:
    enum class Error {
        NoStorage,
    };

    void Clear() {
        atomic_store(&index, 0ull);
        atomic_store(&count, 0ull);
    }

    T& GetCurrentStorage() { return GetCurrentAtomicStorage().data; }

    void Write(const T& data) {
        const auto next_index = (ReadIndex() + 1) % max_entries;
        storages[next_index].Write(data);
        atomic_store(&index, next_index);

        // TODO: why?
        // TODO: should be max_entries - 1
        if (ReadCount() < 1) {
            atomic_fetch_add(&count, 1ull);
        }
    }

    void WriteNext(const T& data) {
        // HACK: const cast
        try {
            const_cast<T&>(data).sampling_number =
                GetCurrentStorage().sampling_number + 1;
        } catch (Error error) {
            const_cast<T&>(data).sampling_number = 0;
        }
        Write(data);
    }

  private:
    u64 unused;
    u64 buffer_count{max_entries};
    u64 index{0};
    u64 count{0};

    struct AtomicStorage {
        u64 sampling_number;
        T data;

        u64 ReadSamplingNumber() const { return atomic_load(&sampling_number); }

        void Write(const T& data_) {
            atomic_store(&sampling_number, data_.sampling_number);
            // TODO: thread barrier?
            data = data_;
        }
    };

    std::array<AtomicStorage, max_entries> storages;

    // Helpers
    u64 ReadIndex() { return atomic_load(&index); }
    u64 ReadCount() { return atomic_load(&count); }

    AtomicStorage& GetCurrentAtomicStorage() {
        const auto count = std::min(ReadCount(), 1ull); // TODO: why limit to 1?
        if (count == 0) {
            throw Error::NoStorage; // TODO: what to do?
        }

        auto index = ReadIndex();
        const auto storage_index =
            (index + 1 - count) % max_entries; // TODO: correct?
        auto& storage = storages[storage_index];
        // TODO: verify sampling numbers

        return storage;
    }
};

struct AnalogStickState {
    i32 x;
    i32 y;
};

struct Vector {
    f32 x;
    f32 y;
    f32 z;
};

struct DirectionState {
    float direction[3][3];
};

struct DebugPadState {
    u64 sampling_number;
    u32 attributes;
    u32 buttons;
    AnalogStickState analog_stick_r;
    AnalogStickState analog_stick_l;
};

struct DebugPadSharedMemoryFormat {
    RingLifo<DebugPadState> lifo;
    u8 padding[0x138];
};

struct TouchState {
    u64 delta_time;
    u32 attributes; // TODO: enum flags?
    u32 finger_id;
    u32 x;
    u32 y;
    u32 diameter_x;
    u32 diameter_y;
    u32 rotation_angle;
    u32 reserved;
};

struct TouchScreenState {
    u64 sampling_number;
    i32 count;
    u32 reserved;
    TouchState touches[16];
};

struct TouchScreenSharedMemoryFormat {
    RingLifo<TouchScreenState> lifo;
    u8 padding[0x3c8];
};

struct TouchScreenConfigurationForNx {
    u8 mode;
    u8 reserved[0xF];
};

struct MouseState {
    u64 sampling_number;
    i32 x;
    i32 y;
    i32 delta_x;
    i32 delta_y;
    i32 wheel_delta_x;
    i32 wheel_delta_y;
    u32 buttons;
    u32 attributes;
};

struct MouseSharedMemoryFormat {
    RingLifo<MouseState> lifo;
    u8 padding[0xB0];
};

struct KeyboardState {
    u64 sampling_number;
    u64 modifiers;
    u64 keys[4];
};

struct KeyboardSharedMemoryFormat {
    RingLifo<KeyboardState> lifo;
    u8 padding[0x28];
};

struct BasicXpadState {
    u64 sampling_number;
    u32 attributes;
    u32 buttons;
    u64 analog_stick_left;
    u64 analog_stick_right;
};

struct BasicXpadSharedMemoryEntry {
    RingLifo<BasicXpadState> lifo;
    u8 padding[0x138];
};

struct BasicXpadSharedMemoryFormat {
    BasicXpadSharedMemoryEntry entries[4];
};

struct DigitizerState {
    u64 sampling_number;
    u32 unk_0x8;
    u32 unk_0xC;
    u32 attributes;
    u32 buttons;
    u32 unk_0x18;
    u32 unk_0x1C;
    u32 unk_0x20;
    u32 unk_0x24;
    u32 unk_0x28;
    u32 unk_0x2C;
    u32 unk_0x30;
    u32 unk_0x34;
    u32 unk_0x38;
    u32 unk_0x3C;
    u32 unk_0x40;
    u32 unk_0x44;
    u32 unk_0x48;
    u32 unk_0x4C;
    u32 unk_0x50;
    u32 unk_0x54;
};

struct DigitizerSharedMemoryFormat {
    RingLifo<DigitizerState> lifo;
    u8 padding[0x980];
};

struct HomeButtonState {
    u64 sampling_number;
    u64 buttons;
};

struct HomeButtonSharedMemoryFormat {
    RingLifo<HomeButtonState> lifo;
    u8 padding[0x48];
};

struct SleepButtonState {
    u64 sampling_number;
    u64 buttons;
};

struct SleepButtonSharedMemoryFormat {
    RingLifo<SleepButtonState> lifo;
    u8 padding[0x48];
};

struct CaptureButtonState {
    u64 sampling_number;
    u64 buttons;
};

struct CaptureButtonSharedMemoryFormat {
    RingLifo<CaptureButtonState> lifo;
    u8 padding[0x48];
};

struct InputDetectorState {
    u64 input_source_state;
    u64 sampling_number;
};

struct InputDetectorSharedMemoryEntry {
    RingLifo<InputDetectorState, 2> lifo;
    u8 padding[0x30];
};

struct InputDetectorSharedMemoryFormat {
    InputDetectorSharedMemoryEntry entries[16];
};

struct UniquePadConfigMutex {
    u8 unk_0x0[0x20];
};

struct SixAxisSensorUserCalibrationState {
    u32 flags;
    u8 reserved[4];
    u64 stage;
    u64 sampling_number;
};

struct AnalogStickCalibrationStateImpl {
    u64 state;
    u64 flags;
    u64 stage;
    u64 sampling_number;
};

struct UniquePadConfig {
    u32 type;
    u32 interface;
    u8 serial_number[0x10];
    u32 controller_number;
    bool is_active;
    u8 reserved[3];
    u64 sampling_number;
};

struct UniquePadLifo {
    RingLifo<UniquePadConfig, 2> config_lifo;
    RingLifo<AnalogStickCalibrationStateImpl, 2> analog_stick_calib_lifo[2];
    RingLifo<SixAxisSensorUserCalibrationState, 2> sixaxis_calib_lifo;
    UniquePadConfigMutex mutex;
};

struct UniquePadSharedMemoryEntry {
    UniquePadLifo lifo;
    u8 padding[0x220];
};

struct UniquePadSharedMemoryFormat {
    UniquePadSharedMemoryEntry entries[16];
};

struct NpadControllerColor {
    NpadColor main;
    NpadColor sub;
};

struct NpadFullKeyColorState {
    u32 attribute;
    NpadControllerColor full_key;
};

struct NpadJoyColorState {
    u32 attribute;
    NpadControllerColor left;
    NpadControllerColor right;
};

struct NpadCommonState {
    u64 sampling_number;
    NpadButtons buttons;
    AnalogStickState analog_stick_l;
    AnalogStickState analog_stick_r;
    NpadAttributes attributes;
    u32 reserved;
};

struct NpadGcState {
    u64 sampling_number;
    u64 buttons;
    AnalogStickState analog_stick_l;
    AnalogStickState analog_stick_r;
    u32 attributes;
    u32 trigger_l;
    u32 trigger_r;
    u32 pad;
};

struct NpadLarkState {
    u64 sampling_number;
    u64 buttons;
    AnalogStickState analog_stick_l;
    AnalogStickState analog_stick_r;
    u32 attributes;
    NpadLarkType lark_type_l_and_main;
};

struct NpadHandheldLarkState {
    u64 sampling_number;
    u64 buttons;
    AnalogStickState analog_stick_l;
    AnalogStickState analog_stick_r;
    u32 attributes;
    NpadLarkType lark_type_l_and_main;
    NpadLarkType lark_type_r;
    u32 pad;
};

struct NpadLuciaState {
    u64 sampling_number;
    u64 buttons;
    AnalogStickState analog_stick_l;
    AnalogStickState analog_stick_r;
    u32 attributes;
    NpadLuciaType lucia_type;
};

struct NpadGcTriggerState {
    u64 sampling_number;
    u32 trigger_l;
    u32 trigger_r;
};

struct SixAxisSensorState {
    u64 delta_time;
    u64 sampling_number;
    Vector acceleration;
    Vector angular_velocity;
    Vector angle;
    DirectionState direction;
    u32 attributes;
    u32 reserved;
};

struct PowerInfo {
    bool is_powered;
    bool is_charging;
    u8 reserved[6];
    u32 battery_level;
};

struct XcdDeviceHandle {
    u64 handle;
};

struct NfcXcdDeviceHandleStateImpl {
    XcdDeviceHandle handle;
    u8 is_available;
    u8 is_activated;
    u8 reserved[6];
    u64 sampling_number;
};

struct NpadInternalState {
    NpadStyleSet style_set;
    NpadJoyAssignmentMode joy_assignment_mode;
    NpadFullKeyColorState full_key_color;
    NpadJoyColorState joy_color;
    RingLifo<NpadCommonState> full_key_lifo;
    RingLifo<NpadCommonState> handheld_lifo;
    RingLifo<NpadCommonState> joy_dual_lifo;
    RingLifo<NpadCommonState> joy_left_lifo;
    RingLifo<NpadCommonState> joy_right_lifo;
    RingLifo<NpadCommonState> palma_lifo;
    RingLifo<NpadCommonState> system_ext_lifo;
    RingLifo<SixAxisSensorState> full_key_six_axis_sensor_lifo;
    RingLifo<SixAxisSensorState> handheld_six_axis_sensor_lifo;
    RingLifo<SixAxisSensorState> joy_dual_left_six_axis_sensor_lifo;
    RingLifo<SixAxisSensorState> joy_dual_right_six_axis_sensor_lifo;
    RingLifo<SixAxisSensorState> joy_left_six_axis_sensor_lifo;
    RingLifo<SixAxisSensorState> joy_right_six_axis_sensor_lifo;
    DeviceTypeBits device_type;
    u32 reserved;
    NpadSystemProperties system_properties;
    NpadSystemButtonProperties system_button_properties;
    NpadBatteryLevel battery_level_joy_dual;
    NpadBatteryLevel battery_level_joy_left;
    NpadBatteryLevel battery_level_joy_right;
    union {
        struct {
            RingLifo<NfcXcdDeviceHandleStateImpl, 2> nfc_xcd_device_handle;
        };
        struct {
            u32 applet_footer_ui_attribute;
            AppletFooterUiType applet_footer_ui_type;
            u8 reserved_x41AD[0x5B];
        };
    };
    u8 reserved_x4208[0x20];
    RingLifo<NpadGcTriggerState> gc_trigger_lifo;
    NpadLarkType lark_type_l_and_main;
    NpadLarkType lark_type_r;
    NpadLuciaType lucia_type;
    NpadLagerType lager_type;
};

struct NpadSharedMemoryEntry {
    NpadInternalState internal_state;
    u8 pad[0xC10];
};

struct NpadSharedMemoryFormat {
    NpadSharedMemoryEntry entries[NPAD_COUNT];
};

struct GesturePoint {
    u32 x;
    u32 y;
};

struct GestureState {
    u64 sampling_number;
    u64 context_number;
    u32 type;
    u32 direction;
    u32 x;
    u32 y;
    i32 delta_x;
    i32 delta_y;
    f32 velocity_x;
    f32 velocity_y;
    u32 attributes;
    f32 scale;
    f32 rotation_angle;
    i32 point_count;
    GesturePoint points[4];
};

struct GestureSharedMemoryFormat {
    RingLifo<GestureState> lifo;
    u8 pad[0xF8];
};

struct ConsoleSixAxisSensor {
    u64 sampling_number;
    u8 is_seven_six_axis_sensor_at_rest;
    u8 pad[0x3];
    f32 verticalization_error;
    float3 gyro_bias;
    u8 pad2[0x4];
};

struct SharedMemory {
    DebugPadSharedMemoryFormat debug_pad;
    TouchScreenSharedMemoryFormat touch_screen;
    MouseSharedMemoryFormat mouse;
    KeyboardSharedMemoryFormat keyboard;
    union {
        BasicXpadSharedMemoryFormat basic_xpad;
        DigitizerSharedMemoryFormat digitizer;
    };
    HomeButtonSharedMemoryFormat home_button;
    SleepButtonSharedMemoryFormat sleep_button;
    CaptureButtonSharedMemoryFormat capture_button;
    InputDetectorSharedMemoryFormat input_detector;
    UniquePadSharedMemoryFormat unique_pad;
    NpadSharedMemoryFormat npad;
    GestureSharedMemoryFormat gesture;
    ConsoleSixAxisSensor console_six_axis_sensor;
    u8 unk_x3C220[0x3DE0];
};

} // namespace hydra::horizon::services::hid::internal
