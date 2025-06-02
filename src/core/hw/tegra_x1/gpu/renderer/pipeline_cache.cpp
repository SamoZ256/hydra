#include "core/hw/tegra_x1/gpu/renderer/pipeline_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/pipeline_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

PipelineBase* PipelineCache::Create(const PipelineDescriptor& descriptor) {
    return RENDERER_INSTANCE->CreatePipeline(descriptor);
}

u64 PipelineCache::Hash(const PipelineDescriptor& descriptor) {
    u64 hash = 0;

    // Shaders
    // TODO: use the shader hash instead of the pointer?
    hash += reinterpret_cast<u64>(descriptor.shaders[0]);
    hash = std::rotl(hash, 47);
    hash += reinterpret_cast<u64>(descriptor.shaders[1]);
    hash = std::rotl(hash, 53);

    // Vertex state
    for (u32 i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
        const auto& vertex_attrib_state =
            descriptor.vertex_state.vertex_attrib_states[i];
        hash += vertex_attrib_state.buffer_id;
        hash = std::rotl(hash, 5);
        hash += vertex_attrib_state.is_fixed;
        hash = std::rotl(hash, 1);
        hash += vertex_attrib_state.offset;
        hash = std::rotl(hash, 14);
        hash += static_cast<u32>(vertex_attrib_state.size);
        hash = std::rotl(hash, 6);
        hash += static_cast<u32>(vertex_attrib_state.type);
        hash = std::rotl(hash, 3);
        hash += vertex_attrib_state.bgra;
        hash = std::rotl(hash, 1);
    }

    for (u32 i = 0; i < VERTEX_ARRAY_COUNT; i++) {
        const auto& vertex_array = descriptor.vertex_state.vertex_arrays[i];
        hash += vertex_array.enable;
        hash = std::rotl(hash, 5);
        hash += vertex_array.stride;
        hash = std::rotl(hash, 1);
        hash += vertex_array.is_per_instance;
        hash = std::rotl(hash, 14);
        hash += vertex_array.divisor;
        hash = std::rotl(hash, 6);
    }

    // Color targets
    for (u32 i = 0; i < COLOR_TARGET_COUNT; i++) {
        const auto& color_target_state = descriptor.color_target_states[i];
        hash += static_cast<u32>(color_target_state.format);
        hash = std::rotl(hash, 5);
        hash += color_target_state.blend_enabled;
        hash = std::rotl(hash, 1);
        hash += static_cast<u32>(color_target_state.rgb_op);
        hash = std::rotl(hash, 14);
        hash += static_cast<u32>(color_target_state.src_rgb_factor);
        hash = std::rotl(hash, 6);
        hash += static_cast<u32>(color_target_state.dst_rgb_factor);
        hash = std::rotl(hash, 3);
        hash += static_cast<u32>(color_target_state.alpha_op);
        hash = std::rotl(hash, 1);
        hash += static_cast<u32>(color_target_state.src_alpha_factor);
        hash = std::rotl(hash, 1);
        hash += static_cast<u32>(color_target_state.dst_alpha_factor);
        hash = std::rotl(hash, 1);
    }

    return hash;
}

void PipelineCache::DestroyElement(PipelineBase* pipeline) { delete pipeline; }

} // namespace hydra::hw::tegra_x1::gpu::renderer
