#pragma once

#include "core/horizon/services/audio/const.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::audio {

class IAudioRendererManager : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t OpenAudioRenderer(add_service_fn_t add_service,
                               aligned<AudioRendererParameters, 56> params,
                               u64 work_buffer_size, u64 aruid);
    result_t GetWorkBufferSize(AudioRendererParameters params, u64* out_size);
    result_t GetAudioDeviceService(add_service_fn_t add_service, u64 aruid);
    // TODO: params
    result_t
    GetAudioDeviceServiceWithRevisionInfo(add_service_fn_t add_service);
};

} // namespace hydra::horizon::services::audio
