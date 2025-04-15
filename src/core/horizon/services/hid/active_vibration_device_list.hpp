#pragma once

#include "core/horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::Hid {

class IActiveVibrationDeviceList : public ServiceBase {
  public:
    DEFINE_SERVICE_VIRTUAL_FUNCTIONS(IActiveVibrationDeviceList)

  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
    // Commands
    STUB_REQUEST_COMMAND(ActivateVibrationDevice);
};

} // namespace Hydra::Horizon::Services::Hid
