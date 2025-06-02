#pragma once

#include "core/horizon/kernel/const.hpp"

namespace hydra::horizon {

using result_t = kernel::result_t;

struct FirmwareVersion {
    u8 major;
    u8 minor;
    u8 micro;
    u8 padding1;
    u8 revision_major;
    u8 revision_minor;
    u8 padding2;
    u8 padding3;
    char platform[0x20];
    char version_hash[0x40];
    char display_version[0x18];
    char display_title[0x80];
};

constexpr FirmwareVersion FIRMWARE_VERSION = {
    .major = 1,
    .minor = 0,
    .micro = 0,
    .revision_major = 0,
    .revision_minor = 0,
    .platform = "NX",
    .version_hash = "voyp5gq7m551zuqgspcgobbmo74rg6yydpalt72l",
    .display_version = "1.0.0",
    .display_title = "hydra firmware 1.0.0",
};

enum class ConfigEntryType : u32 {
    EndOfList = 0,        ///< Entry list terminator.
    MainThreadHandle = 1, ///< Provides the handle to the main thread.
    NextLoadPath = 2,    ///< Provides a buffer containing information about the
                         ///< next homebrew application to load.
    OverrideHeap = 3,    ///< Provides heap override information.
    OverrideService = 4, ///< Provides service override information.
    Argv = 5,            ///< Provides argv.
    SyscallAvailableHint =
        6,          ///< Provides syscall availability hints (SVCs 0x00..0x7F).
    AppletType = 7, ///< Provides APT applet type.
    AppletWorkaround =
        8, ///< Indicates that APT is broken and should not be used.
    Reserved9 =
        9, ///< Unused/reserved entry type, formerly used by StdioSockets.
    ProcessHandle = 10,  ///< Provides the process handle.
    LastLoadResult = 11, ///< Provides the last load result.
    RandomSeed = 14, ///< Provides random data used to seed the pseudo-random
                     ///< number generator.
    UserIdStorage =
        15, ///< Provides persistent storage for the preselected user id.
    HosVersion = 16, ///< Provides the currently running horizon OS version.
    SyscallAvailableHint2 =
        17, ///< Provides syscall availability hints (SVCs 0x80..0xBF).
};

enum class ConfigEntryFlag : u32 {
    None = 0,
    IsMandatory = BIT(0), ///< Specifies that the entry **must** be processed by
                          ///< the homebrew application.
};

struct ConfigEntry {
    ConfigEntryType type;  ///< Type of entry
    ConfigEntryFlag flags; ///< Entry flags
    u64 values[2];         ///< Entry arguments (type-specific)
};

enum class AppletMessage {
    None = 0,
    ChangeIntoForeground = 1,
    ChangeIntoBackground = 2,
    Exit = 4,
    ApplicationExited = 6,
    FocusStateChanged = 15,
    Resume = 16,
    DetectShortPressingHomeButton = 20,
    DetectLongPressingHomeButton = 21,
    DetectShortPressingPowerButton = 22,
    DetectMiddlePressingPowerButton = 23,
    DetectLongPressingPowerButton = 24,
    RequestToPrepareSleep = 25,
    FinishedSleepSequence = 26,
    SleepRequiredByHighTemperature = 27,
    SleepRequiredByLowBattery = 28,
    AutoPowerDown = 29,
    OperationModeChanged = 30,
    PerformanceModeChanged = 31,
    DetectReceivingCecSystemStandby = 32,
    SdCardRemoved = 33,
    LaunchApplicationRequested = 50,
    RequestToDisplay = 51,
    ShowApplicationLogo = 55,
    HideApplicationLogo = 56,
    ForceHideApplicationLogo = 57,
    FloatingApplicationDetected = 60,
    DetectShortPressingCaptureButton = 90,
    AlbumScreenShotTaken = 92,
    AlbumRecordingSaved = 93,
    DetectShortPressingCaptureButtonForApplet = 110,
    DetectLongPressingCaptureButtonForApplet = 111,
};

enum class AppletFocusState {
    InFocus = 1,    ///< Applet is focused.
    OutOfFocus = 2, ///< Out of focus - LibraryApplet open.
    Background = 3  ///< Out of focus - HOME menu open / console is sleeping.
};

enum class LaunchParameterKind : u32 {
    UserChannel = 1,
    PreselectedUser,
    Unknown0,
};

enum class LanguageCode : u64 {
    Japanese = str_to_u64("ja"),
    AmericanEnglish = str_to_u64("en-US"),
    French = str_to_u64("fr"),
    German = str_to_u64("de"),
    Italian = str_to_u64("it"),
    Spanish = str_to_u64("es"),
    Chinese = str_to_u64("zh-CN"),
    Korean = str_to_u64("ko"),
    Dutch = str_to_u64("nl"),
    Portuguese = str_to_u64("pt"),
    FuckRussia = str_to_u64("ru"), // TODO: should be "Russian"
    Russian = FuckRussia,
    Taiwanese = str_to_u64("zh-TW"),
    BritishEnglish = str_to_u64("en-GB"),
    CanadianFrench = str_to_u64("fr-CA"),
    LatinAmericanSpanish = str_to_u64("es-419"),
    SimplifiedChinese = str_to_u64("zh-Hans"),
    TraditionalChinese = str_to_u64("zh-Hant"),
    BrazilianPortugese = str_to_u64("pt-BR"),
};

constexpr LanguageCode available_languages[] = {
    LanguageCode::Japanese,
    LanguageCode::AmericanEnglish,
    LanguageCode::French,
    LanguageCode::German,
    LanguageCode::Italian,
    LanguageCode::Spanish,
    LanguageCode::Chinese,
    LanguageCode::Korean,
    LanguageCode::Dutch,
    LanguageCode::Portuguese,
    LanguageCode::Russian,
    LanguageCode::Taiwanese,
    LanguageCode::BritishEnglish,
    LanguageCode::CanadianFrench,
    LanguageCode::LatinAmericanSpanish,
    LanguageCode::SimplifiedChinese,
    LanguageCode::TraditionalChinese,
    LanguageCode::BrazilianPortugese,
};

enum class AppletId : u32 {
    None = 0x00,
    Application = 0x01,
    OverlayApplet = 0x02,
    SystemAppletMenu = 0x03,
    SystemApplication = 0x04,
    LibraryAppletAuth = 0x0A,
    LibraryAppletCabinet = 0x0B,
    LibraryAppletController = 0x0C,
    LibraryAppletDataErase = 0x0D,
    LibraryAppletError = 0x0E,
    LibraryAppletNetConnect = 0x0F,
    LibraryAppletPlayerSelect = 0x10,
    LibraryAppletSwkbd = 0x11,
    LibraryAppletMiiEdit = 0x12,
    LibraryAppletWeb = 0x13,
    LibraryAppletShop = 0x14,
    LibraryAppletPhotoViewer = 0x15,
    LibraryAppletSet = 0x16,
    LibraryAppletOfflineWeb = 0x17,
    LibraryAppletLoginShare = 0x18,
    LibraryAppletWifiWebAuth = 0x19,
    LibraryAppletMyPage = 0x1A,
    LibraryAppletGift = 0x1B,
    LibraryAppletUserMigration = 0x1C,
    LibraryAppletPreomiaSys = 0x1D,
    LibraryAppletStory = 0x1E,
    LibraryAppletPreomiaUsr = 0x1F,
    LibraryAppletPreomiaUsrDummy = 0x20,
    LibraryAppletSample = 0x21,
    LibraryAppletPromoteQualification = 0x22,
    DevelopmentTool = 0x3E8,
    CombinationLA = 0x3F1,
    AeSystemApplet = 0x3F2,
    AeOverlayApplet = 0x3F3,
    AeStarter = 0x3F4,
    AeLibraryAppletAlone = 0x3F5,
    AeLibraryApplet1 = 0x3F6,
    AeLibraryApplet2 = 0x3F7,
    AeLibraryApplet3 = 0x3F8,
    AeLibraryApplet4 = 0x3F9,
    AppletISA = 0x3FA,
    AppletIOA = 0x3FB,
    AppletISTA = 0x3FC,
    AppletILA1 = 0x3FD,
    AppletILA2 = 0x3FE,
    Applet700000C8 = 0x700000C8,
    Applet700000C9 = 0x700000C9,
    Applet700000DC = 0x700000DC,
    Applet700000E6 = 0x700000E6,
    Applet700000E7 = 0x700000E7,
    Applet700000E8 = 0x700000E8,
    Applet700000E9 = 0x700000E9,
    Applet700000EA = 0x700000EA,
    Applet700000EB = 0x700000EB,
    Applet700000EC = 0x700000EC,
    Applet700000ED = 0x700000ED,
    Applet700000F0 = 0x700000F0,
    Applet700000F1 = 0x700000F1,
    Applet700000F2 = 0x700000F2,
    Applet700000F3 = 0x700000F3,
    Applet700000F4 = 0x700000F4,
};

enum class LibraryAppletMode : u32 {
    AllForeground,
    PartialForeground,
    NoUi,
    PartialForegroundWithIndirectDisplay,
    AllForegroundInitiallyHidden,
};

} // namespace hydra::horizon

