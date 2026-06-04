#include "core/horizon/services/timesrv/internal/tzif.hpp"

namespace hydra::horizon::services::timesrv::internal {

void ParseTimeZoneBinary(io::IStream* stream, TimeZoneRule& out_rule) {
    LOG_FUNC_NOT_IMPLEMENTED(Services);

    // HACK
    (void)stream;
    out_rule = {};
    std::memcpy(out_rule.chars, "UTC", 4);
}

} // namespace hydra::horizon::services::timesrv::internal
