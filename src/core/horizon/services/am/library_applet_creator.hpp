#pragma once

#include "core/horizon/services/am/library_applet_accessor.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class ILibraryAppletCreator : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t CreateLibraryApplet(add_service_fn_t add_service, AppletId id,
                                 LibraryAppletMode mode);
    result_t CreateStorage(add_service_fn_t add_service, i64 size);
    result_t CreateTransferMemoryStorage(add_service_fn_t add_service,
                                         InHandle<HandleAttr::Copy> tmem_handle,
                                         bool writable, i64 size);
};

} // namespace hydra::horizon::services::am
