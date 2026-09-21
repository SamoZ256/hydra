#include "core/horizon/services/pctl/parental_control_service_factory.hpp"

#include "core/horizon/services/pctl/parental_control_service.hpp"

namespace hydra::horizon::services::pctl {

DEFINE_SERVICE_COMMAND_TABLE(IParentalControlServiceFactory, 0, createService,
                             1, createServiceWithoutInitialize)

result_t IParentalControlServiceFactory::createService(RequestContext* ctx) {
    auto service = new IParentalControlService();
    addService(*ctx, service);
    return service->initialize();
}

result_t IParentalControlServiceFactory::createServiceWithoutInitialize(
    RequestContext* ctx) {
    addService(*ctx, new IParentalControlService());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::pctl
