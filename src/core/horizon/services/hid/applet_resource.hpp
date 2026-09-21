#pragma once

#include "core/horizon/kernel/applet_resource.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::hid {

namespace internal {
class AppletResource;
}

class IAppletResource : public IService {
  public:
    IAppletResource(System& system_, kernel::AppletResourceUserId aruid_);
    ~IAppletResource() override;

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    System& system;
    kernel::AppletResourceUserId aruid;
    internal::AppletResource& resource;

    // Commands
    result_t getSharedMemoryHandle(kernel::Process* process,
                                   OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::hid
