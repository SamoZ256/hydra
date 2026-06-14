#pragma once

#include "core/horizon/services/timesrv/const.hpp"

namespace hydra::horizon::services::timesrv::internal {

void ParseTimeZoneBinary(io::IStream* stream, TimeZoneRule& out_rule);

} // namespace hydra::horizon::services::timesrv::internal
