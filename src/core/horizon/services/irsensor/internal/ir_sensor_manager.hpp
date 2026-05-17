#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::irsensor::internal {

class IrSensorManager {
  public:
    IrSensorManager(System& system);

  private:
    kernel::SharedMemory* shared_mem;

  public:
    GETTER(shared_mem, GetSharedMemory);
};

} // namespace hydra::horizon::services::irsensor::internal
