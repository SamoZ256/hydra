#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

struct DisplayVersion {
    char name[0x10];
};

enum class GamePlayRecordingState : u32 {
    Disabled = 0,
    Enabled = 1,
};

class IApplicationFunctions : public IService {
  public:
    IApplicationFunctions()
        : gpu_error_detect_event{
              new kernel::Event(false, "Gpu error detect event")} {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    kernel::Event* gpu_error_detect_event;
    GamePlayRecordingState game_play_recording_state{
        GamePlayRecordingState::Disabled}; // TODO: what is the default?

    // Commands
    result_t popLaunchParameter(kernel::Process* process, RequestContext* ctx,
                                kernel::LaunchParameterKind kind);
    result_t ensureSaveData(uuid_t user_id, u64* out_required_size);
    result_t getDesiredLanguage(LanguageCode* out_language_code);
    result_t setTerminateResult(result_t result);
    result_t getDisplayVersion(DisplayVersion* out_version);
    result_t extendSaveData(u128 user_id, i64 size, i64 journal_size,
                            result_t* out_result);                  // 3.0.0+
    result_t getSaveDataSize(i64* out_size, i64* out_journal_size); // 3.0.0+
    STUB_REQUEST_COMMAND(beginBlockingHomeButton);
    STUB_REQUEST_COMMAND(endBlockingHomeButton);
    STUB_REQUEST_COMMAND(notifyRunning);
    result_t getPseudoDeviceId(u128* out_id);
    result_t isGamePlayRecordingSupported(bool* out_enabled);
    STUB_REQUEST_COMMAND(initializeGamePlayRecording);
    result_t setGamePlayRecordingState(GamePlayRecordingState state);
    result_t enableApplicationCrashReport(bool enabled);
    result_t
    getGpuErrorDetectedSystemEvent(kernel::Process* process,
                                   OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::am

ENABLE_ENUM_FORMATTING(hydra::horizon::services::am::GamePlayRecordingState,
                       Disabled, "disabled", Enabled, "enabled");
