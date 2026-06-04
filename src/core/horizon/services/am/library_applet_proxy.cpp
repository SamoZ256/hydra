#include "core/horizon/services/am/library_applet_proxy.hpp"

#include "core/horizon/services/am/library_applet_self_accessor.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ILibraryAppletProxy, 0, GetCommonStateGetter, 1,
                             GetSelfController, 2, GetWindowController, 3,
                             GetAudioController, 4, GetDisplayController, 10,
                             GetProcessWindingController, 11,
                             GetLibraryAppletCreator, 20,
                             OpenLibraryAppletSelfAccessor, 21,
                             GetAppletCommonFunctions, 1000, GetDebugFunctions)

result_t
ILibraryAppletProxy::OpenLibraryAppletSelfAccessor(RequestContext* ctx) {
    AddService(*ctx, new ILibraryAppletSelfAccessor());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
