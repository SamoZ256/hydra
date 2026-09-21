#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::irsensor::internal {

class IrSensorManager {
  public:
    explicit IrSensorManager(System& system);

  private:
    kernel::SharedMemory* shared_mem;

  public:
    GETTER(shared_mem, getSharedMemory);
};

} // namespace hydra::horizon::services::irsensor::internal
