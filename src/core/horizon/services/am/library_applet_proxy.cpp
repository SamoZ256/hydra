#include "core/horizon/services/am/library_applet_proxy.hpp"

#include "core/horizon/services/am/library_applet_self_accessor.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(
    ILibraryAppletProxy, 0, getCommonStateGetter, 1, getSelfController, 2,
    getWindowController, 3, getAudioController, 4, getDisplayController, 10,
    getProcessWindingController, 11, getLibraryAppletCreator, 20,
    openLibraryAppletSelfAccessor, 21, getAppletCommonFunctions, 22,
    getHomeMenuFunctions, 23, getGlobalStateController, 1000, getDebugFunctions)

result_t
ILibraryAppletProxy::openLibraryAppletSelfAccessor(RequestContext* ctx) {
    addService(*ctx, new ILibraryAppletSelfAccessor());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
