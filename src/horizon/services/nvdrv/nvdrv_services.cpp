#include "horizon/services/nvdrv/nvdrv_services.hpp"

#include "horizon/cmif.hpp"

namespace Hydra::Horizon::Services::NvDrv {

void INvDrvServices::Request(Readers& readers, Writers& writers,
                             std::function<void(ServiceBase*)> add_service) {
    auto cmif_in = readers.reader.Read<Cmif::InHeader>();

    Result* res = Cmif::write_out_header(writers.writer);

    switch (cmif_in.command_id) {
    default:
        LOG_WARNING(HorizonServices, "Unknown request {}", cmif_in.command_id);
        break;
    }

    *res = RESULT_SUCCESS;
}

} // namespace Hydra::Horizon::Services::NvDrv
