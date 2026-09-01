#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {
struct GuestShaderState;
}

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

class ObserverBase;
class BuilderBase;

void Decompile(ztd::io::MemoryStream& code_stream, const ShaderType type,
               const GuestShaderState& state, ShaderBackend& out_backend,
               std::vector<u8>& out_code,
               ResourceMapping& out_resource_mapping);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp
