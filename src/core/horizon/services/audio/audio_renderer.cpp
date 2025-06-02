#include "core/horizon/services/audio/audio_renderer.hpp"

namespace hydra::horizon::services::audio {

namespace {

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
    i32 start_sample_offset;
    i32 end_sample_offset;
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
    float pitch;
    float volume;
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

enum class EffectState : u8 {
    Enabled = 3,
    Disabled = 4,
};

struct EffectInfoOutV1 {
    EffectState state;
    u8 _reserved[15];
};

struct SinkInfoOut {
    u32 last_written_offset;
    u32 _padding;
    u64 _reserved[3];
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

struct PerformanceInfoOut {
    u32 history_size;
    u32 _reserved[3];
};

} // namespace

} // namespace hydra::horizon::services::audio

ENABLE_ENUM_FORMATTING(hydra::horizon::services::audio::MemPoolState, Invalid,
                       "invalid", New, "new", RequestDetach, "request detach",
                       Detached, "detached", RequestAttach, "request attach",
                       Attached, "attached", Released, "released")

namespace hydra::horizon::services::audio {

DEFINE_SERVICE_COMMAND_TABLE(IAudioRenderer, 4, RequestUpdate, 5, Start, 6,
                             Stop, 7, QuerySystemEvent)

// TODO: autoclear event?
IAudioRenderer::IAudioRenderer(const AudioRendererParameters& params_,
                               const usize work_buffer_size_)
    : params{params_}, work_buffer_size{work_buffer_size_},
      event(new kernel::Event()) {
    voices.resize(params.voice_count);

    // HACK: create a thread that signals the handle every so often
    auto t = new std::thread([&]() {
        while (true) {
            event.handle->Signal();
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    });
}

result_t
IAudioRenderer::RequestUpdate(InBuffer<BufferAttr::MapAlias> in_buffer,
                              OutBuffer<BufferAttr::MapAlias> out_buffer,
                              OutBuffer<BufferAttr::MapAlias> out_perf_buffer) {
    ONCE(LOG_FUNC_STUBBED(Services));

    auto& reader = *in_buffer.reader;

    auto& writer = *out_buffer.writer;

    // Header
    const auto in_header = reader.Read<UpdateDataHeader>();

    // TODO: correct?
    auto header = writer.WritePtr<UpdateDataHeader>();
    header->revision = in_header.revision; // make_magic4('R', 'E', 'V', '4');
    header->total_size = sizeof(UpdateDataHeader);

    reader.Skip(in_header.behavior_size);

    // Mempools
    u32 mempool_count = (params.effect_count + params.voice_count * 4);
    header->mempools_size = mempool_count * sizeof(MemPoolInfoOut);
    header->total_size += header->mempools_size;
    for (u32 i = 0; i < mempool_count; i++) {
        const auto mempool_in = reader.Read<MemPoolInfoIn>();

        MemPoolInfoOut mempool;
        switch (mempool_in.state) {
        case MemPoolState::RequestAttach:
            mempool.new_state = MemPoolState::Attached;
            break;
        case MemPoolState::RequestDetach:
            mempool.new_state = MemPoolState::Detached;
            break;
        default:
            ONCE(LOG_NOT_IMPLEMENTED(Services, "Memory pool state {}",
                                     mempool_in.state));
            mempool.new_state = MemPoolState::Released; // mempool_in.state;
            break;
        }
        writer.Write(mempool);
    }

    // Voices
    header->voices_size = params.voice_count * sizeof(VoiceInfoOut);
    header->total_size += header->voices_size;
    for (u32 i = 0; i < params.voice_count; i++) {
        const auto voice_in = reader.Read<VoiceInfoIn>();

        VoiceInfoOut* voice = &voices[i];
        if (voice_in.is_new) {
            voice->played_sample_count = 0;
            voice->num_wave_buffers_consumed = 0;
        } else if (voice_in.play_state == VoicePlayState::Started) {
            for (u32 j = 0; j < voice_in.wave_buffer_count; j++) {
                voice->played_sample_count +=
                    (voice_in.wave_buffers[j].end_sample_offset -
                     voice_in.wave_buffers[j].start_sample_offset) /
                    2;
                voice->num_wave_buffers_consumed++;
            }
        } else {
            ONCE(LOG_NOT_IMPLEMENTED(Services, "Voice"));
        }
        writer.Write(*voice);
    }

    // Channels
    header->channels_size = 0x0;
    header->total_size += header->channels_size;
    // TODO

    // Effects
    if (false) {
        // header->effects_size = TODO;
        // TODO
    } else {
        header->effects_size = params.effect_count * sizeof(EffectInfoOutV1);
        for (u32 i = 0; i < params.effect_count; i++) {
            writer.Write<EffectInfoOutV1>({
                .state = EffectState::Enabled,
            });
        }
    }
    header->total_size += header->effects_size;

    // Sinks
    header->sinks_size = params.sink_count * sizeof(SinkInfoOut);
    header->total_size += header->sinks_size;
    for (u32 i = 0; i < params.sink_count; i++) {
        writer.Write<SinkInfoOut>({
            .last_written_offset = 0,
        });
    }

    // Behavior
    header->behavior_size = sizeof(BehaviorInfoOut);
    header->total_size += header->behavior_size;
    writer.Write<BehaviorInfoOut>({
        .error_info_count = 0,
    });

    // Render info
    // TODO: if elapsed frame count supported
    if (false) {
        header->render_info_size = sizeof(RenderInfoOut);
        header->total_size += header->render_info_size;
        writer.Write<RenderInfoOut>({
            .elapsed_frame_count = 0,
        });
    }

    // Performance
    header->performance_manager_size = sizeof(PerformanceInfoOut);
    header->total_size += header->performance_manager_size;
    // HACK
    if (out_perf_buffer.writer->IsValid()) {
        out_perf_buffer.writer->Write<PerformanceInfoOut>({
            .history_size = 0,
        });
    }

    return RESULT_SUCCESS;
}

result_t
IAudioRenderer::QuerySystemEvent(OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = event.id;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::audio
