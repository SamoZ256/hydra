#pragma once

namespace hydra::audio {

enum class PcmFormat : u32 {
    Invalid = 0,
    Int8 = 1,
    Int16 = 2,
    Int24 = 3,
    Int32 = 4,
    Float = 5,
    Adpcm = 6,
};

enum class StreamState : u32 {
    Started,
    Stopped,
    Drained,
    Error,
};

} // namespace hydra::audio

ENABLE_ENUM_FORMATTING(hydra::audio::PcmFormat, Invalid, "invalid", Int8,
                       "int8", Int16, "int16", Int24, "int24", Int32, "int32",
                       Float, "float", Adpcm, "ADPCM")
