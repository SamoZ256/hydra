#include "core/horizon/services/hid/applet_resource.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::hid {

DEFINE_SERVICE_COMMAND_TABLE(IAppletResource, 0, getSharedMemoryHandle)

IAppletResource::IAppletResource(System& system_,
                                 kernel::AppletResourceUserId aruid_)
    : system{system_}, aruid{aruid_},
      resource{system.getOs().getHidResourceManager().createResource(aruid)} {}

IAppletResource::~IAppletResource() {
    system.getOs().getHidResourceManager().destroyResource(aruid);
}

result_t
IAppletResource::getSharedMemoryHandle(kernel::Process* process,
                                       OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->addHandle(resource.getSharedMemory());

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::hid
