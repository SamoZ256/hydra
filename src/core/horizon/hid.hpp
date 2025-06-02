#pragma once

namespace hydra::horizon::hid {

enum class DebugPadButton {
    A = BIT(0),
    B = BIT(1),
    X = BIT(2),
    Y = BIT(3),
    L = BIT(4),
    R = BIT(5),
    ZL = BIT(6),
    ZR = BIT(7),
    Start = BIT(8),
    Select = BIT(9),
    Left = BIT(10),
    Up = BIT(11),
    Right = BIT(12),
    Down = BIT(13),
};

enum class TouchScreenModeForNx {
    UseSystemSetting = 0,
    Finger = 1,
    Heat2 = 2,
};

enum class MouseButton {
    Left = BIT(0),
    Right = BIT(1),
    Middle = BIT(2),
    Forward = BIT(3),
    Back = BIT(4),
};

enum class KeyboardKey {
    A = 4,
    B = 5,
    C = 6,
    D = 7,
    E = 8,
    F = 9,
    G = 10,
    H = 11,
    I = 12,
    J = 13,
    K = 14,
    L = 15,
    M = 16,
    N = 17,
    O = 18,
    P = 19,
    Q = 20,
    R = 21,
    S = 22,
    T = 23,
    U = 24,
    V = 25,
    W = 26,
    X = 27,
    Y = 28,
    Z = 29,
    D1 = 30,
    D2 = 31,
    D3 = 32,
    D4 = 33,
    D5 = 34,
    D6 = 35,
    D7 = 36,
    D8 = 37,
    D9 = 38,
    D0 = 39,
    Return = 40,
    Escape = 41,
    Backspace = 42,
    Tab = 43,
    Space = 44,
    Minus = 45,
    Plus = 46,
    OpenBracket = 47,
    CloseBracket = 48,
    Pipe = 49,
    Tilde = 50,
    Semicolon = 51,
    Quote = 52,
    Backquote = 53,
    Comma = 54,
    Period = 55,
    Slash = 56,
    CapsLock = 57,
    F1 = 58,
    F2 = 59,
    F3 = 60,
    F4 = 61,
    F5 = 62,
    F6 = 63,
    F7 = 64,
    F8 = 65,
    F9 = 66,
    F10 = 67,
    F11 = 68,
    F12 = 69,
    PrintScreen = 70,
    ScrollLock = 71,
    Pause = 72,
    Insert = 73,
    Home = 74,
    PageUp = 75,
    Delete = 76,
    End = 77,
    PageDown = 78,
    RightArrow = 79,
    LeftArrow = 80,
    DownArrow = 81,
    UpArrow = 82,
    NumLock = 83,
    NumPadDivide = 84,
    NumPadMultiply = 85,
    NumPadSubtract = 86,
    NumPadAdd = 87,
    NumPadEnter = 88,
    NumPad1 = 89,
    NumPad2 = 90,
    NumPad3 = 91,
    NumPad4 = 92,
    NumPad5 = 93,
    NumPad6 = 94,
    NumPad7 = 95,
    NumPad8 = 96,
    NumPad9 = 97,
    NumPad0 = 98,
    NumPadDot = 99,
    Backslash = 100,
    Application = 101,
    Power = 102,
    NumPadEquals = 103,
    F13 = 104,
    F14 = 105,
    F15 = 106,
    F16 = 107,
    F17 = 108,
    F18 = 109,
    F19 = 110,
    F20 = 111,
    F21 = 112,
    F22 = 113,
    F23 = 114,
    F24 = 115,
    NumPadComma = 133,
    Ro = 135,
    KatakanaHiragana = 136,
    Yen = 137,
    Henkan = 138,
    Muhenkan = 139,
    NumPadCommaPc98 = 140,
    HangulEnglish = 144,
    Hanja = 145,
    Katakana = 146,
    Hiragana = 147,
    ZenkakuHankaku = 148,
    LeftControl = 224,
    LeftShift = 225,
    LeftAlt = 226,
    LeftGui = 227,
    RightControl = 228,
    RightShift = 229,
    RightAlt = 230,
    RightGui = 231,
};

enum class KeyboardModifier {
    Control = BIT(0),
    Shift = BIT(1),
    LeftAlt = BIT(2),
    RightAlt = BIT(3),
    Gui = BIT(4),
    CapsLock = BIT(8),
    ScrollLock = BIT(9),
    NumLock = BIT(10),
    Katakana = BIT(11),
    Hiragana = BIT(12),
};

enum class KeyboardLockKeyEvent {
    NumLockOn = BIT(0),
    NumLockOff = BIT(1),
    NumLockToggle = BIT(2),
    CapsLockOn = BIT(3),
    CapsLockOff = BIT(4),
    CapsLockToggle = BIT(5),
    ScrollLockOn = BIT(6),
    ScrollLockOff = BIT(7),
    ScrollLockToggle = BIT(8),
};

enum class NpadIdType : u8 {
    No1,
    No2,
    No3,
    No4,
    No5,
    No6,
    No7,
    No8,
    Handheld,
    Other,
};

enum class NpadStyleSet : u32 {
    None = 0,
    FullKey = BIT(0),
    Handheld = BIT(1),
    JoyDual = BIT(2),
    JoyLeft = BIT(3),
    JoyRight = BIT(4),
    Gc = BIT(5),
    Palma = BIT(6),
    Lark = BIT(7),
    HandheldLark = BIT(8),
    Lucia = BIT(9),
    Lagon = BIT(10),
    Lager = BIT(11),
    SystemExt = BIT(29),
    System = BIT(30),

