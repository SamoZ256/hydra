#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/services/am/internal/library_applet_controller.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::applets {
class AppletBase;
}

namespace hydra::horizon::services::am {

class IStorage;

class ILibraryAppletAccessor : public IService {
  public:
    ILibraryAppletAccessor(const AppletId id, const LibraryAppletMode mode);
    ~ILibraryAppletAccessor();

  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    internal::LibraryAppletController controller;
    applets::AppletBase* applet;

    // Commands
    result_t GetAppletStateChangedEvent(kernel::Process* process,
                                        OutHandle<HandleAttr::Copy> out_handle);
    result_t Start(System* system);
    result_t GetResult();
    result_t PushInData(IService* storage_);
    result_t PopOutData(RequestContext* ctx);
    result_t PushInteractiveInData(IService* storage_);
    result_t PopInteractiveOutData(RequestContext* ctx);
    result_t
    GetPopInteractiveOutDataEvent(kernel::Process* process,
                                  OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::am
