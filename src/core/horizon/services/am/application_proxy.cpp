#include "core/horizon/services/am/application_proxy.hpp"

#include "core/horizon/services/am/application_functions.hpp"
#include "core/horizon/services/am/audio_controller.hpp"
#include "core/horizon/services/am/common_state_getter.hpp"
#include "core/horizon/services/am/debug_functions.hpp"
#include "core/horizon/services/am/display_controller.hpp"
#include "core/horizon/services/am/library_applet_creator.hpp"
#include "core/horizon/services/am/self_controller.hpp"
#include "core/horizon/services/am/window_controller.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IApplicationProxy, 0, GetCommonStateGetter, 1,
                             GetSelfController, 2, GetWindowController, 3,
                             GetAudioController, 4, GetDisplayController, 11,
                             GetLibraryAppletCreator, 20,
                             GetApplicationFunctions, 1000, GetDebugFunctions)

result_t IApplicationProxy::GetCommonStateGetter(add_service_fn_t add_service) {
    add_service(new ICommonStateGetter());
    return RESULT_SUCCESS;
}

result_t IApplicationProxy::GetSelfController(add_service_fn_t add_service) {
    add_service(new ISelfController());
    return RESULT_SUCCESS;
}

result_t IApplicationProxy::GetWindowController(add_service_fn_t add_service) {
    add_service(new IWindowController());
    return RESULT_SUCCESS;
}

result_t IApplicationProxy::GetAudioController(add_service_fn_t add_service) {
    add_service(new IAudioController());
    return RESULT_SUCCESS;
}

result_t IApplicationProxy::GetDisplayController(add_service_fn_t add_service) {
    add_service(new IDisplayController());
    return RESULT_SUCCESS;
}

result_t
IApplicationProxy::GetLibraryAppletCreator(add_service_fn_t add_service) {
    add_service(new ILibraryAppletCreator());
    return RESULT_SUCCESS;
}

result_t
IApplicationProxy::GetApplicationFunctions(add_service_fn_t add_service) {
    add_service(new IApplicationFunctions());
    return RESULT_SUCCESS;
}

result_t IApplicationProxy::GetDebugFunctions(add_service_fn_t add_service) {
    add_service(new IDebugFunctions());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
