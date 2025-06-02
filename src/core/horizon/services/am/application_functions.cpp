#include "core/horizon/services/am/application_functions.hpp"

#include "core/horizon/services/am/storage.hpp"
#include "core/horizon/state_manager.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IApplicationFunctions, 1, PopLaunchParameter, 20,
                             EnsureSaveData, 21, GetDesiredLanguage, 22,
                             SetTerminateResult, 25, ExtendSaveData, 26,
                             GetSaveDataSize, 40, NotifyRunning, 50,
                             GetPseudoDeviceId, 66, InitializeGamePlayRecording,
                             67, SetGamePlayRecordingState, 130,
                             GetGpuErrorDetectedSystemEvent)

result_t IApplicationFunctions::PopLaunchParameter(add_service_fn_t add_service,
                                                   LaunchParameterKind kind) {
    LOG_DEBUG(Services, "Kind: {}", kind);

    add_service(
        new IStorage(StateManager::GetInstance().PopLaunchParameter(kind)));
    return RESULT_SUCCESS;
}

result_t IApplicationFunctions::EnsureSaveData(uuid_t user_id,
                                               u64* out_unknown) {
    LOG_DEBUG(Services, "User ID: 0x{:016x}", user_id);

    LOG_FUNC_STUBBED(Services);

    // HACK
    // NOTE: writing anything other than 0x0 causes the game to launch the
    // dataErase LibraryApplet
    *out_unknown = 0x0;
    return RESULT_SUCCESS;
}

result_t
IApplicationFunctions::GetDesiredLanguage(LanguageCode* out_language_code) {
    // TODO: make this configurable
    *out_language_code = LanguageCode::AmericanEnglish;
    return RESULT_SUCCESS;
}

result_t IApplicationFunctions::SetTerminateResult(result_t result) {
    const auto module = GET_RESULT_MODULE(result);
    const auto description = GET_RESULT_DESCRIPTION(result);
    LOG_INFO(Kernel, "Module: {}, description: {}", module, description);

    return RESULT_SUCCESS;
}

result_t IApplicationFunctions::ExtendSaveData(u128 user_id, i64 size,
                                               i64 journal_size,
                                               result_t* out_result) {
    LOG_FUNC_STUBBED(Services);

    LOG_DEBUG(Services,
              "User ID: 0x{:016x}, size: 0x{:08x}, Journal size: 0x{:08x}",
              user_id, size, journal_size);

    // HACK
    // TODO: why 2 results?
    *out_result = RESULT_SUCCESS;

    return RESULT_SUCCESS;
}

result_t IApplicationFunctions::GetSaveDataSize(i64* out_size,
                                                i64* out_journal_size) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_size = 0;
    *out_journal_size = 0;
    return RESULT_SUCCESS;
}

result_t IApplicationFunctions::GetPseudoDeviceId(u128* out_id) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_id = str_to_u64("pseudo_id");
    return RESULT_SUCCESS;
}

result_t IApplicationFunctions::GetGpuErrorDetectedSystemEvent(
    OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = gpu_error_detect_event.id;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
