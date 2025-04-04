#include "hw/tegra_x1/gpu/renderer/metal/pipeline.hpp"

#include "hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/renderer.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/shader.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer::Metal {

Pipeline::Pipeline(const PipelineDescriptor& descriptor)
    : PipelineBase(descriptor) {
    MTL::RenderPipelineDescriptor* pipeline_descriptor =
        MTL::RenderPipelineDescriptor::alloc()->init();

    // Shaders
    const auto vertex_shader =
        static_cast<Shader*>(descriptor.shaders[u32(ShaderType::Vertex)]);
    const auto fragment_shader =
        static_cast<Shader*>(descriptor.shaders[u32(ShaderType::Fragment)]);
    pipeline_descriptor->setVertexFunction(vertex_shader->GetFunction());
    pipeline_descriptor->setFragmentFunction(fragment_shader->GetFunction());

    // Vertex state

    // Vertex attribute states
    for (u32 i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
        const auto& vertex_attrib_state =
            descriptor.vertex_state.vertex_attrib_states[i];
        if (vertex_attrib_state.type == Engines::VertexAttribType::None)
            continue;

        // HACK: how are attributes disabled?
        if (vertex_attrib_state.is_fixed)
            continue;

        ASSERT_DEBUG(i != 31, MetalRenderer,
                     "Vertex attribute 31 is not available on Metal");

        // LOG_DEBUG(MetalRenderer,
        //           "Buffer ID: {}, is fixed: {}, offset: 0x{:08x}",
        //           vertex_attrib_state.buffer_id,
        //           vertex_attrib_state.is_fixed, vertex_attrib_state.offset);

        auto vertex_attrib_descriptor =
            pipeline_descriptor->vertexDescriptor()->attributes()->object(i);
        vertex_attrib_descriptor->setBufferIndex(
            GetVertexBufferIndex(vertex_attrib_state.buffer_id));
        // TODO: is fixed
        vertex_attrib_descriptor->setOffset(vertex_attrib_state.offset);
        vertex_attrib_descriptor->setFormat(to_mtl_vertex_format(
            vertex_attrib_state.type, vertex_attrib_state.size,
            vertex_attrib_state.bgra));
    }

    // Vertex arrays
    for (u32 i = 0; i < VERTEX_ARRAY_COUNT; i++) {
        const auto& vertex_array = descriptor.vertex_state.vertex_arrays[i];
        if (!vertex_array.enable)
            continue;

        auto vertex_array_descriptor =
            pipeline_descriptor->vertexDescriptor()->layouts()->object(
                GetVertexBufferIndex(i));
        vertex_array_descriptor->setStride(vertex_array.stride);
        vertex_array_descriptor->setStepFunction(
            vertex_array.is_per_instance ? MTL::VertexStepFunctionPerInstance
                                         : MTL::VertexStepFunctionPerVertex);
        // TODO: divisor
        // vertex_array_descriptor->setStepRate(1);
    }

    // HACK
    pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(
        MTL::PixelFormatBGRA8Unorm);

    // Pipeline
    NS::Error* error;
    pipeline = Renderer::GetInstance().GetDevice()->newRenderPipelineState(
        pipeline_descriptor, &error);
    pipeline_descriptor->release();
    if (error) {
        LOG_ERROR(MetalRenderer, "Failed to create pipeline: {}",
                  error->localizedDescription()->utf8String());
        error->release(); // TODO: release?
        return;
    }
}

Pipeline::~Pipeline() { pipeline->release(); }

} // namespace Hydra::HW::TegraX1::GPU::Renderer::Metal
