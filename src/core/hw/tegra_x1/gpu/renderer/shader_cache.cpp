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
    io::MemoryStream code_stream(
        std::span(reinterpret_cast<u8*>(descriptor.code_ptr),
                  0x1000)); // TODO: size
    shader_decomp::Decompiler decompiler;
    decompiler.Decompile(code_stream, host_descriptor.type, descriptor.state,
                         host_descriptor.backend, host_descriptor.code,
                         host_descriptor.resource_mapping);

    return renderer.CreateShader(host_descriptor);
}

u32 ShaderCache::Hash(const GuestShaderDescriptor& descriptor) {
    HashCode hash;
    hash.Add(descriptor.stage);
    hash.Add(descriptor.code_ptr);

    // Take a few samples from the code
    // TODO: this should be limited by the size of the code
    io::MemoryStream code_stream(
        std::span(reinterpret_cast<u8*>(descriptor.code_ptr),
                  0x1000)); // TODO: size
    code_stream.SeekBy(80); // Header
    for (u32 i = 0; i < 8; i++) {
        hash.Add(code_stream.Read<u8>());
        code_stream.SeekBy(17);
    }

    // Vertex state
    if (descriptor.stage == engines::ShaderStage::VertexB) {
        for (u32 i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
            const auto& vertex_attrib_state =
                descriptor.state.vertex_attrib_states[i];
            hash.Add(vertex_attrib_state.is_fixed);
            hash.Add(vertex_attrib_state.size);
            hash.Add(vertex_attrib_state.type);
        }
    }

    // Color target data types
    if (descriptor.stage == engines::ShaderStage::Fragment) {
        for (u32 i = 0; i < COLOR_TARGET_COUNT; i++) {
            const auto color_target_data_type =
                descriptor.state.color_target_data_types[i];
            hash.Add(color_target_data_type);
        }
    }

    return hash.ToHashCode();
}

void ShaderCache::DestroyElement(ShaderBase* shader) { delete shader; }

} // namespace hydra::hw::tegra_x1::gpu::renderer
