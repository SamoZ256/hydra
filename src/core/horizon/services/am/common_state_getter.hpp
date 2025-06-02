#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/state_manager.hpp"

namespace hydra::horizon::services::am {

enum class OperationMode {
    Handheld,
    Console,
};

class ICommonStateGetter : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetEventHandle(OutHandle<HandleAttr::Copy> out_handle);
    result_t ReceiveMessage(AppletMessage* out_message);
    STUB_REQUEST_COMMAND(DisallowToEnterSleep);
    result_t GetOperationMode(OperationMode* out_mode);
    STUB_REQUEST_COMMAND(GetPerformanceMode);
    result_t GetCurrentFocusState(AppletFocusState* out_state);
};

} // namespace hydra::horizon::services::am
