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
    .major = 9,
    .minor = 0,
    .micro = 0,
    .revision_major = 0,
    .revision_minor = 0,
    .platform = "NX",
    .version_hash = "voyp5gq7m551zuqgspcgobbmo74rg6yydpalt72l",
    .display_version = "9.0.0",
    .display_title = "Hydra firmware 9.0.0",
};

enum class LanguageCode : u64 {
    Japanese = "ja"_u64,
    AmericanEnglish = "en-US"_u64,
    French = "fr"_u64,
    German = "de"_u64,
    Italian = "it"_u64,
    Spanish = "es"_u64,
    Chinese = "zh-CN"_u64,
    Korean = "ko"_u64,
    Dutch = "nl"_u64,
    Portuguese = "pt"_u64,
    Russian = "ru"_u64,
    Taiwanese = "zh-TW"_u64,
    BritishEnglish = "en-GB"_u64,
    CanadianFrench = "fr-CA"_u64,
    LatinAmericanSpanish = "es-419"_u64,
    SimplifiedChinese = "zh-Hans"_u64,
    TraditionalChinese = "zh-Hant"_u64,
    BrazilianPortuguese = "pt-BR"_u64,
};

inline LanguageCode ToLanguageCode(SystemLanguage lang) {
    switch (lang) {
    case SystemLanguage::AmericanEnglish:
        return LanguageCode::AmericanEnglish;
    case SystemLanguage::BritishEnglish:
        return LanguageCode::BritishEnglish;
    case SystemLanguage::Japanese:
        return LanguageCode::Japanese;
    case SystemLanguage::French:
        return LanguageCode::French;
    case SystemLanguage::German:
        return LanguageCode::German;
    case SystemLanguage::LatinAmericanSpanish:
        return LanguageCode::LatinAmericanSpanish;
    case SystemLanguage::Spanish:
        return LanguageCode::Spanish;
    case SystemLanguage::Italian:
        return LanguageCode::Italian;
    case SystemLanguage::Dutch:
        return LanguageCode::Dutch;
    case SystemLanguage::CanadianFrench:
        return LanguageCode::CanadianFrench;
    case SystemLanguage::Portuguese:
        return LanguageCode::Portuguese;
    case SystemLanguage::Russian:
        return LanguageCode::Russian;
    case SystemLanguage::Korean:
        return LanguageCode::Korean;
    case SystemLanguage::TraditionalChinese:
        return LanguageCode::TraditionalChinese; // TODO: return Chinese on
                                                 // pre-4.0.0?
    case SystemLanguage::SimplifiedChinese:
        return LanguageCode::SimplifiedChinese; // TODO: return Chinese on
                                                // pre-4.0.0?
    case SystemLanguage::BrazilianPortuguese:
        return LanguageCode::BrazilianPortuguese;
    case SystemLanguage::Polish:
    case SystemLanguage::Thai:
        return LanguageCode::AmericanEnglish; // No equivalent
    }
}

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
    LanguageCode::BrazilianPortuguese,
};

// TODO: rename some values
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
