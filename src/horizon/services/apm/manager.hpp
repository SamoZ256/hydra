#pragma once

#include "horizon/services/service.hpp"

namespace Hydra::Horizon::Services::Apm {

class Manager : public ServiceBase {
  public:
    void Request(Writers& writers, u8* in_ptr,
                 std::function<void(ServiceBase*)> add_service) override;

  private:
};

} // namespace Hydra::Horizon::Services::Apm
