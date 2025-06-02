#pragma once

#include "common/type_aliases.hpp"

inline std::string utf16_to_utf8(const std::u16string& utf16_str) {
    std::string utf8_str;
    utf8_str.reserve(utf16_str.size() *
                     3); // Reserve space to avoid reallocations

    for (size_t i = 0; i < utf16_str.size(); ++i) {
        char32_t codepoint;
        char16_t unit = utf16_str[i];

        // Handle surrogate pairs
        if (unit >= 0xD800 && unit <= 0xDBFF) {
            // High surrogate
            if (i + 1 >= utf16_str.size()) {
                throw std::invalid_argument(
                    "Invalid UTF-16: unpaired high surrogate");
            }
            char16_t low = utf16_str[++i];
            if (low < 0xDC00 || low > 0xDFFF) {
                throw std::invalid_argument(
                    "Invalid UTF-16: invalid low surrogate");
            }
            codepoint = 0x10000 + ((unit & 0x3FF) << 10) + (low & 0x3FF);
        } else if (unit >= 0xDC00 && unit <= 0xDFFF) {
            throw std::invalid_argument(
                "Invalid UTF-16: unpaired low surrogate");
        } else {
            codepoint = unit;
        }

        // Convert codepoint to UTF-8
        if (codepoint <= 0x7F) {
            utf8_str.push_back(static_cast<char>(codepoint));
        } else if (codepoint <= 0x7FF) {
            utf8_str.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
            utf8_str.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        } else if (codepoint <= 0xFFFF) {
            utf8_str.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
            utf8_str.push_back(
                static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            utf8_str.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        } else if (codepoint <= 0x10FFFF) {
            utf8_str.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
            utf8_str.push_back(
                static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
            utf8_str.push_back(
                static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            utf8_str.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        } else {
            throw std::invalid_argument("Invalid Unicode codepoint");
        }
    }

    return utf8_str;
}

inline std::u16string utf8_to_utf16(const std::string& utf8_str) {
    std::u16string utf16_str;
    utf16_str.reserve(utf8_str.size()); // Reserve space to avoid reallocations

    for (size_t i = 0; i < utf8_str.size();) {
        char32_t codepoint = 0;
        unsigned char byte = static_cast<unsigned char>(utf8_str[i]);

        // Determine the number of bytes in this UTF-8 character
        if (byte <= 0x7F) {
            // 1-byte character (ASCII)
            codepoint = byte;
            i += 1;
        } else if ((byte & 0xE0) == 0xC0) {
            // 2-byte character
            if (i + 1 >= utf8_str.size()) {
                throw std::invalid_argument(
                    "Invalid UTF-8: incomplete 2-byte sequence");
            }
            unsigned char byte2 = static_cast<unsigned char>(utf8_str[i + 1]);
            if ((byte2 & 0xC0) != 0x80) {
                throw std::invalid_argument(
                    "Invalid UTF-8: invalid continuation byte");
            }
            codepoint = ((byte & 0x1F) << 6) | (byte2 & 0x3F);
            if (codepoint < 0x80) {
                throw std::invalid_argument("Invalid UTF-8: overlong encoding");
            }
            i += 2;
        } else if ((byte & 0xF0) == 0xE0) {
            // 3-byte character
            if (i + 2 >= utf8_str.size()) {
                throw std::invalid_argument(
                    "Invalid UTF-8: incomplete 3-byte sequence");
            }
            unsigned char byte2 = static_cast<unsigned char>(utf8_str[i + 1]);
            unsigned char byte3 = static_cast<unsigned char>(utf8_str[i + 2]);
            if ((byte2 & 0xC0) != 0x80 || (byte3 & 0xC0) != 0x80) {
                throw std::invalid_argument(
                    "Invalid UTF-8: invalid continuation byte");
            }
            codepoint =
                ((byte & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F);
            if (codepoint < 0x800) {
                throw std::invalid_argument("Invalid UTF-8: overlong encoding");
            }
            // Check for UTF-16 surrogate range (which is invalid in UTF-8)
            if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
                throw std::invalid_argument(
                    "Invalid UTF-8: surrogate codepoint");
            }
            i += 3;
        } else if ((byte & 0xF8) == 0xF0) {
            // 4-byte character
            if (i + 3 >= utf8_str.size()) {
                throw std::invalid_argument(
                    "Invalid UTF-8: incomplete 4-byte sequence");
            }
            unsigned char byte2 = static_cast<unsigned char>(utf8_str[i + 1]);
            unsigned char byte3 = static_cast<unsigned char>(utf8_str[i + 2]);
            unsigned char byte4 = static_cast<unsigned char>(utf8_str[i + 3]);
            if ((byte2 & 0xC0) != 0x80 || (byte3 & 0xC0) != 0x80 ||
                (byte4 & 0xC0) != 0x80) {
                throw std::invalid_argument(
                    "Invalid UTF-8: invalid continuation byte");
            }
            codepoint = ((byte & 0x07) << 18) | ((byte2 & 0x3F) << 12) |
                        ((byte3 & 0x3F) << 6) | (byte4 & 0x3F);
            if (codepoint < 0x10000) {
                throw std::invalid_argument("Invalid UTF-8: overlong encoding");
            }
            if (codepoint > 0x10FFFF) {
                throw std::invalid_argument(
                    "Invalid UTF-8: codepoint too large");
            }
            i += 4;
        } else {
            throw std::invalid_argument("Invalid UTF-8: invalid start byte");
        }

        // Convert codepoint to UTF-16
        if (codepoint <= 0xFFFF) {
            // Fits in a single UTF-16 code unit
            utf16_str.push_back(static_cast<char16_t>(codepoint));
        } else {
            // Needs a surrogate pair
            codepoint -= 0x10000;
            char16_t high_surrogate =
                static_cast<char16_t>(0xD800 + (codepoint >> 10));
            char16_t low_surrogate =
                static_cast<char16_t>(0xDC00 + (codepoint & 0x3FF));
            utf16_str.push_back(high_surrogate);
            utf16_str.push_back(low_surrogate);
        }
    }

    return utf16_str;
}
