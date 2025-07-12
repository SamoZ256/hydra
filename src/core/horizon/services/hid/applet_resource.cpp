#include "core/horizon/services/hid/applet_resource.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/os.hpp"

namespace hydra::horizon::services::hid {

DEFINE_SERVICE_COMMAND_TABLE(IAppletResource, 0, GetSharedMemoryHandle)

result_t
IAppletResource::GetSharedMemoryHandle(kernel::Process* process,
                                       OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(
        OS::GetInstance().GetInputManager().GetSharedMemory());

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::hid
