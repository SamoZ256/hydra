#include "core/horizon/services/audio/audio_out.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::audio {

DEFINE_SERVICE_COMMAND_TABLE(IAudioOut, 0, GetAudioOutState, 1, Start, 2, Stop,
                             3, AppendAudioOutBuffer, 4, RegisterBufferEvent, 5,
                             GetReleasedAudioOutBuffers, 7,
                             AppendAudioOutBufferAuto, 8,
                             GetReleasedAudioOutBuffersAuto)

IAudioOut::IAudioOut(System& system, PcmFormat format, u32 sample_rate,
                     u16 channel_count)
    : buffer_event{new kernel::Event(false, "IAudioOut buffer event")} {
    stream = system.GetAudioCore().CreateStream(
        format, sample_rate, channel_count, [&](buffer_id_t buffer_id) {
            {
                std::unique_lock lock(buffer_mutex);
                released_buffers.push_back(buffer_id);
            }

            // Signal event
            buffer_event->Signal();
        });
}

result_t IAudioOut::GetAudioOutState(AudioOutState* out_state) {
    *out_state =
        (stream->GetState() == StreamState::Started ? AudioOutState::Started
                                                    : AudioOutState::Stopped);
    return RESULT_SUCCESS;
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
IAudioOut::AppendAudioOutBuffer(kernel::Process* process, u64 buffer_client_ptr,
                                InBuffer<BufferAttr::MapAlias> buffer_buffer) {
    return AppendAudioOutBufferImpl(process, buffer_client_ptr,
                                    buffer_buffer.stream);
}

result_t
IAudioOut::RegisterBufferEvent(kernel::Process* process,
                               OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(buffer_event);
    return RESULT_SUCCESS;
}

result_t IAudioOut::GetReleasedAudioOutBuffers(
    u32* out_count, OutBuffer<BufferAttr::MapAlias> out_buffers_buffer) {
    return GetReleasedAudioOutBuffersImpl(out_count, out_buffers_buffer.stream);
}

result_t IAudioOut::AppendAudioOutBufferAuto(
    kernel::Process* process, u64 buffer_client_ptr,
    InBuffer<BufferAttr::AutoSelect> buffer_buffer) {
    return AppendAudioOutBufferImpl(process, buffer_client_ptr,
                                    buffer_buffer.stream);
}

result_t IAudioOut::GetReleasedAudioOutBuffersAuto(
    u32* out_count, OutBuffer<BufferAttr::AutoSelect> out_buffers_buffer) {
    return GetReleasedAudioOutBuffersImpl(out_count, out_buffers_buffer.stream);
}

result_t
IAudioOut::AppendAudioOutBufferImpl(kernel::Process* process,
                                    u64 buffer_client_ptr,
                                    io::MemoryStream* in_buffer_stream) {
    const auto buffer = in_buffer_stream->Read<Buffer>();
    // TODO: correct?
    const auto ptr = reinterpret_cast<u8*>(
        process->GetMmu()->UnmapAddr(buffer.sample_buffer_ptr));
    stream->EnqueueBuffer(buffer_client_ptr,
                          std::span{ptr, buffer.sample_buffer_data_size});

    return RESULT_SUCCESS;
}

result_t IAudioOut::GetReleasedAudioOutBuffersImpl(
    u32* out_count, io::MemoryStream* out_buffers_stream) {
    std::unique_lock lock(buffer_mutex);

    *out_count = static_cast<u32>(released_buffers.size());

    if (released_buffers.empty()) {
        out_buffers_stream->Write<u64>(0);
    } else {
        for (const auto client_ptr : released_buffers)
            out_buffers_stream->Write(client_ptr);
        released_buffers.clear();
    }

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::audio
