#pragma once

#include <span>

#include "ztd/type_aliases.hpp"

namespace ztd::compress {

auto decompressLz4(std::span<const u8> src, std::span<u8> dst) noexcept -> void;

} // namespace ztd::compress
