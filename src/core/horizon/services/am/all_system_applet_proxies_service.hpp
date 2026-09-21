#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IAllSystemAppletProxiesService : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    // TODO: correct?
    result_t openSystemAppletProxy(RequestContext* ctx);
    // TODO: correct?
    result_t openLibraryAppletProxyOld(RequestContext* ctx);
    // TODO: PID and type-0x15 buffer nn::am::AppletAttribute
    result_t
    openLibraryAppletProxy(RequestContext* ctx,
                           [[maybe_unused]] u64 _reserved_x0,
                           InHandle<HandleAttr::Copy> crnt_process_handle);
    // TODO: correct?
    result_t openOverlayAppletProxy(RequestContext* ctx);
    // TODO: correct?
    result_t openSystemApplicationProxy(RequestContext* ctx);

    // Impl
    result_t openLibraryAppletProxyImpl(RequestContext* ctx);
};

} // namespace hydra::horizon::services::am
