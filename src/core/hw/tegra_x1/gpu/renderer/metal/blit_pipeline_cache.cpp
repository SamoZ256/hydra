#include "core/hw/tegra_x1/gpu/renderer/metal/blit_pipeline_cache.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

BlitPipelineCache::BlitPipelineCache() {
    // Source
    auto shader_source = R"(
        #include <metal_stdlib>
        using namespace metal;

        struct VertexBlitOut {
            float4 position [[position]];
            uint layer_id [[render_target_array_index]];
            float2 tex_coord;
        };

        constant float2 vertices[3] = {
            float2(-1.0, -3.0),
            float2(-1.0,  1.0),
            float2( 3.0,  1.0)
        };

        vertex VertexBlitOut vertex_blit(ushort vid [[vertex_id]], constant uint& layer_id [[buffer(0)]]) {
            VertexBlitOut out;
            out.position = float4(vertices[vid], 0.0, 1.0);
            out.layer_id = layer_id;
            out.tex_coord = vertices[vid] * 0.5 + 0.5;
            out.tex_coord.y = 1.0 - out.tex_coord.y;

            return out;
        }
    )";

    // Function
    auto vertex_blit = CreateFunctionFromSource(
        METAL_RENDERER_INSTANCE.GetDevice(), shader_source, "vertex_blit");

    // Pipeline descriptor
    pipeline_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pipeline_descriptor->setVertexFunction(vertex_blit);
    pipeline_descriptor->setInputPrimitiveTopology(
        MTL::PrimitiveTopologyClassTriangle);
}

void BlitPipelineCache::Destroy() { pipeline_descriptor->release(); }

MTL::RenderPipelineState*
BlitPipelineCache::Create(const BlitPipelineDescriptor& descriptor) {
    // Source
    auto shader_source = R"(
        #include <metal_stdlib>
        using namespace metal;

        struct VertexBlitOut {
            float4 position [[position]];
            uint layer_id [[render_target_array_index]];
            float2 tex_coord;
        };

        struct BlitParams {
            float2 src_offset;
            float2 src_scale;
        };

        // TODO: choose the correct color data type
        fragment float4 fragment_blit(VertexBlitOut in [[stage_in]], constant BlitParams& params [[buffer(0)]], texture2d<float> tex [[texture(0)]], sampler samplr [[sampler(0)]]) {
            float2 tex_coord = params.src_offset + in.tex_coord * params.src_scale;
            return tex.sample(samplr, tex_coord);
        }
    )";

    // Function
    auto fragment_blit = CreateFunctionFromSource(
        METAL_RENDERER_INSTANCE.GetDevice(), shader_source, "fragment_blit");

    // Pipeline
    pipeline_descriptor->setFragmentFunction(fragment_blit);
    auto color_attachment = pipeline_descriptor->colorAttachments()->object(0);
    color_attachment->setPixelFormat(descriptor.pixel_format);

    fragment_blit->release();

    NS::Error* error;
    auto pipeline = METAL_RENDERER_INSTANCE.GetDevice()->newRenderPipelineState(
        pipeline_descriptor, &error);
    if (error) {
        LOG_ERROR(MetalRenderer, "Failed to create blit pipeline: {}",
                  error->localizedDescription()->utf8String());
        return nullptr;
    }

    return pipeline;
}

u64 BlitPipelineCache::Hash(const BlitPipelineDescriptor& descriptor) {
    return (u64)descriptor.pixel_format;
}

void BlitPipelineCache::DestroyElement(MTL::RenderPipelineState* pipeline) {
    pipeline->release();
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
