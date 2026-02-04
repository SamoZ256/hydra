#include "core/hw/tegra_x1/gpu/renderer/metal/command_buffer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

CommandBuffer::CommandBuffer(MTL::CommandQueue* command_queue) {
    TMP_AUTORELEASE_POOL_BEGIN();
    command_buffer = command_queue->commandBuffer()->retain();
    TMP_AUTORELEASE_POOL_END();
}

CommandBuffer::~CommandBuffer() {
    EndEncoding();
    command_buffer->commit();
    command_buffer->release();
}

MTL::RenderCommandEncoder* CommandBuffer::GetRenderCommandEncoder(
    MTL::RenderPassDescriptor* render_pass_descriptor) {
    if (render_pass_descriptor == encoder_state.render_pass)
        return GetRenderCommandEncoderUnchecked();

    encoder_state.render_pass = render_pass_descriptor;
    encoder_state.render = {};

    return CreateRenderCommandEncoder(render_pass_descriptor);
}

MTL::RenderCommandEncoder* CommandBuffer::CreateRenderCommandEncoder(
    MTL::RenderPassDescriptor* render_pass_descriptor) {
    EndEncoding();

    TMP_AUTORELEASE_POOL_BEGIN();
    command_encoder =
        command_buffer->renderCommandEncoder(render_pass_descriptor)->retain();
    TMP_AUTORELEASE_POOL_END();

    encoder_type = EncoderType::Render;
    encoder_state.render_pass = render_pass_descriptor;

    return GetRenderCommandEncoderUnchecked();
}

MTL::BlitCommandEncoder* CommandBuffer::GetBlitCommandEncoder() {
    if (encoder_type == EncoderType::Blit)
        return GetBlitCommandEncoderUnchecked();

    EndEncoding();

    TMP_AUTORELEASE_POOL_BEGIN();
    command_encoder = command_buffer->blitCommandEncoder()->retain();
    TMP_AUTORELEASE_POOL_END();

    encoder_type = EncoderType::Blit;

    return GetBlitCommandEncoderUnchecked();
}

void CommandBuffer::EndEncoding() {
    if (encoder_type == EncoderType::None)
        return;

    command_encoder->endEncoding();
    command_encoder->release();
    command_encoder = nullptr;
    encoder_type = EncoderType::None;

    // Reset the render pass
    encoder_state.render_pass = nullptr;
}

void CommandBuffer::SetRenderPipelineState(MTL::RenderPipelineState* pipeline) {
    auto& bound_pipeline = encoder_state.render.pipeline;
    if (pipeline == bound_pipeline)
        return;

    GetRenderCommandEncoderUnchecked()->setRenderPipelineState(pipeline);
    bound_pipeline = pipeline;
}

void CommandBuffer::SetDepthStencilState(
    MTL::DepthStencilState* depth_stencil_state) {
    auto& bound_depth_stencil_state = encoder_state.render.depth_stencil_state;
    if (depth_stencil_state == bound_depth_stencil_state)
        return;

    GetRenderCommandEncoderUnchecked()->setDepthStencilState(
        depth_stencil_state);
    bound_depth_stencil_state = depth_stencil_state;
}

void CommandBuffer::SetCullMode(MTL::CullMode cull_mode) {
    auto& bound_cull_mode = encoder_state.render.cull_mode;
    if (cull_mode == bound_cull_mode)
        return;

    GetRenderCommandEncoderUnchecked()->setCullMode(cull_mode);
    bound_cull_mode = cull_mode;
}

void CommandBuffer::SetFrontFaceWinding(MTL::Winding front_face_winding) {
    auto& bound_front_face_winding = encoder_state.render.front_face_winding;
    if (front_face_winding == bound_front_face_winding)
        return;

    GetRenderCommandEncoderUnchecked()->setFrontFacingWinding(
        front_face_winding);
    bound_front_face_winding = front_face_winding;
}

void CommandBuffer::SetBuffer(MTL::Buffer* buffer, u64 offset,
                              ShaderType shader_type, u32 index) {
    ASSERT_DEBUG(index < BUFFER_COUNT, MetalRenderer, "Invalid buffer index {}",
                 index);

    auto& bound_buffer =
        encoder_state.render.buffers[static_cast<u32>(shader_type)][index];
    if (buffer == bound_buffer.buffer && offset == bound_buffer.offset)
        return;

    // TODO: fast path for offset only change

    switch (shader_type) {
    case ShaderType::Vertex:
        GetRenderCommandEncoderUnchecked()->setVertexBuffer(buffer, offset,
                                                            index);
        break;
    case ShaderType::Fragment:
        GetRenderCommandEncoderUnchecked()->setFragmentBuffer(buffer, offset,
                                                              index);
        break;
    default:
        LOG_ERROR(MetalRenderer, "Invalid shader type {}", shader_type);
        break;
    }
    bound_buffer.buffer = buffer;
    bound_buffer.offset = offset;
}

void CommandBuffer::SetTexture(MTL::Texture* texture, ShaderType shader_type,
                               u32 index) {
    ASSERT_DEBUG(index < TEXTURE_COUNT, MetalRenderer,
                 "Invalid texture index {}", index);

    auto& bound_texture =
        encoder_state.render.textures[static_cast<u32>(shader_type)][index];
    if (texture == bound_texture)
        return;

    switch (shader_type) {
    case ShaderType::Vertex:
        GetRenderCommandEncoderUnchecked()->setVertexTexture(texture, index);
        break;
    case ShaderType::Fragment:
        GetRenderCommandEncoderUnchecked()->setFragmentTexture(texture, index);
        break;
    default:
        LOG_ERROR(MetalRenderer, "Invalid shader type {}", shader_type);
        break;
    }
    bound_texture = texture;
}

void CommandBuffer::SetSampler(MTL::SamplerState* sampler,
                               ShaderType shader_type, u32 index) {
    ASSERT_DEBUG(index < TEXTURE_COUNT, MetalRenderer,
                 "Invalid texture index {}", index);

    auto& bound_sampler =
        encoder_state.render.samplers[static_cast<u32>(shader_type)][index];
    if (sampler == bound_sampler)
        return;

    switch (shader_type) {
    case ShaderType::Vertex:
        GetRenderCommandEncoderUnchecked()->setVertexSamplerState(sampler,
                                                                  index);
        break;
    case ShaderType::Fragment:
        GetRenderCommandEncoderUnchecked()->setFragmentSamplerState(sampler,
                                                                    index);
        break;
    default:
        LOG_ERROR(MetalRenderer, "Invalid shader type {}", shader_type);
        break;
    }
    bound_sampler = sampler;
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
