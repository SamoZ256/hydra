#pragma once

#include "core/horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::Am {

class IWindowController : public ServiceBase {
  public:
    DEFINE_SERVICE_VIRTUAL_FUNCTIONS(IWindowController)

  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
    // Commands
    void GetAppletResourceUserId(REQUEST_COMMAND_PARAMS);
    STUB_REQUEST_COMMAND(AcquireForegroundRights);
};

} // namespace Hydra::Horizon::Services::Am
