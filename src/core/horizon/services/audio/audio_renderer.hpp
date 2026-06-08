#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/audio/const.hpp"
#include "core/horizon/services/audio/internal/mempool.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::audio {

class IAudioRenderer : public IService {
  public:
    IAudioRenderer(const AudioRendererParameters& params_,
                   std::span<u8> work_buffer_);

  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    AudioRendererParameters params;
    std::span<u8> work_buffer;

    kernel::Event* event;

    u32 rendering_time_limit{0x1000}; // TODO: what should this be?

    std::vector<internal::MemPool> mempools;

    // Commands
    result_t RequestUpdate(kernel::Process* process,
                           InBuffer<BufferAttr::MapAlias> in_buffer,
                           OutBuffer<BufferAttr::MapAlias> out_buffer,
                           OutBuffer<BufferAttr::MapAlias> out_perf_buffer);
    STUB_REQUEST_COMMAND(Start);
    STUB_REQUEST_COMMAND(Stop);
    result_t QuerySystemEvent(kernel::Process* process,
                              OutHandle<HandleAttr::Copy> out_handle);
    result_t SetRenderingTimeLimit(u32 time_limit);
    result_t GetRenderingTimeLimit(u32* out_time_limit);
    result_t
    RequestUpdateAuto(kernel::Process* process,
                      InBuffer<BufferAttr::AutoSelect> in_buffer,
                      OutBuffer<BufferAttr::AutoSelect> out_buffer,
                      OutBuffer<BufferAttr::AutoSelect> out_perf_buffer);

    // Impl
    result_t RequestUpdateImpl(kernel::Process* process,
                               io::MemoryStream* in_stream,
                               io::MemoryStream* out_stream,
                               io::MemoryStream* out_perf_stream);
};

} // namespace hydra::horizon::services::audio
