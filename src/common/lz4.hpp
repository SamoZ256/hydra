#pragma once

#include <span>

#include "common/type_aliases.hpp"

namespace hydra {

void DecompressLZ4(std::span<const u8> src, std::span<u8> dst);

} // namespace hydra
