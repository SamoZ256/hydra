#pragma once

#include "core/horizon/kernel/event.hpp"
#include "core/horizon/services/const.hpp"
#include "core/horizon/services/oe/const.hpp"

namespace hydra::horizon::services::am {

enum class OperationMode {
    Handheld,
    Console,
};

class ICommonStateGetter : public IService {
  public:
    ICommonStateGetter()
        : default_display_resolution_change_event{new kernel::Event(
              false, "Default display resolution change event")} {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    kernel::Event*
        default_display_resolution_change_event; // TODO: move this to OS

    // Commands
    result_t getEventHandle(kernel::Process* process,
                            OutHandle<HandleAttr::Copy> out_handle);
    result_t receiveMessage(kernel::Process* process,
                            kernel::AppletMessage* out_message);
    STUB_REQUEST_COMMAND(disallowToEnterSleep);
    result_t getOperationMode(OperationMode* out_mode);
    STUB_REQUEST_COMMAND(getPerformanceMode);
    result_t getDefaultDisplayResolution(System* system, i32* out_width,
                                         i32* out_height);
    result_t getDefaultDisplayResolutionChangeEvent(
        kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle);
    result_t getCurrentFocusState(kernel::Process* process,
                                  kernel::AppletFocusState* out_state);
    result_t setCpuBoostMode(oe::CpuBoostMode mode);
    STUB_REQUEST_COMMAND(
        setRequestExitToLibraryAppletAtExecuteNextProgramEnabled);
};

} // namespace hydra::horizon::services::am
