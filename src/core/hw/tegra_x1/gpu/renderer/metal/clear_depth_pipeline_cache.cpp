#include "core/hw/tegra_x1/gpu/renderer/metal/clear_depth_pipeline_cache.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

ClearDepthPipelineCache::ClearDepthPipelineCache() {
    // Source
    auto shader_source = R"(
        #include <metal_stdlib>
        using namespace metal;

        struct VertexClearDepthOut {
            float4 position [[position]];
            uint layer_id [[render_target_array_index]];
        };

        constant float2 vertices[3] = {
            float2(-1.0, -3.0),
            float2(-1.0,  1.0),
            float2( 3.0,  1.0)
        };

        struct VertexClearDepthParams {
            uint layer_id;
            float value;
        };

        vertex VertexClearDepthOut vertex_clear_depth(ushort vid [[vertex_id]], constant VertexClearDepthParams& params [[buffer(0)]]) {
            VertexClearDepthOut out;
            out.position = float4(vertices[vid], params.value, 1.0);
            out.layer_id = params.layer_id;

            return out;
        }
    )";

    // Function
    auto vertex_clear_depth =
        CreateFunctionFromSource(METAL_RENDERER_INSTANCE.GetDevice(),
                                 shader_source, "vertex_clear_depth");

    // Pipeline descriptor
    pipeline_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pipeline_descriptor->setVertexFunction(vertex_clear_depth);
    pipeline_descriptor->setInputPrimitiveTopology(
        MTL::PrimitiveTopologyClassTriangle);
}

void ClearDepthPipelineCache::Destroy() { pipeline_descriptor->release(); }

MTL::RenderPipelineState*
ClearDepthPipelineCache::Create(MTL::PixelFormat pixel_format) {
    // Pipeline
    pipeline_descriptor->setDepthAttachmentPixelFormat(pixel_format);

    NS::Error* error;
    auto pipeline = METAL_RENDERER_INSTANCE.GetDevice()->newRenderPipelineState(
        pipeline_descriptor, &error);
    if (error) {
        LOG_ERROR(MetalRenderer, "Failed to create clear depth pipeline: {}",
                  error->localizedDescription()->utf8String());
        return nullptr;
    }

    return pipeline;
}

u64 ClearDepthPipelineCache::Hash(MTL::PixelFormat pixel_format) {
    return (u64)pixel_format;
}

void ClearDepthPipelineCache::DestroyElement(
    MTL::RenderPipelineState* pipeline) {
    pipeline->release();
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
