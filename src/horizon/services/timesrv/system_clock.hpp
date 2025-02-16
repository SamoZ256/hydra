#pragma once

#include "horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::TimeSrv {

enum class SystemClockType {
    StandardUser,
    StandardNetwork,
    StandardLocal,
    EphemeralNetwork,
};

class ISystemClock : public ServiceBase {
  public:
    DEFINE_VIRTUAL_CLONE(ISystemClock)

    ISystemClock(SystemClockType type_) : type{type_} {}

  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
    SystemClockType type;
};

} // namespace Hydra::Horizon::Services::TimeSrv
