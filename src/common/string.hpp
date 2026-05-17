#pragma once

#include "common/type_aliases.hpp"

namespace hydra {

constexpr usize size_of_string(char value) {
    (void)value;
    return 1;
}

constexpr usize size_of_string(std::string_view value) { return value.size(); }

constexpr usize size_of_string(const std::string& value) {
    return value.size();
}

template <typename T, typename Delimiter>
std::vector<T> split(std::string_view s, Delimiter delimiter) {
    std::vector<T> tokens;
    usize pos = 0;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        std::string_view token = s.substr(0, pos);
        tokens.push_back(T(token));
        s = s.substr(pos + size_of_string(delimiter));
    }
    tokens.push_back(T(s));

    return tokens;
}

inline std::string utf16_to_utf8(const std::u16string& utf16_str) {
    std::string utf8_str;
    utf8_str.reserve(utf16_str.size() *
                     3); // Reserve space to avoid reallocations

    for (usize i = 0; i < utf16_str.size(); ++i) {
        char32_t codepoint;
        char16_t unit = utf16_str[i];

        // Handle surrogate pairs
        if (unit >= 0xd800 && unit <= 0xdbff) {
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
            codepoint = 0x10000u + ((unit & 0x3ffu) << 10) + (low & 0x3ffu);
        } else if (unit >= 0xdc00 && unit <= 0xdfff) {
            throw std::invalid_argument(
                "Invalid UTF-16: unpaired low surrogate");
        } else {
            codepoint = unit;
        }

        // Convert codepoint to UTF-8
        if (codepoint <= 0x7f) {
            utf8_str.push_back(static_cast<char>(codepoint));
        } else if (codepoint <= 0x7ff) {
            utf8_str.push_back(static_cast<char>(0xc0 | (codepoint >> 6)));
            utf8_str.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        } else if (codepoint <= 0xffff) {
            utf8_str.push_back(static_cast<char>(0xe0 | (codepoint >> 12)));
            utf8_str.push_back(
                static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
            utf8_str.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        } else if (codepoint <= 0x10ffff) {
            utf8_str.push_back(static_cast<char>(0xf0 | (codepoint >> 18)));
            utf8_str.push_back(
                static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
            utf8_str.push_back(
                static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
            utf8_str.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        } else {
            throw std::invalid_argument("Invalid Unicode codepoint");
        }
    }

    return utf8_str;
}

inline std::u16string utf8_to_utf16(const std::string& utf8_str) {
    std::u16string utf16_str;
    utf16_str.reserve(utf8_str.size()); // Reserve space to avoid reallocations

    for (usize i = 0; i < utf8_str.size();) {
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
            codepoint = ((byte & 0x1fu) << 6) | (byte2 & 0x3fu);
            if (codepoint < 0x80) {
                throw std::invalid_argument("Invalid UTF-8: overlong encoding");
            }
            i += 2;
        } else if ((byte & 0xf0) == 0xe0) {
            // 3-byte character
            if (i + 2 >= utf8_str.size()) {
                throw std::invalid_argument(
                    "Invalid UTF-8: incomplete 3-byte sequence");
            }
            unsigned char byte2 = static_cast<unsigned char>(utf8_str[i + 1]);
            unsigned char byte3 = static_cast<unsigned char>(utf8_str[i + 2]);
            if ((byte2 & 0xc0) != 0x80 || (byte3 & 0xc0) != 0x80) {
                throw std::invalid_argument(
                    "Invalid UTF-8: invalid continuation byte");
            }
            codepoint = ((byte & 0x0fu) << 12) | ((byte2 & 0x3fu) << 6) |
                        (byte3 & 0x3fu);
            if (codepoint < 0x800) {
                throw std::invalid_argument("Invalid UTF-8: overlong encoding");
            }
            // Check for UTF-16 surrogate range (which is invalid in UTF-8)
            if (codepoint >= 0xd800 && codepoint <= 0xdfff) {
                throw std::invalid_argument(
                    "Invalid UTF-8: surrogate codepoint");
            }
            i += 3;
        } else if ((byte & 0xf8) == 0xf0) {
            // 4-byte character
            if (i + 3 >= utf8_str.size()) {
                throw std::invalid_argument(
                    "Invalid UTF-8: incomplete 4-byte sequence");
            }
            unsigned char byte2 = static_cast<unsigned char>(utf8_str[i + 1]);
            unsigned char byte3 = static_cast<unsigned char>(utf8_str[i + 2]);
            unsigned char byte4 = static_cast<unsigned char>(utf8_str[i + 3]);
            if ((byte2 & 0xc0) != 0x80 || (byte3 & 0xc0) != 0x80 ||
                (byte4 & 0xc0) != 0x80) {
                throw std::invalid_argument(
                    "Invalid UTF-8: invalid continuation byte");
            }
            codepoint = ((byte & 0x07u) << 18) | ((byte2 & 0x3fu) << 12) |
                        ((byte3 & 0x3fu) << 6) | (byte4 & 0x3fu);
            if (codepoint < 0x10000) {
                throw std::invalid_argument("Invalid UTF-8: overlong encoding");
            }
            if (codepoint > 0x10ffff) {
                throw std::invalid_argument(
                    "Invalid UTF-8: codepoint too large");
            }
            i += 4;
        } else {
            throw std::invalid_argument("Invalid UTF-8: invalid start byte");
        }

        // Convert codepoint to UTF-16
        if (codepoint <= 0xffff) {
            // Fits in a single UTF-16 code unit
            utf16_str.push_back(static_cast<char16_t>(codepoint));
        } else {
            // Needs a surrogate pair
            codepoint -= 0x10000;
            char16_t high_surrogate =
                static_cast<char16_t>(0xd800 + (codepoint >> 10));
            char16_t low_surrogate =
                static_cast<char16_t>(0xdc00 + (codepoint & 0x3ff));
            utf16_str.push_back(high_surrogate);
            utf16_str.push_back(low_surrogate);
        }
    }

    return utf16_str;
}

} // namespace hydra
