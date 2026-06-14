#pragma once

namespace hydra::horizon::services::hid {

constexpr usize NPAD_COUNT = 10;

enum class NpadRevision : u32 {
    Revision0 = 0,
    Revision1 = 1,
    Revision2 = 2,
    Revision3 = 3,
};

enum class DebugPadButton : u32 {
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

enum class TouchScreenModeForNx : u32 {
    UseSystemSetting = 0,
    Finger = 1,
    Heat2 = 2,
};

enum class MouseButton : u32 {
    Left = BIT(0),
    Right = BIT(1),
    Middle = BIT(2),
    Forward = BIT(3),
    Back = BIT(4),
};

enum class KeyboardKey : u32 {
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

enum class KeyboardModifier : u32 {
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

enum class KeyboardLockKeyEvent : u32 {
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

enum class NpadIdType : u32 {
    No1 = 0x0,
    No2 = 0x1,
    No3 = 0x2,
    No4 = 0x3,
    No5 = 0x4,
    No6 = 0x5,
    No7 = 0x6,
    No8 = 0x7,
    Other = 0x10,
    Handheld = 0x20,
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
ENABLE_ENUM_BITWISE_OPERATORS(NpadStyleSet)

enum class ColorAttribute : u32 {
    Ok = 0,
    ReadError = 1,
    NoController = 2,
};

enum class NpadButtons : u64 {
    None = 0,

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

    // HACK: alias
    Invalid = None,
};
ENABLE_ENUM_BITWISE_OPERATORS(NpadButtons)

enum class NpadColor : u32 {
    BodyGray = 0x828282,
    BodyNeonRed = 0xFF3C28,
    BodyNeonBlue = 0x0AB9E6,
    BodyNeonYellow = 0xE6FF00,
    BodyNeonGreen = 0x1EDC00,
    BodyNeonPink = 0xFF3278,
    BodyRed = 0xE10F00,
    BodyBlue = 0x4655F5,
    BodyNeonPurple = 0xB400E6,
    BodyNeonOrange = 0xFAA005,
    BodyPokemonLetsGoPikachu = 0xFFDC00,
    BodyPokemonLetsGoEevee = 0xC88C32,
    BodyNintendoLaboCreatorsContestEdition = 0xD7AA73,
    BodyAnimalCrossingSpecialEditionLeftJoyCon = 0x82FF96,
    BodyAnimalCrossingSpecialEditionRightJoyCon = 0x96F5F5,

