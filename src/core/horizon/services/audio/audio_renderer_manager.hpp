#pragma once

#include "core/horizon/services/audio/const.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::audio {

class IAudioRendererManager : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t OpenAudioRenderer(RequestContext* ctx,
                               aligned<AudioRendererParameters, 56> params,
                               u64 work_buffer_size, u64 aruid,
                               InHandle<HandleAttr::Copy> work_buffer_handle,
                               InHandle<HandleAttr::Copy> process_handle);
    result_t GetWorkBufferSize(AudioRendererParameters params, u64* out_size);
    result_t GetAudioDeviceService(RequestContext* ctx, u64 aruid);
    // TODO: params
    result_t GetAudioDeviceServiceWithRevisionInfo(RequestContext* ctx);
};

} // namespace hydra::horizon::services::audio
