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
                               Aligned<AudioRendererParameters, 56> params,
                               u64 work_buffer_size, u64 aruid);
    result_t GetWorkBufferSize(AudioRendererParameters params, u64* out_size);
    result_t GetAudioDeviceService(RequestContext* ctx, u64 aruid);
    // TODO: params
    result_t GetAudioDeviceServiceWithRevisionInfo(RequestContext* ctx);
};

} // namespace hydra::horizon::services::audio
