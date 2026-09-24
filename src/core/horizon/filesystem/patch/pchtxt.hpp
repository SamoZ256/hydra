#pragma once

#include "core/horizon/filesystem/patch/patch.hpp"

namespace hydra::horizon::filesystem::patch {

std::optional<Patch> parsePchtxt(std::string_view text,
                                 const std::string_view name);

} // namespace hydra::horizon::filesystem::patch
