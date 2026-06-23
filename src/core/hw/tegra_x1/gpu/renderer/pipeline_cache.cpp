#include "core/hw/tegra_x1/gpu/renderer/pipeline_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/pipeline_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

PipelineBase* PipelineCache::Create(const PipelineDescriptor& descriptor) {
    return renderer.CreatePipeline(descriptor);
}

u32 PipelineCache::Hash(const PipelineDescriptor& descriptor) {
    HashCode hash;

    // Shaders
    // TODO: use the shader hash instead of the pointer?
    hash.Add(descriptor.shaders[0]);
    hash.Add(descriptor.shaders[1]);

    // Vertex state

    // Vertex attributes
    for (const auto& vertex_attrib_state :
         descriptor.vertex_state.vertex_attrib_states) {
        hash.Add(vertex_attrib_state.buffer_id);
        // is_fixed is in vertex shader hash
        hash.Add(vertex_attrib_state.offset);
        // size and type are in vertex shader hash
        hash.Add(vertex_attrib_state.bgra);
    }

    // Vertex arrays
    for (const auto& vertex_array : descriptor.vertex_state.vertex_arrays) {
        hash.Add(vertex_array.enable);
        hash.Add(vertex_array.stride);
        hash.Add(vertex_array.is_per_instance);
        hash.Add(vertex_array.divisor);
    }

    // Color state

    // Color targets
    for (const auto& color_target_state : descriptor.color_target_states) {
        hash.Add(color_target_state.format);
        hash.Add(color_target_state.blend_enabled);
        if (color_target_state.blend_enabled) {
            hash.Add(color_target_state.rgb_op);
            hash.Add(color_target_state.src_rgb_factor);
            hash.Add(color_target_state.dst_rgb_factor);
            hash.Add(color_target_state.alpha_op);
            hash.Add(color_target_state.src_alpha_factor);
            hash.Add(color_target_state.dst_alpha_factor);
        }
    }

    return hash.ToHashCode();
}

void PipelineCache::DestroyElement(PipelineBase* pipeline) { delete pipeline; }

} // namespace hydra::hw::tegra_x1::gpu::renderer
