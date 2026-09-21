#pragma once

#include "core/horizon/services/timesrv/const.hpp"

namespace hydra::horizon::services::timesrv::internal {

void parseTimeZoneBinary(ztd::io::IStream* stream, TimeZoneRule& out_rule);

} // namespace hydra::horizon::services::timesrv::internal
