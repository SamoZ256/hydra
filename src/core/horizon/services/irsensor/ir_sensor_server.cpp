#include "core/horizon/services/irsensor/ir_sensor_server.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/kernel/shared_memory.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::irsensor {

DEFINE_SERVICE_COMMAND_TABLE(IIrSensorServer, 303, deactivateIrsensor, 304,
                             getIrsensorSharedMemoryHandle, 311,
                             getNpadIrCameraHandle, 319,
                             activateIrsensorWithFunctionLevel)

result_t IIrSensorServer::getIrsensorSharedMemoryHandle(
    System* system, kernel::Process* process, u64 aruid,
    OutHandle<HandleAttr::Copy> out_handle) {
    (void)aruid;
    out_handle = process->addHandle(
        system->getOs().getIrSensorManager().getSharedMemory());
    return RESULT_SUCCESS;
}

result_t IIrSensorServer::getNpadIrCameraHandle(hid::NpadIdType npad_id,
                                                IrCameraHandle* out_handle) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "npad ID: {}", npad_id);

    // HACK
    *out_handle = {
        .player_number = 0,
        .device_type = 0,
    };
    return RESULT_SUCCESS;
}

result_t IIrSensorServer::activateIrsensorWithFunctionLevel(
    PackedFunctionLevel function_level, u64 aruid) {
    (void)aruid;
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "function level: {}", function_level);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::irsensor
