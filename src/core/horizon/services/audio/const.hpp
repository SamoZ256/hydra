#pragma once

#include "core/audio/core.hpp"

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
    i32 unknown_x1c;
    u8 _unknown_x20;
    u8 padding1[3];
    i32 splitter_count;
    i32 _unknown_x2c;
    i32 _unknown_x30;
    u32 revision;
};

enum class AudioFeature {
    Splitter,
};

inline bool IsAudioRendererFeatureSupported(const AudioFeature feature,
                                            u32 revision) {
    const auto version = revision - make_magic4('R', 'E', 'V', '0');
    switch (feature) {
    case AudioFeature::Splitter:
        return version >= 2;
    }
}

} // namespace hydra::horizon::services::audio
