#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/audio/const.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::audio {
class StreamBase;
}

namespace hydra::horizon::services::audio {

namespace {

struct Buffer {
    u64 next_ptr; // Unused
    u64 sample_buffer_ptr;
    u64 sample_buffer_capacity;
    u64 sample_buffer_data_size;
    u64 sample_buffer_data_offset; // TODO: unused/ignored?
};

} // namespace

class IAudioOut : public ServiceBase {
  public:
    IAudioOut(PcmFormat format, u32 sample_rate, u16 channel_count);

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    kernel::HandleWithId<kernel::Event> buffer_event;
    StreamBase* stream;

    std::mutex buffer_mutex;
    std::vector<vaddr_t> released_buffers;

    // Commands
    result_t Start();
    result_t Stop();
    result_t AppendAudioOutBuffer(u64 buffer_client_ptr,
                                  InBuffer<BufferAttr::MapAlias> buffer_buffer);
    result_t RegisterBufferEvent(OutHandle<HandleAttr::Copy> out_handle);
    result_t GetReleasedAudioOutBuffers(
        u32* out_count, OutBuffer<BufferAttr::MapAlias> out_buffers_buffer);
    result_t
    AppendAudioOutBufferAuto(u64 buffer_client_ptr,
                             InBuffer<BufferAttr::AutoSelect> buffer_buffer);
    result_t GetReleasedAudioOutBuffersAuto(
        u32* out_count, OutBuffer<BufferAttr::AutoSelect> out_buffers_buffer);

    // Impl
    result_t AppendAudioOutBufferImpl(u64 buffer_client_ptr,
                                      Reader buffer_reader);
    result_t GetReleasedAudioOutBuffersImpl(u32* out_count,
                                            Writer& out_buffers_writer);
};

} // namespace hydra::horizon::services::audio
