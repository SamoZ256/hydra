#pragma once

namespace hydra::horizon::services::pl {

enum class SharedFontType : u32 {
    JapanUsEurope = 0,
    ChineseSimplified = 1,
    ExtendedChineseSimplified = 2,
    ChineseTraditional = 3,
    Korean = 4,
    NintendoExtended = 5,
};
ENABLE_ENUM_ARITHMETIC_OPERATORS(SharedFontType)

enum class LoadState : u32 {
    Loading = 0,
    Loaded = 1,
};

} // namespace hydra::horizon::services::pl

ENABLE_ENUM_FORMATTING(hydra::horizon::services::pl::SharedFontType,
                       JapanUsEurope, "Japan/US/Europe", ChineseSimplified,
                       "Chinese Simplified", ExtendedChineseSimplified,
                       "Extended Chinese Simplified", ChineseTraditional,
                       "Chinese Traditional", Korean, "Korean",
                       NintendoExtended, "Nintendo Extended")

ENABLE_ENUM_FORMATTING(hydra::horizon::services::pl::LoadState, Loading,
                       "Loading", Loaded, "Loaded")