ENABLE_ENUM_FORMATTING(
    hydra::horizon::AppletMessage, None, "none", ChangeIntoForeground,
    "change into foreground", ChangeIntoBackground, "change into background",
    Exit, "exit", ApplicationExited, "application exited", FocusStateChanged,
    "focus state changed", Resume, "resume", DetectShortPressingHomeButton,
    "detect short pressing home button", DetectLongPressingHomeButton,
    "detect long pressing home button", DetectShortPressingPowerButton,
    "detect short pressing power button", DetectMiddlePressingPowerButton,
    "detect middle pressing power button", DetectLongPressingPowerButton,
    "detect long pressing power button", RequestToPrepareSleep,
    "request to prepare sleep", FinishedSleepSequence,
    "finished sleep sequence", SleepRequiredByHighTemperature,
    "sleep required by high temperature", SleepRequiredByLowBattery,
    "sleep required by low battery", AutoPowerDown, "auto power down",
    OperationModeChanged, "operation mode changed", PerformanceModeChanged,
    "performance mode changed", DetectReceivingCecSystemStandby,
    "detect receiving cec system standby", SdCardRemoved, "sd card removed",
    LaunchApplicationRequested, "launch application requested",
    RequestToDisplay, "request to display", ShowApplicationLogo,
    "show application logo", HideApplicationLogo, "hide application logo",
    ForceHideApplicationLogo, "force hide application logo",
    FloatingApplicationDetected, "floating application detected",
    DetectShortPressingCaptureButton, "detect short pressing capture button",
    AlbumScreenShotTaken, "album screenshot taken", AlbumRecordingSaved,
    "album recording saved", DetectShortPressingCaptureButtonForApplet,
    "detect short pressing capture button for applet",
    DetectLongPressingCaptureButtonForApplet,
    "detect long pressing capture button for applet")

