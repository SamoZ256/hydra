#include "core/horizon/services/audio/audio_out.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::audio {

DEFINE_SERVICE_COMMAND_TABLE(IAudioOut, 0, getAudioOutState, 1, start, 2, stop,
                             3, appendAudioOutBuffer, 4, registerBufferEvent, 5,
                             getReleasedAudioOutBuffers, 7,
                             appendAudioOutBufferAuto, 8,
                             getReleasedAudioOutBuffersAuto)

IAudioOut::IAudioOut(System& system, PcmFormat format, u32 sample_rate,
                     u16 channel_count)
    : buffer_event{new kernel::Event(false, "IAudioOut buffer event")} {
    stream = system.getAudioCore().createStream(
        format, sample_rate, channel_count, [&](buffer_id_t buffer_id) {
            {
                std::unique_lock lock(buffer_mutex);
                released_buffers.push_back(buffer_id);
            }

            // Signal event
            buffer_event->signal();
        });
}

result_t IAudioOut::getAudioOutState(AudioOutState* out_state) {
    *out_state =
        (stream->getState() == StreamState::Started ? AudioOutState::Started
                                                    : AudioOutState::Stopped);
    return RESULT_SUCCESS;
}

result_t IAudioOut::start() {
    stream->start();
    return RESULT_SUCCESS;
}

result_t IAudioOut::stop() {
    stream->stop();
    return RESULT_SUCCESS;
}

result_t
IAudioOut::appendAudioOutBuffer(kernel::Process* process, u64 buffer_client_ptr,
                                InBuffer<BufferAttr::MapAlias> buffer_buffer) {
    return appendAudioOutBufferImpl(process, buffer_client_ptr,
                                    buffer_buffer.stream);
}

result_t
IAudioOut::registerBufferEvent(kernel::Process* process,
                               OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->addHandle(buffer_event);
    return RESULT_SUCCESS;
}

result_t IAudioOut::getReleasedAudioOutBuffers(
    u32* out_count, OutBuffer<BufferAttr::MapAlias> out_buffers_buffer) {
    return getReleasedAudioOutBuffersImpl(out_count, out_buffers_buffer.stream);
}

result_t IAudioOut::appendAudioOutBufferAuto(
    kernel::Process* process, u64 buffer_client_ptr,
    InBuffer<BufferAttr::AutoSelect> buffer_buffer) {
    return appendAudioOutBufferImpl(process, buffer_client_ptr,
                                    buffer_buffer.stream);
}

result_t IAudioOut::getReleasedAudioOutBuffersAuto(
    u32* out_count, OutBuffer<BufferAttr::AutoSelect> out_buffers_buffer) {
    return getReleasedAudioOutBuffersImpl(out_count, out_buffers_buffer.stream);
}

result_t IAudioOut::appendAudioOutBufferImpl(
    kernel::Process* process, u64 buffer_client_ptr,
    std::optional<ztd::io::MemoryStream> in_buffer_stream) {
    const auto buffer = in_buffer_stream->read<Buffer>();
    // TODO: correct?
    const auto ptr = reinterpret_cast<u8*>(
        process->getMmu()->unmapAddr(buffer.sample_buffer_ptr));
    stream->enqueueBuffer(buffer_client_ptr,
                          std::span{ptr, buffer.sample_buffer_data_size});

    return RESULT_SUCCESS;
}

result_t IAudioOut::getReleasedAudioOutBuffersImpl(
    u32* out_count, std::optional<ztd::io::MemoryStream> out_buffers_stream) {
    std::unique_lock lock(buffer_mutex);

    *out_count = static_cast<u32>(released_buffers.size());

    if (released_buffers.empty()) {
        out_buffers_stream->write<u64>(0);
    } else {
        for (const auto client_ptr : released_buffers)
            out_buffers_stream->write(client_ptr);
        released_buffers.clear();
    }

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::audio
