#include "core/horizon/services/am/overlay_applet_proxy.hpp"

#include "core/horizon/services/am/overlay_functions.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IOverlayAppletProxy, 0, getCommonStateGetter, 1,
                             getSelfController, 2, getWindowController, 3,
                             getAudioController, 4, getDisplayController, 11,
                             getLibraryAppletCreator, 20, getOverlayFunctions,
                             21, getAppletCommonFunctions, 1000,
                             getDebugFunctions)

result_t IOverlayAppletProxy::getOverlayFunctions(RequestContext* ctx) {
    addService(*ctx, new IOverlayFunctions());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
