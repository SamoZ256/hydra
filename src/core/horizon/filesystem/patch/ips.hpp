#pragma once

#include "core/horizon/filesystem/patch/patch.hpp"

namespace hydra::horizon::filesystem::patch {

std::optional<Patch> parseIps(std::span<const u8> bytes,
                              const BuildId& build_id,
                              const std::string_view name);

} // namespace hydra::horizon::filesystem::patch
