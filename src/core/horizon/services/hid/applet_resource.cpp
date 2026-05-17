#include "core/horizon/services/hid/applet_resource.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::hid {

DEFINE_SERVICE_COMMAND_TABLE(IAppletResource, 0, GetSharedMemoryHandle)

IAppletResource::IAppletResource(System& system_,
                                 kernel::AppletResourceUserId aruid_)
    : system{system_}, aruid{aruid_},
      resource{system.GetOS().GetHidResourceManager().CreateResource(aruid)} {}

IAppletResource::~IAppletResource() {
    system.GetOS().GetHidResourceManager().DestroyResource(aruid);
}

result_t
IAppletResource::GetSharedMemoryHandle(kernel::Process* process,
                                       OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(resource.GetSharedMemory());

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::hid
