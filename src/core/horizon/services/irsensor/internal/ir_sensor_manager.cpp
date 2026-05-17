#include "core/horizon/services/irsensor/internal/ir_sensor_manager.hpp"

#include "core/horizon/kernel/shared_memory.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::irsensor::internal {

IrSensorManager::IrSensorManager(System& system)
    : shared_mem{new kernel::SharedMemory(system.GetCpu(), 0x8000)} {}

} // namespace hydra::horizon::services::irsensor::internal
