#pragma once

#include "core/horizon/services/am/library_applet_accessor.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class ILibraryAppletCreator : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t createLibraryApplet(RequestContext* ctx, AppletId id,
                                 LibraryAppletMode mode);
    result_t createStorage(RequestContext* ctx, i64 size);
    result_t createTransferMemoryStorage(kernel::Process* process,
                                         RequestContext* ctx,
                                         InHandle<HandleAttr::Copy> tmem_handle,
                                         bool writable, i64 size);
};

} // namespace hydra::horizon::services::am
