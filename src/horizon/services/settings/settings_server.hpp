#pragma once

#include "horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::Settings {

class ISettingsServer : public ServiceBase {
  public:
    DEFINE_SERVICE_VIRTUAL_FUNCTIONS(ISettingsServer)

  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
    // Commands
    void GetLanguageCode(REQUEST_COMMAND_PARAMS);
    void GetAvailableLanguageCodes(REQUEST_COMMAND_PARAMS);
    void GetAvailableLanguageCodeCount(REQUEST_COMMAND_PARAMS);
};

} // namespace Hydra::Horizon::Services::Settings
