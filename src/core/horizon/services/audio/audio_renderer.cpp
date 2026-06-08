#include "core/horizon/services/audio/audio_renderer.hpp"

#include "core/debugger/debugger_manager.hpp"
#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::audio {

namespace {

#pragma pack(push, 1)
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
#pragma pack(pop)

} // namespace

} // namespace hydra::horizon::services::audio

ENABLE_ENUM_FORMATTING(hydra::horizon::services::audio::MemPoolState, Invalid,
                       "invalid", New, "new", RequestDetach, "request detach",
                       Detached, "detached", RequestAttach, "request attach",
                       Attached, "attached", Released, "released")

namespace hydra::horizon::services::audio {

DEFINE_SERVICE_COMMAND_TABLE(IAudioRenderer, 4, RequestUpdate, 5, Start, 6,
                             Stop, 7, QuerySystemEvent, 8,
                             SetRenderingTimeLimit, 9, GetRenderingTimeLimit,
                             10, RequestUpdateAuto)

IAudioRenderer::IAudioRenderer(const AudioRendererParameters& params_,
                               std::span<u8> work_buffer_)
    : params{params_}, work_buffer{work_buffer_}, event{new kernel::Event(
                                                      false,
                                                      "IAudioRenderer event")} {
    mempools.resize(params.effect_count + 4 * params.voice_count);
    // voices.resize(params.voice_count);

    // HACK: create a thread that signals the handle every so often
    new std::thread([&]() {
        GET_CURRENT_PROCESS_DEBUGGER().RegisterThisThread("Audren signal");
        while (true) {
            event->Signal();
            std::this_thread::sleep_for(std::chrono::microseconds(2));
        }
        GET_CURRENT_PROCESS_DEBUGGER().UnregisterThisThread();
    });
}

result_t
IAudioRenderer::RequestUpdate(kernel::Process* process,
                              InBuffer<BufferAttr::MapAlias> in_buffer,
                              OutBuffer<BufferAttr::MapAlias> out_buffer,
                              OutBuffer<BufferAttr::MapAlias> out_perf_buffer) {
    return RequestUpdateImpl(process, in_buffer.stream, out_buffer.stream,
                             out_perf_buffer.stream);
}

result_t
IAudioRenderer::QuerySystemEvent(kernel::Process* process,
                                 OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(event);
    return RESULT_SUCCESS;
}

result_t IAudioRenderer::SetRenderingTimeLimit(u32 time_limit) {
    rendering_time_limit = time_limit;
    return RESULT_SUCCESS;
}

result_t IAudioRenderer::GetRenderingTimeLimit(u32* out_time_limit) {
    *out_time_limit = rendering_time_limit;
    return RESULT_SUCCESS;
}

result_t IAudioRenderer::RequestUpdateAuto(
    kernel::Process* process, InBuffer<BufferAttr::AutoSelect> in_buffer,
    OutBuffer<BufferAttr::AutoSelect> out_buffer,
    OutBuffer<BufferAttr::AutoSelect> out_perf_buffer) {
    return RequestUpdateImpl(process, in_buffer.stream, out_buffer.stream,
                             out_perf_buffer.stream);
}

result_t IAudioRenderer::RequestUpdateImpl(kernel::Process* process,
                                           io::MemoryStream* in_stream,
                                           io::MemoryStream* out_stream,
                                           io::MemoryStream* out_perf_stream) {
    const auto mmu = process->GetMmu();

    // Header
    const auto in_header = in_stream->Read<UpdateDataHeader>();

    // TODO: correct?
    auto header = out_stream->WriteReturningPtr<UpdateDataHeader>();
    header->revision = in_header.revision; // make_magic4('R', 'E', 'V', '4');
    header->total_size = sizeof(UpdateDataHeader);

    in_stream->SeekBy(in_header.behavior_size);

    // Mempools
    header->mempools_size =
        static_cast<u32>(mempools.size()) * sizeof(MemPoolInfoOut);
    header->total_size += header->mempools_size;
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

    // Voices
    // TODO
    header->voices_size = params.voice_count * sizeof(VoiceInfoOut);
    header->total_size += header->voices_size;
    for (u32 i = 0; i < params.voice_count; i++) {
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

    // Channels
    header->channels_size = 0x0;
    header->total_size += header->channels_size;
    // TODO

    // Effects
    // TODO
    if (false) {
        // header->effects_size = TODO;
        // TODO
    } else {
        header->effects_size = params.effect_count * sizeof(EffectInfoOutV1);
        for (u32 i = 0; i < params.effect_count; i++) {
            out_stream->Write<EffectInfoOutV1>({
                .state = EffectState::Enabled,
            });
        }
    }
    header->total_size += header->effects_size;

    // Mixes
    // TODO

    // Sinks
    header->sinks_size = params.sink_count * sizeof(SinkInfoOut);
    header->total_size += header->sinks_size;
    for (u32 i = 0; i < params.sink_count; i++) {
        out_stream->Write<SinkInfoOut>({
            .last_written_offset = 0,
        });
    }

    // Behavior
    header->behavior_size = sizeof(BehaviorInfoOut);
    header->total_size += header->behavior_size;
    out_stream->Write<BehaviorInfoOut>({
        .error_info_count = 0,
    });

    // Render info
    // TODO: if elapsed frame count supported
    if (false) {
        header->render_info_size = sizeof(RenderInfoOut);
        header->total_size += header->render_info_size;
        out_stream->Write<RenderInfoOut>({
            .elapsed_frame_count = 0,
        });
    }

    // Performance
    header->performance_manager_size = sizeof(PerformanceInfoOut);
    header->total_size += header->performance_manager_size;
    // HACK
    if (out_perf_stream) {
        out_perf_stream->Write<PerformanceInfoOut>({
            .history_size = 0,
        });
    }

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::audio
