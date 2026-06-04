#include "core/horizon/services/am/overlay_applet_proxy.hpp"

#include "core/horizon/services/am/overlay_functions.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IOverlayAppletProxy, 0, GetCommonStateGetter, 1,
                             GetSelfController, 2, GetWindowController, 3,
                             GetAudioController, 4, GetDisplayController, 11,
                             GetLibraryAppletCreator, 20, GetOverlayFunctions,
                             21, GetAppletCommonFunctions, 1000,
                             GetDebugFunctions)

result_t IOverlayAppletProxy::GetOverlayFunctions(RequestContext* ctx) {
    AddService(*ctx, new IOverlayFunctions());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