ENABLE_ENUM_FORMATTING(hydra::horizon::LaunchParameterKind, UserChannel,
                       "user channel", PreselectedUser, "preselected user",
                       Unknown0, "unknown0")

ENABLE_ENUM_FORMATTING(
    hydra::horizon::AppletId, None, "none", Application, "application",
    LibraryAppletAuth, "library applet auth", LibraryAppletCabinet,
    "library applet cabinet", LibraryAppletController,
    "library applet controller", LibraryAppletDataErase,
    "library applet data erase", LibraryAppletError, "library applet error",
    LibraryAppletNetConnect, "library applet net connect",
    LibraryAppletPlayerSelect, "library applet player select",
    LibraryAppletSwkbd, "library applet swkbd", LibraryAppletMiiEdit,
    "library applet mii edit", LibraryAppletWeb, "library applet web",
    LibraryAppletShop, "library applet shop", LibraryAppletPhotoViewer,
    "library applet photo viewer", LibraryAppletSet, "library applet set",
    LibraryAppletOfflineWeb, "library applet offline web",
    LibraryAppletLoginShare, "library applet login share",
    LibraryAppletWifiWebAuth, "library applet wifi web auth",
    LibraryAppletMyPage, "library applet my page", LibraryAppletGift,
    "library applet gift", LibraryAppletUserMigration,
    "library applet user migration", LibraryAppletPreomiaSys,
    "library applet preomia sys", LibraryAppletStory, "library applet story",
    LibraryAppletPreomiaUsr, "library applet preomia usr",
    LibraryAppletPreomiaUsrDummy, "library applet preomia usr dummy",
    LibraryAppletSample, "library applet sample",
    LibraryAppletPromoteQualification, "library applet promote qualification",
    DevelopmentTool, "development tool", CombinationLA, "combination la",
    AeSystemApplet, "ae system applet", AeOverlayApplet, "ae overlay applet",
    AeStarter, "ae starter", AeLibraryAppletAlone, "ae library applet alone",
    AeLibraryApplet1, "ae library applet 1", AeLibraryApplet2,
    "ae library applet 2", AeLibraryApplet3, "ae library applet 3",
    AeLibraryApplet4, "ae library applet 4", AppletISA, "applet isa", AppletIOA,
    "applet ioa", AppletISTA, "applet ista", AppletILA1, "applet ila 1",
    AppletILA2, "applet ila 2", Applet700000C8, "applet 700000c8",
    Applet700000C9, "applet 700000c9", Applet700000DC, "applet 700000dc",
    Applet700000E6, "applet 700000e6", Applet700000E7, "applet 700000e7",
    Applet700000E8, "applet 700000e8", Applet700000E9, "applet 700000e9",
    Applet700000EA, "applet 700000ea", Applet700000EB, "applet 700000eb",
    Applet700000EC, "applet 700000ec", Applet700000ED, "applet 700000ed",
    Applet700000F0, "applet 700000f0", Applet700000F1, "applet 700000f1",
    Applet700000F2, "applet 700000f2", Applet700000F3, "applet 700000f3",
    Applet700000F4, "applet 700000f4")

ENABLE_ENUM_FORMATTING(hydra::horizon::LibraryAppletMode, AllForeground,
                       "all foreground", PartialForeground,
                       "partial foreground", NoUi, "no UI",
                       PartialForegroundWithIndirectDisplay,
                       "partial foreground with indirect display",
                       AllForegroundInitiallyHidden,
                       "all foreground initially hidden")
