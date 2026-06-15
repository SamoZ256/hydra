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

class IAudioRenderer : public IService {
  public:
    IAudioRenderer(const AudioRendererParameters& params_,
                   const u64 work_buffer_size_);

  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    AudioRendererParameters params;
    // TODO: use
    [[maybe_unused]] u64 work_buffer_size;

    kernel::Event* event;

    u32 rendering_time_limit{0x1000}; // TODO: what should this be?
    std::vector<VoiceInfoOut> voices;

    // Commands
    result_t RequestUpdate(InBuffer<BufferAttr::MapAlias> in_buffer,
                           OutBuffer<BufferAttr::MapAlias> out_buffer,
                           OutBuffer<BufferAttr::MapAlias> out_perf_buffer);
    STUB_REQUEST_COMMAND(Start);
    STUB_REQUEST_COMMAND(Stop);
    result_t QuerySystemEvent(kernel::Process* process,
                              OutHandle<HandleAttr::Copy> out_handle);
    result_t SetRenderingTimeLimit(u32 time_limit);
    result_t GetRenderingTimeLimit(u32* out_time_limit);
    result_t
    RequestUpdateAuto(InBuffer<BufferAttr::AutoSelect> in_buffer,
                      OutBuffer<BufferAttr::AutoSelect> out_buffer,
                      OutBuffer<BufferAttr::AutoSelect> out_perf_buffer);

    // Impl
    result_t RequestUpdateImpl(std::optional<io::MemoryStream> in_stream,
                               std::optional<io::MemoryStream> out_stream,
                               std::optional<io::MemoryStream> out_perf_stream);
};

} // namespace hydra::horizon::services::audio
