#pragma once

#include "horizon/services/service.hpp"

namespace Hydra::Horizon::Services::Am {

class IApmManager : public ServiceBase {
  public:
    void Request(Writers& writers, Reader& reader,
                 std::function<void(ServiceBase*)> add_service) override;

  private:
};

} // namespace Hydra::Horizon::Services::Am
