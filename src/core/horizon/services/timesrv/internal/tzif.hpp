#pragma once

#include "core/horizon/services/timesrv/const.hpp"

namespace hydra::horizon::services::timesrv::internal {

enum class ParseTimeZoneBinaryError {
    InvalidMagic,
    InvalidBinary,
    InsufficientDataSize,
};

void ParseTimeZoneBinary(io::IStream* stream, TimeZoneRule& out_rule);

} // namespace hydra::horizon::services::timesrv::internal
