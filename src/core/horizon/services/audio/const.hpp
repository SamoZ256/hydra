#pragma once

#include "core/audio/core_base.hpp"

namespace hydra::horizon::services::audio {

using namespace ::hydra::audio;

enum class AudioOutState : u32 {
    Started = 0,
    Stopped = 1,
};

struct AudioRendererParameters {
    i32 sample_rate;
    i32 sample_count;
    i32 mix_buffer_count;
    i32 submix_count;
    i32 voice_count;
    i32 sink_count;
    i32 effect_count;
    i32 unk1;
    u8 unk2;
    u8 padding1[3];
    i32 splitter_count;
    i32 unk3;
    i32 unk4;
    u32 revision;
};

} // namespace hydra::horizon::services::audio
