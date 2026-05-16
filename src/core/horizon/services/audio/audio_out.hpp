#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/audio/const.hpp"
#include "core/horizon/services/const.hpp"

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

class IAudioOut : public IService {
  public:
    IAudioOut(System& system, PcmFormat format, u32 sample_rate,
              u16 channel_count);

  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    kernel::Event* buffer_event;
    IStream* stream;

    std::mutex buffer_mutex;
    std::vector<vaddr_t> released_buffers;

    // Commands
    result_t GetAudioOutState(AudioOutState* out_state);
    result_t Start();
    result_t Stop();
    result_t AppendAudioOutBuffer(kernel::Process* process,
                                  u64 buffer_client_ptr,
                                  InBuffer<BufferAttr::MapAlias> buffer_buffer);
    result_t RegisterBufferEvent(kernel::Process* process,
                                 OutHandle<HandleAttr::Copy> out_handle);
    result_t GetReleasedAudioOutBuffers(
        u32* out_count, OutBuffer<BufferAttr::MapAlias> out_buffers_buffer);
    result_t
    AppendAudioOutBufferAuto(kernel::Process* process, u64 buffer_client_ptr,
                             InBuffer<BufferAttr::AutoSelect> buffer_buffer);
    result_t GetReleasedAudioOutBuffersAuto(
        u32* out_count, OutBuffer<BufferAttr::AutoSelect> out_buffers_buffer);

    // Impl
    result_t AppendAudioOutBufferImpl(kernel::Process* process,
                                      u64 buffer_client_ptr,
                                      io::MemoryStream* in_buffer_stream);
    result_t
    GetReleasedAudioOutBuffersImpl(u32* out_count,
                                   io::MemoryStream* out_buffers_stream);
};

} // namespace hydra::horizon::services::audio
