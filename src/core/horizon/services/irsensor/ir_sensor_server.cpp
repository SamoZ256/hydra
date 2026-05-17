#include "core/horizon/services/irsensor/ir_sensor_server.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/kernel/shared_memory.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::irsensor {

DEFINE_SERVICE_COMMAND_TABLE(IIrSensorServer, 303, DeactivateIrsensor, 304,
                             GetIrsensorSharedMemoryHandle, 311,
                             GetNpadIrCameraHandle, 319,
                             ActivateIrsensorWithFunctionLevel)

result_t IIrSensorServer::GetIrsensorSharedMemoryHandle(
    System* system, kernel::Process* process, u64 aruid,
    OutHandle<HandleAttr::Copy> out_handle) {
    (void)aruid;
    out_handle = process->AddHandle(
        system->GetOS().GetIrSensorManager().GetSharedMemory());
    return RESULT_SUCCESS;
}

result_t IIrSensorServer::GetNpadIrCameraHandle(hid::NpadIdType npad_id,
                                                IrCameraHandle* out_handle) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "npad ID: {}", npad_id);

    // HACK
    *out_handle = {
        .player_number = 0,
        .device_type = 0,
    };
    return RESULT_SUCCESS;
}

result_t IIrSensorServer::ActivateIrsensorWithFunctionLevel(
    PackedFunctionLevel function_level, u64 aruid) {
    (void)aruid;
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "function level: {}", function_level);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::irsensor
