#include "core/hw/tegra_x1/gpu/renderer/shader_cache.hpp"

#include "core/hw/tegra_x1/gpu/engines/3d.hpp"
#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decompiler.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

ShaderBase* ShaderCache::Create(const GuestShaderDescriptor& descriptor) {
    ShaderDescriptor host_descriptor{};
    host_descriptor.type = engines::to_renderer_shader_type(descriptor.stage);

    // Decompile
    Reader code_reader(reinterpret_cast<u8*>(descriptor.code_ptr),
                       0x1000); // TODO: size
    shader_decomp::Decompiler decompiler;
    decompiler.Decompile(code_reader, host_descriptor.type, descriptor.state,
                         host_descriptor.backend, host_descriptor.code,
                         host_descriptor.resource_mapping);

    return RENDERER_INSTANCE->CreateShader(host_descriptor);
}

u64 ShaderCache::Hash(const GuestShaderDescriptor& descriptor) {
    u64 hash = 0;
    hash += static_cast<u64>(descriptor.stage);
    hash = rotl(hash, 37);
    hash += descriptor.code_ptr;

    // TODO: vertex attribute states
    // TODO: color target formats

    return hash;
}

void ShaderCache::DestroyElement(ShaderBase* shader) { delete shader; }

} // namespace hydra::hw::tegra_x1::gpu::renderer
