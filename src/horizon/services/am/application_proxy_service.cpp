#include "horizon/services/am/application_proxy_service.hpp"

#include "horizon/cmif.hpp"
#include "horizon/kernel.hpp"
#include "horizon/services/am/application_proxy.hpp"

namespace Hydra::Horizon::Services::Am {

void ApplicationProxyService::Request(
    Writers& writers, u8* in_ptr,
    std::function<void(ServiceBase*)> add_service) {
    auto cmif_in = Cmif::read_in_header(in_ptr);

    Result* res = Cmif::write_out_header(writers.writer);

    switch (cmif_in.command_id) {
    case 0: {
        printf("OpenApplicationProxy\n");

        add_service(new ApplicationProxy());

        break;
    }
    default:
        printf("Unknown appletOE request %u\n", cmif_in.command_id);
        break;
    }

    *res = RESULT_SUCCESS;
}

} // namespace Hydra::Horizon::Services::Am
