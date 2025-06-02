#pragma once

namespace hydra::horizon::services::ncm {

enum class StorageID : u8 {
    None = 0,
    Host = 1,
    GameCard = 2,
    BuiltInSystem = 3,
    BuiltInUser = 4,
    SdCard = 5,
    Any = 6,
};

}

ENABLE_ENUM_FORMATTING(hydra::horizon::services::ncm::StorageID, None, "none",
                       Host, "host", GameCard, "game card", BuiltInSystem,
                       "built-in system", BuiltInUser, "built-in user", SdCard,
                       "SD card", Any, "any")
