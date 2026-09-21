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

result_t IProxy::getCommonStateGetter(RequestContext* ctx) {
    addService(*ctx, new ICommonStateGetter());
    return RESULT_SUCCESS;
}

result_t IProxy::getSelfController(RequestContext* ctx) {
    addService(*ctx, new ISelfController());
    return RESULT_SUCCESS;
}

result_t IProxy::getWindowController(RequestContext* ctx) {
    addService(*ctx, new IWindowController());
    return RESULT_SUCCESS;
}

result_t IProxy::getAudioController(RequestContext* ctx) {
    addService(*ctx, new IAudioController());
    return RESULT_SUCCESS;
}

result_t IProxy::getDisplayController(RequestContext* ctx) {
    addService(*ctx, new IDisplayController());
    return RESULT_SUCCESS;
}

result_t IProxy::getProcessWindingController(RequestContext* ctx) {
    addService(*ctx, new IProcessWindingController());
    return RESULT_SUCCESS;
}

result_t IProxy::getLibraryAppletCreator(RequestContext* ctx) {
    addService(*ctx, new ILibraryAppletCreator());
    return RESULT_SUCCESS;
}

result_t IProxy::getAppletCommonFunctions(RequestContext* ctx) {
    addService(*ctx, new IAppletCommonFunctions());
    return RESULT_SUCCESS;
}

result_t IProxy::getHomeMenuFunctions(RequestContext* ctx) {
    addService(*ctx, new IHomeMenuFunctions());
    return RESULT_SUCCESS;
}

result_t IProxy::getGlobalStateController(RequestContext* ctx) {
    addService(*ctx, new IGlobalStateController());
    return RESULT_SUCCESS;
}

result_t IProxy::getDebugFunctions(RequestContext* ctx) {
    addService(*ctx, new IDebugFunctions());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