    FullCtrl = FullKey | Handheld | JoyDual,
    Standard = FullCtrl | JoyLeft | JoyRight,
};
ENABLE_ENUM_BITMASK_OPERATORS(NpadStyleSet)

enum class ColorAttribute {
    Ok = 0,
    ReadError = 1,
    NoController = 2,
};

enum class NpadButtons : u64 {
    None,
    A = BITL(0),
    B = BITL(1),
    X = BITL(2),
    Y = BITL(3),
    StickL = BITL(4),
    StickR = BITL(5),
    L = BITL(6),
    R = BITL(7),
    ZL = BITL(8),
    ZR = BITL(9),
    Plus = BITL(10),
    Minus = BITL(11),
    Left = BITL(12),
    Up = BITL(13),
    Right = BITL(14),
    Down = BITL(15),
    StickLLeft = BITL(16),
    StickLUp = BITL(17),
    StickLRight = BITL(18),
    StickLDown = BITL(19),
    StickRLeft = BITL(20),
    StickRUp = BITL(21),
    StickRRight = BITL(22),
    StickRDown = BITL(23),
    LeftSL = BITL(24),
    LeftSR = BITL(25),
    RightSL = BITL(26),
    RightSR = BITL(27),
    Palma = BITL(28),
    Verification = BITL(29),
    HandheldLeftB = BITL(30),
    LagonCLeft = BITL(31),
    LagonCUp = BITL(32),
    LagonCRight = BITL(33),
    LagonCDown = BITL(34),
};
ENABLE_ENUM_BITMASK_OPERATORS(NpadButtons)

enum class DebugPadAttribute {
    IsConnected = BIT(0),
};

enum class HidTouchAttribute {
    Start = BIT(0),
    End = BIT(1),
};

enum class MouseAttribute {
    Transferable = BIT(0),
    IsConnected = BIT(1),
};

enum class NpadAttributes : u32 {
    IsConnected = BIT(0),
    IsWired = BIT(1),
    IsLeftConnected = BIT(2),
    IsLeftWired = BIT(3),
    IsRightConnected = BIT(4),
    IsRightWired = BIT(5),
};

enum class SixAxisSensorAttribute {
    IsConnected = BIT(0),
    IsInterpolated = BIT(1),
};

enum class GestureAttribute {
    IsNewTouch = BIT(4),
    IsDoubleTap = BIT(8),
};

enum class GestureDirection {
    None = 0,
    Left = 1,
    Up = 2,
    Right = 3,
    Down = 4,
};

enum class GestureType {
    Idle = 0,
    Complete = 1,
    Cancel = 2,
    Touch = 3,
    Press = 4,
    Tap = 5,
    Pan = 6,
    Swipe = 7,
    Pinch = 8,
    Rotate = 9,
};

enum class GyroscopeZeroDriftMode {
    Loose = 0,
    Standard = 1,
    Tight = 2,
};

enum class NpadJoyHoldType {
    Vertical = 0,
    Horizontal = 1,
};

enum class NpadJoyDeviceType {
    Left = 0,
    Right = 1,
};

enum class NpadHandheldActivationMode {
    Dual = 0,
    Single = 1,
    None = 2,
};

enum class NpadJoyAssignmentMode {
    Dual = 0,
    Single = 1,
};

enum class NpadCommunicationMode {
    Ms5 = 0,
    Ms10 = 1,
    Ms15 = 2,
    Default = 3,
};

enum class DeviceTypeBits : u32 {
    FullKey = BIT(0),
    DebugPad = BIT(1),
    HandheldLeft = BIT(2),
    HandheldRight = BIT(3),
    JoyLeft = BIT(4),
    JoyRight = BIT(5),
    Palma = BIT(6),
    LarkHvcLeft = BIT(7),
    LarkHvcRight = BIT(8),
    LarkNesLeft = BIT(9),
    LarkNesRight = BIT(10),
    HandheldLarkHvcLeft = BIT(11),
    HandheldLarkHvcRight = BIT(12),
    HandheldLarkNesLeft = BIT(13),
    HandheldLarkNesRight = BIT(14),
    Lucia = BIT(15),
    Lagon = BIT(16),
    Lager = BIT(17),
    System = BIT(31),
};

enum class DeviceType {
    JoyRight1 = 1,
    JoyLeft2 = 2,
    FullKey3 = 3,
    JoyLeft4 = 4,
    JoyRight5 = 5,
    FullKey6 = 6,
    LarkHvcLeft = 7,
    LarkHvcRight = 8,
    LarkNesLeft = 9,
    LarkNesRight = 10,
    Lucia = 11,
    Palma = 12,
    FullKey13 = 13,
    FullKey15 = 15,
    DebugPad = 17,
    System19 = 19,
    System20 = 20,
    System21 = 21,
    Lagon = 22,
    Lager = 28,
};

enum class AppletFooterUiType {
    None = 0,
    HandheldNone = 1,
    HandheldJoyConLeftOnly = 2,
    HandheldJoyConRightOnly = 3,
    HandheldJoyConLeftJoyConRight = 4,
    JoyDual = 5,
    JoyDualLeftOnly = 6,
    JoyDualRightOnly = 7,
    JoyLeftHorizontal = 8,
    JoyLeftVertical = 9,
    JoyRightHorizontal = 10,
    JoyRightVertical = 11,
    SwitchProController = 12,
    CompatibleProController = 13,
    CompatibleJoyCon = 14,
    LarkHvc1 = 15,
    LarkHvc2 = 16,
    LarkNesLeft = 17,
    LarkNesRight = 18,
    Lucia = 19,
    Verification = 20,
    Lagon = 21,
};

enum class NpadInterfaceType {
    Bluetooth = 1,
    Rail = 2,
    USB = 3,
    Unknown4 = 4,
};

enum class XcdInterfaceType {
    Bluetooth = BIT(0),
    Uart = BIT(1),
    Usb = BIT(2),
    FieldSet = BIT(7),
};

enum class NpadLarkType {
    Invalid = 0,
    H1 = 1,
    H2 = 2,
    NL = 3,
    NR = 4,
};

enum class NpadLuciaType {
    Invalid = 0,
    J = 1,
    E = 2,
    U = 3,
};

enum class NpadLagerType {
    Invalid = 0,
    J = 1,
    E = 2,
    U = 3,
};

enum class VibrationDeviceType {
    Unknown = 0,
    LinearResonantActuator = 1,
    GcErm = 2,
};

enum class VibrationDevicePosition {
    None = 0,
    Left = 1,
    Right = 2,
};

enum class VibrationGcErmCommand {
    Stop = 0,
    Start = 1,
    StopHard = 2,
};

enum class PalmaOperationType {
    PlayActivity = 0,
    SetFrModeType = 1,
    ReadStep = 2,
    EnableStep = 3,
    ResetStep = 4,
    ReadApplicationSection = 5,
    WriteApplicationSection = 6,
    ReadUniqueCode = 7,
    SetUniqueCodeInvalid = 8,
    WriteActivityEntry = 9,
    WriteRgbLedPatternEntry = 10,
    WriteWaveEntry = 11,
    ReadDataBaseIdentificationVersion = 12,
    WriteDataBaseIdentificationVersion = 13,
    SuspendFeature = 14,
    ReadPlayLog = 15,
    ResetPlayLog = 16,
};

enum class PalmaFrModeType {
    Off = 0,
    B01 = 1,
    B02 = 2,
    B03 = 3,
    Downloaded = 4,
};

enum class PalmaWaveSet {
    Small = 0,
    Medium = 1,
    Large = 2,
};

enum class PalmaFeature {
    FrMode = BIT(0),
    RumbleFeedback = BIT(1),
    Step = BIT(2),
    MuteSwitch = BIT(3),
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

struct CommonLifoHeader {
    u64 unused;
    u64 buffer_count;
    u64 tail;
    u64 count;
};

struct DebugPadState {
    u64 sampling_number;
    u32 attributes;
    u32 buttons;
    AnalogStickState analog_stick_r;
    AnalogStickState analog_stick_l;
};

struct DebugPadStateAtomicStorage {
    u64 sampling_number;
    DebugPadState state;
};

struct DebugPadLifo {
    CommonLifoHeader header;
    DebugPadStateAtomicStorage storage[17];
};

struct DebugPadSharedMemoryFormat {
    DebugPadLifo lifo;
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

struct TouchScreenStateAtomicStorage {
    u64 sampling_number;
    TouchScreenState state;
};

struct TouchScreenLifo {
    CommonLifoHeader header;
    TouchScreenStateAtomicStorage storage[17];
};

struct TouchScreenSharedMemoryFormat {
    TouchScreenLifo lifo;
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

struct MouseStateAtomicStorage {
    u64 sampling_number;
    MouseState state;
};

struct MouseLifo {
    CommonLifoHeader header;
    MouseStateAtomicStorage storage[17];
};

struct MouseSharedMemoryFormat {
    MouseLifo lifo;
    u8 padding[0xB0];
};

struct KeyboardState {
    u64 sampling_number;
    u64 modifiers;
    u64 keys[4];
};

struct KeyboardStateAtomicStorage {
    u64 sampling_number;
    KeyboardState state;
};

struct KeyboardLifo {
    CommonLifoHeader header;
    KeyboardStateAtomicStorage storage[17];
};

struct KeyboardSharedMemoryFormat {
    KeyboardLifo lifo;
    u8 padding[0x28];
};

struct BasicXpadState {
    u64 sampling_number;
    u32 attributes;
    u32 buttons;
    u64 analog_stick_left;
    u64 analog_stick_right;
};

struct BasicXpadStateAtomicStorage {
    u64 sampling_number;
    BasicXpadState state;
};

struct BasicXpadLifo {
    CommonLifoHeader header;
    BasicXpadStateAtomicStorage storage[17];
};

struct BasicXpadSharedMemoryEntry {
    BasicXpadLifo lifo;
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

struct DigitizerStateAtomicStorage {
    u64 sampling_number;
    DigitizerState state;
};

struct DigitizerLifo {
    CommonLifoHeader header;
    DigitizerStateAtomicStorage storage[17];
};

struct DigitizerSharedMemoryFormat {
    DigitizerLifo lifo;
    u8 padding[0x980];
};

struct HomeButtonState {
    u64 sampling_number;
    u64 buttons;
};

struct HomeButtonStateAtomicStorage {
    u64 sampling_number;
    HomeButtonState state;
};

struct HomeButtonLifo {
    CommonLifoHeader header;
    HomeButtonStateAtomicStorage storage[17];
};

struct HomeButtonSharedMemoryFormat {
    HomeButtonLifo lifo;
    u8 padding[0x48];
};

struct SleepButtonState {
    u64 sampling_number;
    u64 buttons;
};

struct SleepButtonStateAtomicStorage {
    u64 sampling_number;
    SleepButtonState state;
};

struct SleepButtonLifo {
    CommonLifoHeader header;
    SleepButtonStateAtomicStorage storage[17];
};

struct SleepButtonSharedMemoryFormat {
    SleepButtonLifo lifo;
    u8 padding[0x48];
};

struct CaptureButtonState {
    u64 sampling_number;
    u64 buttons;
};

struct CaptureButtonStateAtomicStorage {
    u64 sampling_number;
    CaptureButtonState state;
};

struct CaptureButtonLifo {
    CommonLifoHeader header;
    CaptureButtonStateAtomicStorage storage[17];
};

struct CaptureButtonSharedMemoryFormat {
    CaptureButtonLifo lifo;
    u8 padding[0x48];
};

struct InputDetectorState {
    u64 input_source_state;
    u64 sampling_number;
};

struct InputDetectorStateAtomicStorage {
    u64 sampling_number;
    InputDetectorState state;
};

struct InputDetectorLifo {
    CommonLifoHeader header;
    InputDetectorStateAtomicStorage storage[2];
};

struct InputDetectorSharedMemoryEntry {
    InputDetectorLifo lifo;
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

struct SixAxisSensorUserCalibrationStateAtomicStorage {
    u64 sampling_number;
    SixAxisSensorUserCalibrationState calib_state;
};

struct SixAxisSensorUserCalibrationStateLifo {
    CommonLifoHeader header;
    SixAxisSensorUserCalibrationStateAtomicStorage storage[2];
};

struct AnalogStickCalibrationStateImpl {
    u64 state;
    u64 flags;
    u64 stage;
    u64 sampling_number;
};

struct AnalogStickCalibrationStateImplAtomicStorage {
    u64 sampling_number;
    AnalogStickCalibrationStateImpl calib_state;
};

struct AnalogStickCalibrationStateImplLifo {
    CommonLifoHeader header;
    AnalogStickCalibrationStateImplAtomicStorage storage[2];
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

struct UniquePadConfigAtomicStorage {
    u64 sampling_number;
    UniquePadConfig config;
};

struct UniquePadConfigLifo {
    CommonLifoHeader header;
    UniquePadConfigAtomicStorage storage[2];
};

struct UniquePadLifo {
    UniquePadConfigLifo config_lifo;
    AnalogStickCalibrationStateImplLifo analog_stick_calib_lifo[2];
    SixAxisSensorUserCalibrationStateLifo sixaxis_calib_lifo;
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
    u32 main;
    u32 sub;
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

struct NpadCommonStateAtomicStorage {
    u64 sampling_number;
    NpadCommonState state;
};

struct NpadCommonLifo {
    CommonLifoHeader header;
    NpadCommonStateAtomicStorage storage[17];
};

struct NpadGcTriggerState {
    u64 sampling_number;
    u32 trigger_l;
    u32 trigger_r;
};

struct NpadGcTriggerStateAtomicStorage {
    u64 sampling_number;
    NpadGcTriggerState state;
};

struct NpadGcTriggerLifo {
    CommonLifoHeader header;
    NpadGcTriggerStateAtomicStorage storage[17];
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

struct SixAxisSensorStateAtomicStorage {
    u64 sampling_number;
    SixAxisSensorState state;
};

struct NpadSixAxisSensorLifo {
    CommonLifoHeader header;
    SixAxisSensorStateAtomicStorage storage[17];
};

struct NpadSystemProperties {
    u64 is_charging : 3;
    u64 is_powered : 3;
    u64 bit6 : 1;
    u64 bit7 : 1;
    u64 bit8 : 1;
    u64 is_unsupported_button_pressed_on_npad_system : 1;
    u64 is_unsupported_button_pressed_on_npad_system_ext : 1;
    u64 is_abxy_button_oriented : 1;
    u64 is_sl_sr_button_oriented : 1;
    u64 is_plus_available : 1;
    u64 is_minus_available : 1;
    u64 is_directional_buttons_available : 1;
    u64 unused : 48;
};

struct NpadSystemButtonProperties {
    u32 is_unintended_home_button_input_protection_enabled : 1;
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

struct NfcXcdDeviceHandleStateImplAtomicStorage {
    u64 sampling_number;
    NfcXcdDeviceHandleStateImpl state;
};

struct NfcXcdDeviceHandleState {
    CommonLifoHeader header;
    NfcXcdDeviceHandleStateImplAtomicStorage storage[2];
};

struct NpadInternalState {
    NpadStyleSet style_set;
    u32 joy_assignment_mode;
    NpadFullKeyColorState full_key_color;
    NpadJoyColorState joy_color;
    NpadCommonLifo full_key_lifo;
    NpadCommonLifo handheld_lifo;
    NpadCommonLifo joy_dual_lifo;
    NpadCommonLifo joy_left_lifo;
    NpadCommonLifo joy_right_lifo;
    NpadCommonLifo palma_lifo;
    NpadCommonLifo system_ext_lifo;
    NpadSixAxisSensorLifo full_key_six_axis_sensor_lifo;
    NpadSixAxisSensorLifo handheld_six_axis_sensor_lifo;
    NpadSixAxisSensorLifo joy_dual_left_six_axis_sensor_lifo;
    NpadSixAxisSensorLifo joy_dual_right_six_axis_sensor_lifo;
    NpadSixAxisSensorLifo joy_left_six_axis_sensor_lifo;
    NpadSixAxisSensorLifo joy_right_six_axis_sensor_lifo;
    u32 device_type;
    u32 reserved;
    NpadSystemProperties system_properties;
    NpadSystemButtonProperties system_button_properties;
    u32 battery_level[3];
    union {
        struct {
            NfcXcdDeviceHandleState nfc_xcd_device_handle;
        };
        struct {
            u32 applet_footer_ui_attribute;
            u8 applet_footer_ui_type;
            u8 reserved_x41AD[0x5B];
        };
    };
    u8 reserved_x4208[0x20];
    NpadGcTriggerLifo gc_trigger_lifo;
    u32 lark_type_l_and_main;
    u32 lark_type_r;
    u32 lucia_type;
    u32 lager_type;
};

struct NpadSharedMemoryEntry {
    NpadInternalState internal_state;
    u8 pad[0xC10];
};

struct NpadSharedMemoryFormat {
    NpadSharedMemoryEntry entries[10];
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

struct GestureDummyStateAtomicStorage {
    u64 sampling_number;
    GestureState state;
};

struct GestureLifo {
    CommonLifoHeader header;
    GestureDummyStateAtomicStorage storage[17];
};

struct GestureSharedMemoryFormat {
    GestureLifo lifo;
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

} // namespace hydra::horizon::hid
