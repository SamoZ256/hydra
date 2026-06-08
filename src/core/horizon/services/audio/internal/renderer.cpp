#include "core/horizon/services/audio/internal/renderer.hpp"

#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::horizon::services::audio::internal {

namespace {

#pragma pack(push, 1)
struct BehaviorInfoIn {
    u32 revision;
    u32 _padding1;
    u64 flags;
};

enum class MemPoolState : u32 {
    Invalid,
    New,
    RequestDetach,
    Detached,
    RequestAttach,
    Attached,
    Released,
};

struct MemPoolInfoIn {
    vaddr_t address;
    u64 size;
    MemPoolState state;
    u32 _padding[3];
};

struct MemPoolInfoOut {
    MemPoolState new_state;
    u32 _padding[3];
};

enum class VoicePlayState : u8 {
    Started = 0,
    Stopped = 1,
    Paused = 2,
};

struct BiquadFilter {
    bool enable;
    u8 _padding;
    i16 numerator[3];
    i16 denominator[2];
};

struct WaveBuffer {
    vaddr_t address;
    u64 size;
    u32 start_sample_offset;
    u32 end_sample_offset;
    bool is_looping;
    bool end_of_stream;
    bool sent_to_server;
    u8 _padding1[5];
    vaddr_t context_addr;
    u64 context_sz;
    u64 _padding2;
};

struct VoiceInfoIn {
    u32 id;
    u32 node_id;
    bool is_new;
    bool is_used;
    VoicePlayState play_state;
    PcmFormat sample_format : 8;
    u32 sample_rate;
    u32 priority;
    u32 sorting_order;
    u32 channel_count;
    f32 pitch;
    f32 volume;
    BiquadFilter biquads[2];
    u32 wave_buffer_count;
    i16 wave_buffer_head;
    u16 _padding1;
    u32 _padding2;
    const void* extra_params_ptr;
    u64 extra_params_sz;
    u32 dest_mix_id;
    u32 dest_splitter_id;
    WaveBuffer wave_buffers[4];
    u32 channel_ids[6];
    u8 _padding3[24];
};

struct VoiceInfoOut {
    u64 played_sample_count;
    u32 num_wave_buffers_consumed;
    u32 voice_drops_count;
};

enum class EffectState : u8 {
    Enabled = 3,
    Disabled = 4,
};

struct EffectInfoOutV1 {
    EffectState state;
    u8 _reserved[15];
};

struct MixInfoIn {
    float volume;
    u32 sample_rate;
    u32 buffer_count;
    bool is_used;
    u8 _padding1[3];
    u32 mix_id;
    u32 _padding2;
    u32 node_id;
    u32 _padding3[2];
    float mix[24][24]; // [src_index][dest_index]
    u32 dest_mix_id;
    u32 dest_splitter_id;
    u32 _padding4;
};

struct DownMixParameters {
    u8 coefficients[16];
};

enum class SinkType : u8 {
    Invalid = 0,
    Device = 1,
    CircularBuffer = 2,
};

struct DeviceSinkInfoIn {
    char name[255];
    u8 _padding1;
    u32 input_count;
    u8 inputs[6];
    u8 _padding2;
    bool downmix_params_enabled;
    DownMixParameters downmix_params;
};

struct CircularBufferSinkInfoIn {
    void* buffer_ptr;
    u32 buffer_size;
    u32 input_count;
    u32 sample_count;
    u32 last_read_offset;
    PcmFormat sample_format;
    u8 inputs[6];
    u8 _padding2[6];
};

struct SinkInfoIn {
    SinkType type;
    bool is_used;
    u8 _padding1[2];
    u32 node_id;
    u64 _padding2[3];
    union {
        DeviceSinkInfoIn device_sink;
        CircularBufferSinkInfoIn circular_buffer_sink;
    };
};

struct SinkInfoOut {
    u32 last_written_offset;
    u32 _padding;
    u64 _reserved[3];
};

struct PerformanceBufferInfoIn {
    u32 detail_target;
    u32 _padding1[3];
};

struct PerformanceBufferInfoOut {
    u32 history_size;
    u32 _reserved[3];
};

struct ErrorInfo {
    result_t result;
    u32 _padding;
    u64 extra_error_info;
};

struct BehaviorInfoOut {
    ErrorInfo error_infos[10];
    u32 error_info_count;
    u32 _reserved[3];
};

struct RenderInfoOut {
    u64 elapsed_frame_count;
    u64 _reserved;
};
#pragma pack(pop)

} // namespace

} // namespace hydra::horizon::services::audio::internal

ENABLE_ENUM_FORMATTING(hydra::horizon::services::audio::internal::MemPoolState,
                       Invalid, "invalid", New, "new", RequestDetach,
                       "request detach", Detached, "detached", RequestAttach,
                       "request attach", Attached, "attached", Released,
                       "released")

namespace hydra::horizon::services::audio::internal {

Renderer::Renderer(const AudioRendererParameters& params,
                   std::span<u8> work_buffer_)
    : work_buffer{work_buffer_} {
    mempools.resize(params.effect_count + 4 * params.voice_count);
    voice_count = params.voice_count;
    effect_count = params.effect_count;
    sink_count = params.sink_count;
}

result_t Renderer::UpdateBehaviorInfoIn(io::MemoryStream* in_stream) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    const auto info_in = in_stream->Read<BehaviorInfoIn>();
    (void)info_in;

    return RESULT_SUCCESS;
}

