#include "core/horizon/services/am/system_applet_proxy.hpp"

#include "core/horizon/services/am/application_creator.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ISystemAppletProxy, 0, GetCommonStateGetter, 1,
                             GetSelfController, 2, GetWindowController, 3,
                             GetAudioController, 4, GetDisplayController, 10,
                             GetProcessWindingController, 11,
                             GetLibraryAppletCreator, 20, GetHomeMenuFunctions,
                             21, GetGlobalStateController, 22,
                             GetApplicationCreator, 23,
                             GetAppletCommonFunctions, 1000, GetDebugFunctions)

result_t ISystemAppletProxy::GetApplicationCreator(RequestContext* ctx) {
    AddService(*ctx, new IApplicationCreator());
    return RESULT_SUCCESS;
}
} // namespace hydra::horizon::services::am
