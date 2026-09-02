#include "core/hw/tegra_x1/gpu/renderer/pipeline_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/pipeline_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

PipelineBase* PipelineCache::Create(const PipelineDescriptor& descriptor) {
    return renderer.CreatePipeline(descriptor);
}

u32 PipelineCache::Hash(const PipelineDescriptor& descriptor) {
    ztd::hash::XxHash32 hash;

    // Shaders
    // TODO: use the shader hash instead of the pointer?
    hash.add(descriptor.shaders[0]);
    hash.add(descriptor.shaders[1]);

    // Vertex state

    // Vertex attributes
    for (const auto& vertex_attrib_state :
         descriptor.vertex_state.vertex_attrib_states) {
        hash.add(vertex_attrib_state.buffer_id);
        // is_fixed is in vertex shader hash
        hash.add(vertex_attrib_state.offset);
        // size and type are in vertex shader hash
        hash.add(vertex_attrib_state.bgra);
    }

    // Vertex arrays
    for (const auto& vertex_array : descriptor.vertex_state.vertex_arrays) {
        hash.add(vertex_array.enable);
        hash.add(vertex_array.stride);
        hash.add(vertex_array.is_per_instance);
        hash.add(vertex_array.divisor);
    }

    // Color state

    // Color targets
    for (const auto& color_target_state : descriptor.color_target_states) {
        hash.add(color_target_state.format);
        hash.add(color_target_state.blend_enabled);
        if (color_target_state.blend_enabled) {
            hash.add(color_target_state.rgb_op);
            hash.add(color_target_state.src_rgb_factor);
            hash.add(color_target_state.dst_rgb_factor);
            hash.add(color_target_state.alpha_op);
            hash.add(color_target_state.src_alpha_factor);
            hash.add(color_target_state.dst_alpha_factor);
        }
    }

    return hash.toHashCode();
}

void PipelineCache::DestroyElement(PipelineBase* pipeline) { delete pipeline; }

} // namespace hydra::hw::tegra_x1::gpu::renderer
