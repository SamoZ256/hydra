#include "core/horizon/services/am/application_functions.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/am/storage.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(
    IApplicationFunctions, 1, PopLaunchParameter, 20, EnsureSaveData, 21,
    GetDesiredLanguage, 22, SetTerminateResult, 23, GetDisplayVersion, 25,
    ExtendSaveData, 26, GetSaveDataSize, 32, BeginBlockingHomeButton, 33,
    EndBlockingHomeButton, 40, NotifyRunning, 50, GetPseudoDeviceId, 66,
    InitializeGamePlayRecording, 67, SetGamePlayRecordingState, 90,
    EnableApplicationCrashReport, 130, GetGpuErrorDetectedSystemEvent)

result_t
IApplicationFunctions::PopLaunchParameter(kernel::Process* process,
                                          RequestContext* ctx,
                                          kernel::LaunchParameterKind kind) {
    LOG_DEBUG(Services, "Kind: {}", kind);

    AddService(
        *ctx, new IStorage(process->GetAppletState().PopLaunchParameter(kind)));
    return RESULT_SUCCESS;
}

result_t IApplicationFunctions::EnsureSaveData(uuid_t user_id,
                                               u64* out_required_size) {
    LOG_DEBUG(Services, "User ID: 0x{:016x}", user_id);

    LOG_FUNC_STUBBED(Services);

    // TODO: check if there is enough space for the save data
    *out_required_size = 0x0;
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

result_t IApplicationFunctions::GetDisplayVersion(DisplayVersion* out_version) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_version = {
        .name = "1.0.0",
    };
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
    *out_id = "dev_id"_u64;
    return RESULT_SUCCESS;
}

result_t IApplicationFunctions::EnableApplicationCrashReport(bool enabled) {
    LOG_FUNC_STUBBED(Services);

    LOG_DEBUG(Services, "Enabled: {}", enabled);

    return RESULT_SUCCESS;
}

result_t IApplicationFunctions::GetGpuErrorDetectedSystemEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(gpu_error_detect_event);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
