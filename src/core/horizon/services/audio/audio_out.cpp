#include "core/horizon/services/audio/audio_out.hpp"

#include "core/horizon/os.hpp"
#include "core/hw/tegra_x1/cpu/mmu_base.hpp"

namespace hydra::horizon::services::audio {

DEFINE_SERVICE_COMMAND_TABLE(IAudioOut, 1, Start, 2, Stop, 3,
                             AppendAudioOutBuffer, 4, RegisterBufferEvent, 5,
                             GetReleasedAudioOutBuffers, 7,
                             AppendAudioOutBufferAuto, 8,
                             GetReleasedAudioOutBuffersAuto)

IAudioOut::IAudioOut(PcmFormat format, u32 sample_rate, u16 channel_count)
    : buffer_event(new kernel::Event(true)) {
    stream = OS_INSTANCE.GetAudioCore().CreateStream(
        format, sample_rate, channel_count, [&](buffer_id_t buffer_id) {
            {
                std::unique_lock lock(buffer_mutex);
                released_buffers.push_back(buffer_id);
            }

            // Signal event
            buffer_event.handle->Signal();
        });
}

result_t IAudioOut::Start() {
    stream->Start();
    return RESULT_SUCCESS;
}

result_t IAudioOut::Stop() {
    stream->Stop();
    return RESULT_SUCCESS;
}

result_t
IAudioOut::AppendAudioOutBuffer(u64 buffer_client_ptr,
                                InBuffer<BufferAttr::MapAlias> buffer_buffer) {
    return AppendAudioOutBufferImpl(buffer_client_ptr, *buffer_buffer.reader);
}

result_t
IAudioOut::RegisterBufferEvent(OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = buffer_event.id;
    return RESULT_SUCCESS;
}

result_t IAudioOut::GetReleasedAudioOutBuffers(
    u32* out_count, OutBuffer<BufferAttr::MapAlias> out_buffers_buffer) {
    return GetReleasedAudioOutBuffersImpl(out_count,
                                          *out_buffers_buffer.writer);
}

result_t IAudioOut::AppendAudioOutBufferAuto(
    u64 buffer_client_ptr, InBuffer<BufferAttr::AutoSelect> buffer_buffer) {
    return AppendAudioOutBufferImpl(buffer_client_ptr, *buffer_buffer.reader);
}

result_t IAudioOut::GetReleasedAudioOutBuffersAuto(
    u32* out_count, OutBuffer<BufferAttr::AutoSelect> out_buffers_buffer) {
    return GetReleasedAudioOutBuffersImpl(out_count,
                                          *out_buffers_buffer.writer);
}

result_t IAudioOut::AppendAudioOutBufferImpl(u64 buffer_client_ptr,
                                             Reader buffer_reader) {
    const auto buffer = buffer_reader.Read<Buffer>();
    // TODO: correct?
    stream->EnqueueBuffer(
        buffer_client_ptr,
        sized_ptr(KERNEL_INSTANCE.GetMMU()->UnmapAddr(buffer.sample_buffer_ptr),
                  buffer.sample_buffer_data_size));

    return RESULT_SUCCESS;
}

result_t IAudioOut::GetReleasedAudioOutBuffersImpl(u32* out_count,
                                                   Writer& out_buffers_writer) {
    std::unique_lock lock(buffer_mutex);

    *out_count = released_buffers.size();
    for (const auto client_ptr : released_buffers)
        out_buffers_writer.Write(client_ptr);
    released_buffers.clear();

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::audio
