#include "core/horizon/services/am/application_proxy.hpp"

#include "core/horizon/services/am/application_functions.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IApplicationProxy, 0, getCommonStateGetter, 1,
                             getSelfController, 2, getWindowController, 3,
                             getAudioController, 4, getDisplayController, 11,
                             getLibraryAppletCreator, 20,
                             getApplicationFunctions, 1000, getDebugFunctions)

result_t IApplicationProxy::getApplicationFunctions(RequestContext* ctx) {
    addService(*ctx, new IApplicationFunctions());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
