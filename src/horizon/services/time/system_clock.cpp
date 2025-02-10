#include "horizon/services/time/system_clock.hpp"

#include "horizon/cmif.hpp"

namespace Hydra::Horizon::Services::Time {

void SystemClock::Request(Writers& writers, u8* in_ptr,
                          std::function<void(ServiceBase*)> add_service) {
    auto cmif_in = Cmif::read_in_header(in_ptr);

    Result* res = Cmif::write_out_header(writers.writer);

    switch (cmif_in.command_id) {
    default:
        LOG_WARNING(HorizonServices, "Unknown time::system_clock request {}",
                    cmif_in.command_id);
        break;
    }

    *res = RESULT_SUCCESS;
}

} // namespace Hydra::Horizon::Services::Time
