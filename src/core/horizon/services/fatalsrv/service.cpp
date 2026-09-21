#include "core/horizon/services/fatalsrv/service.hpp"

#include "core/debugger/debugger_manager.hpp"

namespace hydra::horizon::services::fatalsrv {

DEFINE_SERVICE_COMMAND_TABLE(IService, 1, throwFatalWithPolicy, 2,
                             throwFatalWithCpuContext)

result_t
IService::throwFatalWithCpuContext(u64 code, u64 _unknown_x8,
                                   InBuffer<BufferAttr::MapAlias> in_buffer) {
    (void)_unknown_x8;

    // TODO: print properly
    LOG_INFO(Services, "Error: 0x{:x}", code);

    // TODO: stack trace and other stuff in the buffer
    (void)in_buffer;

    GET_CURRENT_PROCESS_DEBUGGER().breakOnThisThread(
        "fatalsrv::IService::ThrowFatalWithCpuContext");

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::fatalsrv
