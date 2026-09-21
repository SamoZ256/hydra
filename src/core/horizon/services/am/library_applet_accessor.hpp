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
    ~ILibraryAppletAccessor() override;

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    internal::LibraryAppletController controller;
    applets::AppletBase* applet;

    // Commands
    result_t getAppletStateChangedEvent(kernel::Process* process,
                                        OutHandle<HandleAttr::Copy> out_handle);
    result_t start(System* system);
    result_t getResult();
    result_t pushInData(IService* storage_);
    result_t popOutData(RequestContext* ctx);
    result_t pushInteractiveInData(IService* storage_);
    result_t popInteractiveOutData(RequestContext* ctx);
    result_t
    getPopInteractiveOutDataEvent(kernel::Process* process,
                                  OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::am
