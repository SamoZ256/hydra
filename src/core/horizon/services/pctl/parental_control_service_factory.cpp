#include "core/horizon/services/pctl/parental_control_service_factory.hpp"

#include "core/horizon/services/pctl/parental_control_service.hpp"

namespace hydra::horizon::services::pctl {

DEFINE_SERVICE_COMMAND_TABLE(IParentalControlServiceFactory, 0, CreateService,
                             1, CreateServiceWithoutInitialize)

result_t
IParentalControlServiceFactory::CreateService(add_service_fn_t add_service) {
    auto service = new IParentalControlService();
    add_service(service);
    return service->Initialize();
}

result_t IParentalControlServiceFactory::CreateServiceWithoutInitialize(
    add_service_fn_t add_service) {
    add_service(new IParentalControlService());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::pctl
