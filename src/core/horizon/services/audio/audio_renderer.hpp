#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/audio/const.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::audio {

struct VoiceInfoOut {
    u64 played_sample_count;
    u32 num_wave_buffers_consumed;
    u32 voice_drops_count;
};

class IAudioRenderer : public ServiceBase {
  public:
    IAudioRenderer(const AudioRendererParameters& params_,
                   const usize work_buffer_size_);

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    AudioRendererParameters params;
    usize work_buffer_size;

    kernel::HandleWithId<kernel::Event> event;

    std::vector<VoiceInfoOut> voices;

    // Commands
    result_t RequestUpdate(InBuffer<BufferAttr::MapAlias> in_buffer,
                           OutBuffer<BufferAttr::MapAlias> out_buffer,
                           OutBuffer<BufferAttr::MapAlias> out_perf_buffer);
    STUB_REQUEST_COMMAND(Start);
    STUB_REQUEST_COMMAND(Stop);
    result_t QuerySystemEvent(OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::audio
