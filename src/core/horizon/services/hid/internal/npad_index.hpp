#pragma once

#include "core/horizon/services/hid/const.hpp"

namespace hydra::horizon::services::hid::internal {

enum class NpadIndex {
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

inline NpadIndex toNpadIndex(NpadIdType type) {
    switch (type) {
    case NpadIdType::No1:
        return NpadIndex::No1;
    case NpadIdType::No2:
        return NpadIndex::No2;
    case NpadIdType::No3:
        return NpadIndex::No3;
    case NpadIdType::No4:
        return NpadIndex::No4;
    case NpadIdType::No5:
        return NpadIndex::No5;
    case NpadIdType::No6:
        return NpadIndex::No6;
    case NpadIdType::No7:
        return NpadIndex::No7;
    case NpadIdType::No8:
        return NpadIndex::No8;
    case NpadIdType::Handheld:
        return NpadIndex::Handheld;
    case NpadIdType::Other:
        return NpadIndex::Other;
    default:
        unreachable();
    }
}

inline NpadIdType toNpadIdType(NpadIndex index) {
    switch (index) {
    case NpadIndex::No1:
        return NpadIdType::No1;
    case NpadIndex::No2:
        return NpadIdType::No2;
    case NpadIndex::No3:
        return NpadIdType::No3;
    case NpadIndex::No4:
        return NpadIdType::No4;
    case NpadIndex::No5:
        return NpadIdType::No5;
    case NpadIndex::No6:
        return NpadIdType::No6;
    case NpadIndex::No7:
        return NpadIdType::No7;
    case NpadIndex::No8:
        return NpadIdType::No8;
    case NpadIndex::Handheld:
        return NpadIdType::Handheld;
    case NpadIndex::Other:
        return NpadIdType::Other;
    default:
        unreachable();
    }
}

} // namespace hydra::horizon::services::hid::internal

ENABLE_ENUM_FORMATTING(hydra::horizon::services::hid::internal::NpadIndex, No1,
                       "Number 1", No2, "Number 2", No3, "Number 3", No4,
                       "Number 4", No5, "Number 5", No6, "Number 6", No7,
                       "Number 7", No8, "Number 8", Handheld, "Handheld", Other,
                       "Other")
