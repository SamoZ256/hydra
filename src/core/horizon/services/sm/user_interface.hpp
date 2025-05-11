#pragma once

#include "core/horizon/services/const.hpp"

namespace Hydra::Horizon::Services::Sm {

class IUserInterface : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    STUB_REQUEST_COMMAND(RegisterProcess);
    result_t GetServiceHandle(add_service_fn_t add_service, u64 name);
};

} // namespace Hydra::Horizon::Services::Sm
