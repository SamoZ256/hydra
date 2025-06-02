#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IApplicationProxy : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetCommonStateGetter(add_service_fn_t add_service);
    result_t GetSelfController(add_service_fn_t add_service);
    result_t GetWindowController(add_service_fn_t add_service);
    result_t GetAudioController(add_service_fn_t add_service);
    result_t GetDisplayController(add_service_fn_t add_service);
    result_t GetLibraryAppletCreator(add_service_fn_t add_service);
    result_t GetApplicationFunctions(add_service_fn_t add_service);
    result_t GetDebugFunctions(add_service_fn_t add_service);
};

} // namespace hydra::horizon::services::am
