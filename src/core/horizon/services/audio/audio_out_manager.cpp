#include "core/horizon/services/audio/audio_out_manager.hpp"

#include "core/horizon/services/audio/audio_out.hpp"

namespace hydra::horizon::services::audio {

DEFINE_SERVICE_COMMAND_TABLE(IAudioOutManager, 0, ListAudioOuts, 1,
                             OpenAudioOut, 2, ListAudioOutsAuto, 3,
                             OpenAudioOutAuto)

result_t
IAudioOutManager::ListAudioOuts(u32* out_count,
                                OutBuffer<BufferAttr::MapAlias> out_buffer) {
    return ListAudioOutsImpl(out_count, out_buffer.stream);
}

result_t IAudioOutManager::OpenAudioOut(
    RequestContext* ctx, u32 sample_rate, u16 channel_count,
    [[maybe_unused]] u16 _reserved, u64 aruid,
    InBuffer<BufferAttr::MapAlias> in_device_name_buffer, u32* out_sample_rate,
    u32* out_channel_count, PcmFormat* out_format, AudioOutState* out_state,
    OutBuffer<BufferAttr::MapAlias> out_device_name_buffer) {
    return OpenAudioOutImpl(ctx, sample_rate, channel_count, aruid,
                            in_device_name_buffer.stream, out_sample_rate,
                            out_channel_count, out_format, out_state,
                            out_device_name_buffer.stream);
}

result_t IAudioOutManager::ListAudioOutsAuto(
    u32* out_count, OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    return ListAudioOutsImpl(out_count, out_buffer.stream);
}

result_t IAudioOutManager::OpenAudioOutAuto(
    RequestContext* ctx, u32 sample_rate, u16 channel_count,
    [[maybe_unused]] u16 _reserved, u64 aruid,
    InBuffer<BufferAttr::AutoSelect> in_device_name_buffer,
    u32* out_sample_rate, u32* out_channel_count, PcmFormat* out_format,
    AudioOutState* out_state,
    OutBuffer<BufferAttr::AutoSelect> out_device_name_buffer) {
    return OpenAudioOutImpl(ctx, sample_rate, channel_count, aruid,
                            in_device_name_buffer.stream, out_sample_rate,
                            out_channel_count, out_format, out_state,
                            out_device_name_buffer.stream);
}

result_t IAudioOutManager::ListAudioOutsImpl(u32* out_count,
                                             io::MemoryStream* out_stream) {
    (void)out_stream;

    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_count = 0;
    return RESULT_SUCCESS;
}

result_t IAudioOutManager::OpenAudioOutImpl(
    RequestContext* ctx, u32 sample_rate, u16 channel_count, u64 aruid,
    io::MemoryStream* in_device_name_stream, u32* out_sample_rate,
    u32* out_channel_count, PcmFormat* out_format, AudioOutState* out_state,
    io::MemoryStream* out_device_name_stream) {
    (void)aruid;

    [[maybe_unused]] const auto device_name_in =
        in_device_name_stream->ReadNullTerminatedString();
    LOG_DEBUG(Services, "Sample rate: {}, channel count: {}, device name: {}",
              sample_rate, channel_count, device_name_in);

    // Out
    // TODO: check this
    if (sample_rate == 0)
        sample_rate = 48000;
    if (channel_count <= 2)
        channel_count = 2;
    else
        channel_count = 6;
    const auto format = PcmFormat::Int16;

    *out_sample_rate = sample_rate;
    *out_channel_count = channel_count;
    *out_format = format;
    *out_state = AudioOutState::Stopped;

    // TODO: is this somehow connected to device name in?
    std::string device_name_out = "Hydra audio device";
    out_device_name_stream->WriteNullTerminatedString(device_name_out);

    AddService(*ctx,
               new IAudioOut(ctx->system, format, sample_rate, channel_count));
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::audio
