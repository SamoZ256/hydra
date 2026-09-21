#include "core/horizon/services/am/audio_controller.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IAudioController, 0, setExpectedMasterVolume, 1,
                             getMainAppletExpectedMasterVolume, 2,
                             getLibraryAppletExpectedMasterVolume)

result_t IAudioController::setExpectedMasterVolume(f32 main_applet_volume,
                                                   f32 library_applet_volume) {
    LOG_FUNC_WITH_ARGS_STUBBED(
        Services, "main applet volume: {}, library applet volume: {}",
        main_applet_volume, library_applet_volume);
    return RESULT_SUCCESS;
}

result_t IAudioController::getMainAppletExpectedMasterVolume(f32* out_volume) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_volume = 1.0f;
    return RESULT_SUCCESS;
}

result_t
IAudioController::getLibraryAppletExpectedMasterVolume(f32* out_volume) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_volume = 1.0f;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
