#include "horizon/services/timesrv/steady_clock.hpp"

#include "horizon/cmif.hpp"

namespace Hydra::Horizon::Services::TimeSrv {

void ISteadyClock::Request(Writers& writers, u8* in_ptr,
                           std::function<void(ServiceBase*)> add_service) {
    auto cmif_in = Cmif::read_in_header(in_ptr);

    Result* res = Cmif::write_out_header(writers.writer);

    switch (cmif_in.command_id) {
    default:
        LOG_WARNING(HorizonServices, "Unknown request {}", cmif_in.command_id);
        break;
    }

    *res = RESULT_SUCCESS;
}

} // namespace Hydra::Horizon::Services::TimeSrv
