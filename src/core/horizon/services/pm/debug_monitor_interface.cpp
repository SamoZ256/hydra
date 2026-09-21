#include "core/horizon/services/pm/debug_monitor_interface.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::pm {

// NOTE: the commands are offset by +1 on pre-5.0.0
DEFINE_SERVICE_COMMAND_TABLE(IDebugMonitorInterface, 2, getProcessId, 4,
                             getApplicationProcessId)

result_t IDebugMonitorInterface::getProcessId(u64 program_id, u64* out_pid) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "program ID: 0x{:016X}", program_id);

    // HACK
    *out_pid = 0x1029384756;
    return RESULT_SUCCESS;
}

result_t
IDebugMonitorInterface::getApplicationProcessId(kernel::Process* process,
                                                u64* out_pid) {
    return getProcessId(process->getTitleId(), out_pid);
}

} // namespace hydra::horizon::services::pm
