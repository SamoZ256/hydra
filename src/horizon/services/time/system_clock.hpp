#pragma once

#include "horizon/services/service.hpp"

namespace Hydra::Horizon::Services::Time {

enum class SystemClockType {
    StandardUser,
    StandardNetwork,
    StandardLocal,
    EphemeralNetwork,
};

class SystemClock : public ServiceBase {
  public:
    void Request(Writers& writers, u8* in_ptr,
                 std::function<void(ServiceBase*)> add_service) override;

  private:
    SystemClockType type;
};

} // namespace Hydra::Horizon::Services::Time