    ButtonGray = 0x0F0F0F,
    ButtonNeonRed = 0x1E0A0A,
    ButtonNeonBlue = 0x001E1E,
    ButtonNeonYellow = 0x142800,
    ButtonNeonGreen = 0x002800,
    ButtonNeonPink = 0x28001E,
    ButtonRed = 0x280A0A,
    ButtonBlue = 0x00000A,
    ButtonNeonPurple = 0x140014,
    ButtonNeonOrange = 0x0F0A00,
    ButtonPokemonLetsGoPikachu = 0x322800,
    ButtonPokemonLetsGoEevee = 0x281900,
    ButtonNintendoLaboCreatorsContestEdition = 0x1E1914,
    ButtonAnimalCrossingSpecialEditionLeftJoyCon = 0x0A1E0A,
    ButtonAnimalCrossingSpecialEditionRightJoyCon = 0x0A1E28,
};

enum class NpadSystemProperties : u64 {
    None = 0,
    IsChargingJoyDual = BIT(0),
    IsChargingJoyLeft = BIT(1),
    IsChargingJoyRight = BIT(2),
    IsPoweredJoyDual = BIT(3),
    IsPoweredJoyLeft = BIT(4),
    IsPoweredJoyRight = BIT(5),
    IsUnsuportedButtonPressedOnNpadSystem = BIT(9),
    IsUnsuportedButtonPressedOnNpadSystemExt = BIT(10),
    IsAbxyButtonOriented = BIT(11),
    IsSlSrButtonOriented = BIT(12),
    IsPlusAvailable = BIT(13),
    IsMinusAvailable = BIT(14),
    IsDirectionalButtonsAvailable = BIT(15),
};
ENABLE_ENUM_BITWISE_OPERATORS(NpadSystemProperties)

enum class NpadSystemButtonProperties : u32 {
    None = 0,
    IsUnintendedHomeButtonInputProtectionEnabled = BIT(0),
};
ENABLE_ENUM_BITWISE_OPERATORS(NpadSystemButtonProperties)

enum class DebugPadAttribute : u32 {
    IsConnected = BIT(0),
};

enum class HidTouchAttribute : u32 {
    Start = BIT(0),
    End = BIT(1),
};

enum class MouseAttribute : u32 {
    Transferable = BIT(0),
    IsConnected = BIT(1),
};

enum class NpadAttributes : u32 {
    None = 0,
    IsConnected = BIT(0),
    IsWired = BIT(1),
    IsLeftConnected = BIT(2),
    IsLeftWired = BIT(3),
    IsRightConnected = BIT(4),
    IsRightWired = BIT(5),
};
ENABLE_ENUM_BITWISE_OPERATORS(NpadAttributes)

enum class SixAxisSensorAttribute : u32 {
    IsConnected = BIT(0),
    IsInterpolated = BIT(1),
};

enum class GestureAttribute : u32 {
    IsNewTouch = BIT(4),
    IsDoubleTap = BIT(8),
};

enum class GestureDirection : u32 {
    None = 0,
    Left = 1,
    Up = 2,
    Right = 3,
    Down = 4,
};

enum class GestureType : u32 {
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

enum class GyroscopeZeroDriftMode : u32 {
    Loose = 0,
    Standard = 1,
    Tight = 2,
};

enum class NpadJoyHoldType : u32 {
    Vertical = 0,
    Horizontal = 1,
};

enum class NpadJoyDeviceType : u32 {
    Left = 0,
    Right = 1,
};

enum class NpadHandheldActivationMode {
    Dual = 0,
    Single = 1,
    None = 2,
};

enum class NpadJoyAssignmentMode : u32 {
    Dual = 0,
    Single = 1,
};

enum class NpadCommunicationMode : u32 {
    Ms5 = 0,
    Ms10 = 1,
    Ms15 = 2,
    Default = 3,
};

enum class NpadBatteryLevel : u32 {
    Percent0,
    Percent25,
    Percent50,
    Percent75,
    Percent100,
};

enum class DeviceTypeBits : u32 {
    None = 0,
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
ENABLE_ENUM_BITWISE_OPERATORS(DeviceTypeBits)

enum class DeviceType : u32 {
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

enum class AppletFooterUiType : u8 {
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

enum class NpadInterfaceType : u32 {
    Bluetooth = 1,
    Rail = 2,
    USB = 3,
    Unknown4 = 4,
};

enum class XcdInterfaceType : u32 {
    Bluetooth = BIT(0),
    Uart = BIT(1),
    Usb = BIT(2),
    FieldSet = BIT(7),
};

enum class NpadLarkType : u32 {
    Invalid = 0,
    H1 = 1,
    H2 = 2,
    NL = 3,
    NR = 4,
};

enum class NpadLuciaType : u32 {
    Invalid = 0,
    J = 1,
    E = 2,
    U = 3,
};

enum class NpadLagerType : u32 {
    Invalid = 0,
    J = 1,
    E = 2,
    U = 3,
};

enum class VibrationDeviceType : u32 {
    Unknown = 0,
    LinearResonantActuator = 1,
    GcErm = 2,
    Erm = 3, // TODO: correct?
};

enum class VibrationDevicePosition : u32 {
    None = 0,
    Left = 1,
    Right = 2,
};

enum class VibrationGcErmCommand : u32 {
    Stop = 0,
    Start = 1,
    StopHard = 2,
};

enum class PalmaOperationType : u32 {
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

enum class PalmaFrModeType : u32 {
    Off = 0,
    B01 = 1,
    B02 = 2,
    B03 = 3,
    Downloaded = 4,
};

enum class PalmaWaveSet : u32 {
    Small = 0,
    Medium = 1,
    Large = 2,
};

enum class PalmaFeature : u32 {
    FrMode = BIT(0),
    RumbleFeedback = BIT(1),
    Step = BIT(2),
    MuteSwitch = BIT(3),
};

} // namespace hydra::horizon::services::hid

ENABLE_ENUM_FORMATTING(hydra::horizon::services::hid::NpadRevision, Revision0,
                       "revision 0", Revision1, "revision 1", Revision2,
                       "revision 2", Revision3, "revision 3")

ENABLE_ENUM_FORMATTING(hydra::horizon::services::hid::NpadIdType, No1,
                       "Number 1", No2, "Number 2", No3, "Number 3", No4,
                       "Number 4", No5, "Number 5", No6, "Number 6", No7,
                       "Number 7", No8, "Number 8", Other, "Other", Handheld,
                       "Handheld")

ENABLE_ENUM_FLAGS_FORMATTING(hydra::horizon::services::hid::NpadStyleSet,
                             FullKey, "full key", Handheld, "handheld", JoyDual,
                             "joy dual", JoyLeft, "joy left", JoyRight,
                             "joy right", Gc, "GC", Palma, "palma", Lark,
                             "lark", HandheldLark, "handheld lark", Lucia,
                             "lucia", Lagon, "lagon", Lager, "lager", SystemExt,
                             "system ext", System, "system")
