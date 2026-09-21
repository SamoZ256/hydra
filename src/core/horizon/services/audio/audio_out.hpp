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
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    kernel::Event* buffer_event;
    IStream* stream;

    std::mutex buffer_mutex;
    std::vector<vaddr_t> released_buffers;

    // Commands
    result_t getAudioOutState(AudioOutState* out_state);
    result_t start();
    result_t stop();
    result_t appendAudioOutBuffer(kernel::Process* process,
                                  u64 buffer_client_ptr,
                                  InBuffer<BufferAttr::MapAlias> buffer_buffer);
    result_t registerBufferEvent(kernel::Process* process,
                                 OutHandle<HandleAttr::Copy> out_handle);
    result_t getReleasedAudioOutBuffers(
        u32* out_count, OutBuffer<BufferAttr::MapAlias> out_buffers_buffer);
    result_t
    appendAudioOutBufferAuto(kernel::Process* process, u64 buffer_client_ptr,
                             InBuffer<BufferAttr::AutoSelect> buffer_buffer);
    result_t getReleasedAudioOutBuffersAuto(
        u32* out_count, OutBuffer<BufferAttr::AutoSelect> out_buffers_buffer);

    // Impl
    result_t appendAudioOutBufferImpl(
        kernel::Process* process, u64 buffer_client_ptr,
        std::optional<ztd::io::MemoryStream> in_buffer_stream);
    result_t getReleasedAudioOutBuffersImpl(
        u32* out_count,
        std::optional<ztd::io::MemoryStream> out_buffers_stream);
};

} // namespace hydra::horizon::services::audio
