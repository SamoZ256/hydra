#include "core/horizon/services/am/proxy.hpp"

#include "core/horizon/services/am/applet_common_functions.hpp"
#include "core/horizon/services/am/audio_controller.hpp"
#include "core/horizon/services/am/common_state_getter.hpp"
#include "core/horizon/services/am/debug_functions.hpp"
#include "core/horizon/services/am/display_controller.hpp"
#include "core/horizon/services/am/global_state_controller.hpp"
#include "core/horizon/services/am/home_menu_functions.hpp"
#include "core/horizon/services/am/library_applet_creator.hpp"
#include "core/horizon/services/am/process_winding_controller.hpp"
#include "core/horizon/services/am/self_controller.hpp"
#include "core/horizon/services/am/window_controller.hpp"

namespace hydra::horizon::services::am {

result_t IProxy::GetCommonStateGetter(RequestContext* ctx) {
    AddService(*ctx, new ICommonStateGetter());
    return RESULT_SUCCESS;
}

result_t IProxy::GetSelfController(RequestContext* ctx) {
    AddService(*ctx, new ISelfController());
    return RESULT_SUCCESS;
}

result_t IProxy::GetWindowController(RequestContext* ctx) {
    AddService(*ctx, new IWindowController());
    return RESULT_SUCCESS;
}

result_t IProxy::GetAudioController(RequestContext* ctx) {
    AddService(*ctx, new IAudioController());
    return RESULT_SUCCESS;
}

result_t IProxy::GetDisplayController(RequestContext* ctx) {
    AddService(*ctx, new IDisplayController());
    return RESULT_SUCCESS;
}

result_t IProxy::GetProcessWindingController(RequestContext* ctx) {
    AddService(*ctx, new IProcessWindingController());
    return RESULT_SUCCESS;
}

result_t IProxy::GetLibraryAppletCreator(RequestContext* ctx) {
    AddService(*ctx, new ILibraryAppletCreator());
    return RESULT_SUCCESS;
}

result_t IProxy::GetAppletCommonFunctions(RequestContext* ctx) {
    AddService(*ctx, new IAppletCommonFunctions());
    return RESULT_SUCCESS;
}

result_t IProxy::GetHomeMenuFunctions(RequestContext* ctx) {
    AddService(*ctx, new IHomeMenuFunctions());
    return RESULT_SUCCESS;
}

result_t IProxy::GetGlobalStateController(RequestContext* ctx) {
    AddService(*ctx, new IGlobalStateController());
    return RESULT_SUCCESS;
}

result_t IProxy::GetDebugFunctions(RequestContext* ctx) {
    AddService(*ctx, new IDebugFunctions());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
