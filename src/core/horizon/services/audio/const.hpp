#pragma once

#include "core/audio/core.hpp"

namespace hydra::horizon::services::audio {

using namespace ::hydra::audio;

enum class AudioOutState : u32 {
    Started = 0,
    Stopped = 1,
};

struct AudioRendererParameters {
    u32 sample_rate;
    u32 sample_count;
    u32 mix_buffer_count;
    u32 submix_count;
    u32 voice_count;
    u32 sink_count;
    u32 effect_count;
    u32 unknown_x1c;
    u8 _unknown_x20;
    u8 padding1[3];
    u32 splitter_count;
    u32 _unknown_x2c;
    u32 _unknown_x30;
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

struct UpdateDataHeader {
    u32 revision;
    u32 behavior_size;
    u32 mempools_size;
    u32 voices_size;
    u32 channels_size;
    u32 effects_size;
    u32 mixes_size;
    u32 sinks_size;
    u32 performance_manager_size;
    u32 _unknown;
    u32 render_info_size;
    u32 _reserved[4];
    u32 total_size;
};

} // namespace hydra::horizon::services::audio
