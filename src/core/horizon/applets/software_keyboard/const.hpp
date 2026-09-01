#pragma once

#include "core/horizon/const.hpp"

namespace hydra::horizon::applets::software_keyboard {

enum class KeyboardMode : u32 {
    Full = 0,
    Numeric = 1,
    ASCII = 2,
    FullLatin = 3,
    Alphabet = 4,
    SimplifiedChinese = 5,
    TraditionalChinese = 6,
    Korean = 7,
    LanguageSet2 = 8,
    LanguageSet2Latin = 9,
};

enum class InvalidCharFlags : u32 {
    None = 0,
    Space = ZTD_BIT(1),
    AtMark = ZTD_BIT(2),
    Percent = ZTD_BIT(3),
    Slash = ZTD_BIT(4),
    BackSlash = ZTD_BIT(5),
    Numeric = ZTD_BIT(6),
    OutsideOfDownloadCode = ZTD_BIT(7),
    OutsideOfMiiNickName = ZTD_BIT(8),
};
ZTD_ENABLE_ENUM_BITWISE_OPERATORS(InvalidCharFlags)

enum class InitialCursorPosition : u32 {
    First = 0,
    Last = 1,
};

enum class PasswordMode : u32 {
    Show = 0,
    Hide = 1,
};

enum class InputFormMode : u32 {
    OneLine = 0,
    MultiLine = 1,
    Separate = 2,
};

enum class DictionaryLanguage : u16 {
    Japanese = 0,
    AmericanEnglish = 1,
    CanadianFrench = 2,
    LatinAmericanSpanish = 3,
    Reserved1 = 4,
    BritishEnglish = 5,
    French = 6,
    German = 7,
    Spanish = 8,
    Italian = 9,
    Dutch = 10,
    Portuguese = 11,
    Russian = 12,
    Reserved2 = 13,
    SimplifiedChinesePinyin = 14,
    TraditionalChineseCangjie = 15,
    TraditionalChineseSimplifiedCangjie = 16,
    TraditionalChineseZhuyin = 17,
    Korean = 18,
};

#pragma pack(push, 1)
struct KeyboardConfigCommon {
    KeyboardMode mode;
    char16_t ok_text[0x9];
    u16 left_optional_symbol_key;
    u16 right_optional_symbol_key;
    bool prediction_enabled;
    u8 _padding;
    InvalidCharFlags invalid_chars;
    InitialCursorPosition initial_cursor_pos;
    char16_t header_text[0x41];
    char16_t sub_text[0x81];
    char16_t guide_text[0x101];
    u32 max_text_len;
    u32 min_text_len;
    PasswordMode password_mode;
    InputFormMode input_form_mode;
    bool new_line_enabled;
    bool utf8_enabled;
    bool blur_background;
    u32 initial_string_offset;
    u32 initial_string_len;
    u32 user_dictionary_offset;
    u32 user_dictionary_count;
    bool text_check_enabled;
    u8 _reserved[3];
};

// 0x0 - 0x50009
struct KeyboardConfigAdditional_x0 {
    u8 _reserved[4];
    u64 text_check_callback;    // TODO: what type?
    u8 separate_text_pos[0x20]; // TODO: what is this?
};

// 0x6000b+
struct KeyboardConfigAdditional_x6000b {
    u8 separate_text_pos[0x20]; // TODO: what is this?
    struct {
        vaddr_t buffer_addr; // TODO: virtual address?
        u32 buffer_size;
        struct {
            u32 offset;
            u16 size;
            DictionaryLanguage lang;
        } infos[4];
        u16 count;
    } customized_dicts;
    u8 customized_dict_count;
    bool cancel_button_disabled; // 8.0.0+
    u8 _reserved1[0xd];
    u8 trigger; // 8.0.0+ (TODO: what is this?)
    u8 _reserved2[0x4];
};
#pragma pack(pop)

enum class SoftwareKeyboardResult : u32 {
    OK = 0,
    Cancel = 1,
};

} // namespace hydra::horizon::applets::software_keyboard
