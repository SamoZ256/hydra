#include "core/horizon/services/am/system_applet_proxy.hpp"

#include "core/horizon/services/am/application_creator.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ISystemAppletProxy, 0, getCommonStateGetter, 1,
                             getSelfController, 2, getWindowController, 3,
                             getAudioController, 4, getDisplayController, 10,
                             getProcessWindingController, 11,
                             getLibraryAppletCreator, 20, getHomeMenuFunctions,
                             21, getGlobalStateController, 22,
                             getApplicationCreator, 23,
                             getAppletCommonFunctions, 1000, getDebugFunctions)

result_t ISystemAppletProxy::getApplicationCreator(RequestContext* ctx) {
    addService(*ctx, new IApplicationCreator());
    return RESULT_SUCCESS;
}
} // namespace hydra::horizon::services::am
