#include "core/horizon/services/am/all_system_applet_proxies_service.hpp"

#include "core/horizon/services/am/application_proxy.hpp"
#include "core/horizon/services/am/library_applet_proxy.hpp"
#include "core/horizon/services/am/overlay_applet_proxy.hpp"
#include "core/horizon/services/am/system_applet_proxy.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IAllSystemAppletProxiesService, 100,
                             openSystemAppletProxy, 200,
                             openLibraryAppletProxyOld, 201,
                             openLibraryAppletProxy, 300,
                             openOverlayAppletProxy, 350,
                             openSystemApplicationProxy)

result_t
IAllSystemAppletProxiesService::openSystemAppletProxy(RequestContext* ctx) {
    addService(*ctx, new ISystemAppletProxy());
    return RESULT_SUCCESS;
}

result_t
IAllSystemAppletProxiesService::openLibraryAppletProxyOld(RequestContext* ctx) {
    return openLibraryAppletProxyImpl(ctx);
}

result_t IAllSystemAppletProxiesService::openLibraryAppletProxy(
    RequestContext* ctx, [[maybe_unused]] u64 _reserved_x0,
    InHandle<HandleAttr::Copy> crnt_process_handle) {
    (void)crnt_process_handle;
    return openLibraryAppletProxyImpl(ctx);
}

result_t IAllSystemAppletProxiesService::openLibraryAppletProxyImpl(
    RequestContext* ctx) {
    addService(*ctx, new ILibraryAppletProxy());
    return RESULT_SUCCESS;
}

result_t
IAllSystemAppletProxiesService::openOverlayAppletProxy(RequestContext* ctx) {
    addService(*ctx, new IOverlayAppletProxy());
    return RESULT_SUCCESS;
}

result_t IAllSystemAppletProxiesService::openSystemApplicationProxy(
    RequestContext* ctx) {
    addService(*ctx, new IApplicationProxy());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
