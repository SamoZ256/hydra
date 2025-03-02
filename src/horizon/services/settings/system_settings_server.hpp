#pragma once

#include "horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::Settings {

class ISystemSettingsServer : public ServiceBase {
  public:
    DEFINE_SERVICE_VIRTUAL_FUNCTIONS(ISystemSettingsServer)

  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
};

} // namespace Hydra::Horizon::Services::Settings
