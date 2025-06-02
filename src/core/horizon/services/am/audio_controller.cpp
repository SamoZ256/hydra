#include "core/horizon/services/am/audio_controller.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IAudioController, 0, SetExpectedMasterVolume, 1,
                             GetMainAppletExpectedMasterVolume, 2,
                             GetLibraryAppletExpectedMasterVolume)

result_t IAudioController::SetExpectedMasterVolume(f32 main_applet_volume,
                                                   f32 library_applet_volume) {
    LOG_FUNC_STUBBED(Services);
    return RESULT_SUCCESS;
}

result_t IAudioController::GetMainAppletExpectedMasterVolume(f32* out_volume) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_volume = 1.0f;
    return RESULT_SUCCESS;
}

result_t
IAudioController::GetLibraryAppletExpectedMasterVolume(f32* out_volume) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_volume = 1.0f;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
