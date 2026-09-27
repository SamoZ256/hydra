#pragma once

namespace hydra::horizon::filesystem::patch {

constexpr usize BUILD_ID_SIZE = 0x20;
constexpr usize BUILD_ID_MATCH_SIZE = 0x8;
constexpr u32 NSO_HEADER_SIZE = 0x100;

constexpr std::string_view PCHTXT_EXTENSION = ".pchtxt";
constexpr std::string_view IPS_EXTENSION = ".ips";

using BuildId = std::array<u8, BUILD_ID_SIZE>;

struct Record {
    u32 offset;
    std::vector<u8> data;
};

struct Block {
    std::string name;
    bool enabled;
    std::vector<Record> records;
};

inline bool buildIdsMatch(const BuildId& lhs, const BuildId& rhs) {
    return std::equal(lhs.begin(), lhs.begin() + BUILD_ID_MATCH_SIZE,
                      rhs.begin());
}

} // namespace hydra::horizon::filesystem::patch
