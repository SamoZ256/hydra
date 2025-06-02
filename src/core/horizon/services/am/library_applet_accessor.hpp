#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::applets {
class AppletBase;
}

namespace hydra::horizon::services::am {

class IStorage;

class ILibraryAppletAccessor : public ServiceBase {
  public:
    ILibraryAppletAccessor(const AppletId id, const LibraryAppletMode mode);
    ~ILibraryAppletAccessor();

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    applets::AppletBase* applet;

    // Commands
    result_t GetAppletStateChangedEvent(OutHandle<HandleAttr::Copy> out_handle);
    result_t Start();
    result_t GetResult();
    result_t PushInData(ServiceBase* storage_);
    result_t PopOutData(add_service_fn_t add_service);
};

} // namespace hydra::horizon::services::am