result_t Renderer::UpdateMemoryPools(io::MemoryStream* in_stream,
                                     hw::tegra_x1::cpu::IMmu* mmu,
                                     UpdateDataHeader& out_header,
                                     io::MemoryStream* out_stream) {
    for (u32 i = 0; i < mempools.size(); i++) {
        auto& mempool = mempools[i];
        const auto mempool_in = in_stream->Read<MemPoolInfoIn>();

        auto state = mempool_in.state;
        if (state == MemPoolState::RequestAttach) {
            if (!mempool.Map(std::span(
                    reinterpret_cast<u8*>(mmu->UnmapAddr(mempool_in.address)),
                    mempool_in.size)))
                return MAKE_RESULT(Audio, 1); // TODO
            state = MemPoolState::Attached;
        } else if (state == MemPoolState::RequestDetach) {
            if (!mempool.Unmap())
                return MAKE_RESULT(Audio, 2); // TODO
            state = MemPoolState::Detached;
        }

        out_stream->Write<MemPoolInfoOut>({.new_state = state});
    }

    out_header.mempools_size =
        static_cast<u32>(mempools.size()) * sizeof(MemPoolInfoOut);
    out_header.total_size += out_header.mempools_size;
    return RESULT_SUCCESS;
}

result_t Renderer::UpdateChannels(io::MemoryStream* in_stream,
                                  UpdateDataHeader& out_header,
                                  io::MemoryStream* out_stream) {
    LOG_FUNC_STUBBED(Services);

    // TODO
    (void)in_stream;
    (void)out_stream;

    out_header.channels_size = 0x0;
    out_header.total_size += out_header.channels_size;
    return RESULT_SUCCESS;
}

result_t Renderer::UpdateVoices(io::MemoryStream* in_stream,
                                UpdateDataHeader& out_header,
                                io::MemoryStream* out_stream) {
    LOG_FUNC_STUBBED(Services);

    // TODO
    for (u32 i = 0; i < voice_count; i++) {
        const auto voice_in = in_stream->Read<VoiceInfoIn>();

        VoiceInfoOut voice{}; // = voices[i];
        if (voice_in.is_new) {
            voice.played_sample_count = 0;
            voice.num_wave_buffers_consumed = 0;
        } else if (voice_in.play_state == VoicePlayState::Started) {
            // HACK
            if (voice_in.wave_buffer_count >=
                sizeof_array(voice_in.wave_buffers)) {
                ONCE(LOG_WARN(Services,
                              "Voice {} has invalid wave buffer count {:#x}", i,
                              voice_in.wave_buffer_count));
                continue;
            }

            for (u32 j = 0; j < voice_in.wave_buffer_count; j++) {
                voice.played_sample_count +=
                    (voice_in.wave_buffers[j].end_sample_offset -
                     voice_in.wave_buffers[j].start_sample_offset) /
                    2;
                voice.num_wave_buffers_consumed++;
            }
        } else {
            ONCE(LOG_NOT_IMPLEMENTED(Services, "Voice"));
        }
        out_stream->Write(voice);
    }

    out_header.voices_size = voice_count * sizeof(VoiceInfoOut);
    out_header.total_size += out_header.voices_size;
    return RESULT_SUCCESS;
}

result_t Renderer::UpdateEffects(io::MemoryStream* in_stream,
                                 UpdateDataHeader& out_header,
                                 io::MemoryStream* out_stream) {
    LOG_FUNC_STUBBED(Services);

    // TODO
    (void)in_stream;
    if (false) {
        // TODO

        // header->effects_size = TODO;
    } else {
        for (u32 i = 0; i < effect_count; i++) {
            out_stream->Write<EffectInfoOutV1>({
                .state = EffectState::Disabled,
            });
        }

        out_header.effects_size = effect_count * sizeof(EffectInfoOutV1);
    }

    out_header.total_size += out_header.effects_size;
    return RESULT_SUCCESS;
}

result_t Renderer::UpdateMixes(io::MemoryStream* in_stream,
                               UpdateDataHeader& out_header,
                               io::MemoryStream* out_stream) {
    LOG_FUNC_STUBBED(Services);

    // TODO
    (void)in_stream;
    (void)out_stream;

    out_header.mixes_size = 0x0;
    out_header.total_size += out_header.mixes_size;
    return RESULT_SUCCESS;
}

result_t Renderer::UpdateSinks(io::MemoryStream* in_stream,
                               UpdateDataHeader& out_header,
                               io::MemoryStream* out_stream) {
    LOG_FUNC_STUBBED(Services);

    // TODO
    (void)in_stream;
    for (u32 i = 0; i < sink_count; i++) {
        out_stream->Write<SinkInfoOut>({
            .last_written_offset = 0,
        });
    }

    out_header.sinks_size = sink_count * sizeof(SinkInfoOut);
    out_header.total_size += out_header.sinks_size;
    return RESULT_SUCCESS;
}

result_t Renderer::UpdatePerformanceBuffer(io::MemoryStream* in_stream,
                                           UpdateDataHeader& out_header,
                                           io::MemoryStream* out_stream,
                                           io::MemoryStream* out_perf_stream) {
    const auto info_in = in_stream->Read<PerformanceBufferInfoIn>();

    auto& info_out = out_stream->WriteReturningRef<PerformanceBufferInfoOut>();
    info_out.history_size = 0;
    if (out_perf_stream) {
        // TODO
        (void)info_in;
        LOG_NOT_IMPLEMENTED(Services, "Performance info");
    }

    out_header.performance_manager_size = sizeof(PerformanceBufferInfoOut);
    out_header.total_size += out_header.performance_manager_size;
    return RESULT_SUCCESS;
}

result_t Renderer::UpdateBehaviorInfoOut(UpdateDataHeader& out_header,
                                         io::MemoryStream* out_stream) {
    LOG_FUNC_STUBBED(Services);

    // TODO
    out_stream->Write<BehaviorInfoOut>({
        .error_info_count = 0,
    });

    out_header.behavior_size = sizeof(BehaviorInfoOut);
    out_header.total_size += out_header.behavior_size;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::audio::internal
