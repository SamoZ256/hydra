#include "horizon/services/pctl/ipc/parental_control_service_factory.hpp"
#include "horizon/services/pctl/ipc/parental_control_service.hpp"

namespace Hydra::Horizon::Services::Pctl::Ipc {

DEFINE_SERVICE_COMMAND_TABLE(IParentalControlServiceFactory, 0,
                             CreateParentalControlService)

void IParentalControlServiceFactory::CreateParentalControlService(
    REQUEST_COMMAND_PARAMS) {
    add_service(new IParentalControlService());
}

} // namespace Hydra::Horizon::Services::Pctl::Ipc
