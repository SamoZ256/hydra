#include "core/horizon/services/audio/audio_renderer_manager.hpp"

#include "core/horizon/services/audio/audio_device.hpp"
#include "core/horizon/services/audio/audio_renderer.hpp"

namespace hydra::horizon::services::audio {

DEFINE_SERVICE_COMMAND_TABLE(IAudioRendererManager, 0, OpenAudioRenderer, 1,
                             GetWorkBufferSize, 2, GetAudioDeviceService, 4,
                             GetAudioDeviceServiceWithRevisionInfo)

result_t IAudioRendererManager::OpenAudioRenderer(
    add_service_fn_t add_service, aligned<AudioRendererParameters, 56> params,
    u64 work_buffer_size, u64 aruid) {
    add_service(new IAudioRenderer(params, work_buffer_size));
    return RESULT_SUCCESS;
}

result_t
IAudioRendererManager::GetWorkBufferSize(AudioRendererParameters params,
                                         u64* out_size) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_size = 0x20000;
    return RESULT_SUCCESS;
}

result_t
IAudioRendererManager::GetAudioDeviceService(add_service_fn_t add_service,
                                             u64 aruid) {
    add_service(new IAudioDevice());
    return RESULT_SUCCESS;
}

result_t IAudioRendererManager::GetAudioDeviceServiceWithRevisionInfo(
    add_service_fn_t add_service) {
    // TODO: revision info
    add_service(new IAudioDevice());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::audio
