#include "core/horizon/services/hid/applet_resource.hpp"

#include "core/horizon/os.hpp"

namespace hydra::horizon::services::hid {

DEFINE_SERVICE_COMMAND_TABLE(IAppletResource, 0, GetSharedMemoryHandle)

result_t
IAppletResource::GetSharedMemoryHandle(OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = OS::GetInstance().GetInputManager().GetSharedMemoryId();

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::hid
