#pragma once

#include "horizon/services/service.hpp"

namespace Hydra::Horizon::Services {

class DomainService : public ServiceBase {
  public:
    void Request(Writers& writers, Reader& reader,
                 std::function<void(ServiceBase*)> add_service) override;

    Handle AddObject(ServiceBase* object) {
        Handle handle = object_pool.size();
        object_pool.push_back(object);
        object->SetHandle(handle);

        return handle;
    }

  private:
    std::vector<ServiceBase*> object_pool;
};

} // namespace Hydra::Horizon::Services
