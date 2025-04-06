#pragma once

#include "horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::HosBinder {

class IHOSBinderDriver : public ServiceBase {
  public:
    DEFINE_SERVICE_VIRTUAL_FUNCTIONS(IHOSBinderDriver)

  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
    // Commands
    void TransactParcel(REQUEST_COMMAND_PARAMS);
    void AdjustRefcount(REQUEST_COMMAND_PARAMS);
    void GetNativeHandle(REQUEST_COMMAND_PARAMS);
};

} // namespace Hydra::Horizon::Services::HosBinder
