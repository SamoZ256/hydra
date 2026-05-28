#include "core/hw/tegra_x1/gpu/renderer/null/renderer.hpp"

#include "core/hw/tegra_x1/gpu/renderer/null/buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/null/surface_compositor.hpp"
#include "core/hw/tegra_x1/gpu/renderer/null/texture.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::null {

CommandBuffer::CommandBuffer() {}
CommandBuffer::~CommandBuffer() {}

Sampler::Sampler(const SamplerDescriptor& descriptor)
    : SamplerBase(descriptor) {}
Sampler::~Sampler() {}

RenderPass::RenderPass(const RenderPassDescriptor& descriptor)
    : RenderPassBase(descriptor) {}
RenderPass::~RenderPass() {}

Pipeline::Pipeline(const PipelineDescriptor& descriptor)
    : PipelineBase(descriptor) {}
Pipeline::~Pipeline() {}

Shader::Shader(const ShaderDescriptor& descriptor) : ShaderBase(descriptor) {}
Shader::~Shader() {}

Renderer::Renderer() {}
Renderer::~Renderer() {}

void Renderer::SetSurface([[maybe_unused]] void* surface) {}

ISurfaceCompositor* Renderer::AcquireNextSurface() {
    return new SurfaceCompositor();
}

BufferBase* Renderer::CreateBuffer(u64 size) { return new Buffer(size); }

BufferBase* Renderer::AllocateTemporaryBuffer(const u64 size) {
    return new Buffer(size);
}

void Renderer::FreeTemporaryBuffer(BufferBase* buffer) {
    auto buffer_impl = static_cast<Buffer*>(buffer);
    delete buffer_impl;
}

ITexture* Renderer::CreateTexture(const TextureDescriptor& descriptor) {
    return new Texture(descriptor);
}

void Renderer::BlitTexture(
    [[maybe_unused]] ICommandBuffer* command_buffer,
    [[maybe_unused]] ITextureView* src, [[maybe_unused]] float3 src_origin,
    [[maybe_unused]] uint3 src_size, [[maybe_unused]] u32 src_level,
    [[maybe_unused]] u32 src_layer, [[maybe_unused]] ITextureView* dst,
    [[maybe_unused]] float3 dst_origin, [[maybe_unused]] uint3 dst_size,
    [[maybe_unused]] u32 dst_level, [[maybe_unused]] u32 dst_layer,
    [[maybe_unused]] u32 level_count, [[maybe_unused]] u32 layer_count) {}

SamplerBase* Renderer::CreateSampler(const SamplerDescriptor& descriptor) {
    return new Sampler(descriptor);
}

ICommandBuffer* Renderer::CreateCommandBuffer() { return new CommandBuffer(); }

RenderPassBase*
Renderer::CreateRenderPass(const RenderPassDescriptor& descriptor) {
    return new RenderPass(descriptor);
}

void Renderer::BindRenderPass(
    [[maybe_unused]] const RenderPassBase* render_pass) {}

void Renderer::ClearColor([[maybe_unused]] ICommandBuffer* command_buffer,
                          [[maybe_unused]] u32 render_target_id,
                          [[maybe_unused]] u32 layer, [[maybe_unused]] u8 mask,
                          [[maybe_unused]] const uint4 color) {}

void Renderer::ClearDepth([[maybe_unused]] ICommandBuffer* command_buffer,
                          [[maybe_unused]] u32 layer,
                          [[maybe_unused]] const float value) {}

void Renderer::ClearStencil([[maybe_unused]] ICommandBuffer* command_buffer,
                            [[maybe_unused]] u32 layer,
                            [[maybe_unused]] const u32 value) {}

ShaderBase* Renderer::CreateShader(const ShaderDescriptor& descriptor) {
    return new Shader(descriptor);
}

PipelineBase* Renderer::CreatePipeline(const PipelineDescriptor& descriptor) {
    return new Pipeline(descriptor);
}

void Renderer::BindPipeline([[maybe_unused]] const PipelineBase* pipeline) {}

void Renderer::SetDepthTestEnabled([[maybe_unused]] bool enabled) {}
void Renderer::SetDepthWriteEnabled([[maybe_unused]] bool enabled) {}
void Renderer::SetDepthCompareOp([[maybe_unused]] engines::CompareOp op) {}

// Viewport and scissor
void Renderer::SetViewport([[maybe_unused]] u32 index,
                           [[maybe_unused]] const Viewport& viewport) {}
void Renderer::SetScissor([[maybe_unused]] u32 index,
                          [[maybe_unused]] const Scissor& scissor) {}

// Resource binding
void Renderer::BindVertexBuffer([[maybe_unused]] const BufferView& buffer,
                                [[maybe_unused]] u32 index) {}
void Renderer::BindIndexBuffer([[maybe_unused]] const BufferView& index_buffer,
                               [[maybe_unused]] engines::IndexType index_type) {
}
void Renderer::BindUniformBuffer([[maybe_unused]] const BufferView& buffer,
                                 [[maybe_unused]] ShaderType shader_type,
                                 [[maybe_unused]] u32 index) {}
void Renderer::BindTexture([[maybe_unused]] ITextureView* texture,
                           [[maybe_unused]] SamplerBase* sampler,
                           [[maybe_unused]] ShaderType shader_type,
                           [[maybe_unused]] u32 index) {}

// Resource unbinding
void Renderer::UnbindUniformBuffers([[maybe_unused]] ShaderType shader_type) {}
void Renderer::UnbindTextures([[maybe_unused]] ShaderType shader_type) {}

// Draw
void Renderer::Draw(
    [[maybe_unused]] ICommandBuffer* command_buffer,
    [[maybe_unused]] const engines::PrimitiveType primitive_type,
    [[maybe_unused]] const u32 start, [[maybe_unused]] const u32 count,
    [[maybe_unused]] const u32 base_instance,
    [[maybe_unused]] const u32 instance_count) {}
void Renderer::DrawIndexed(
    [[maybe_unused]] ICommandBuffer* command_buffer,
    [[maybe_unused]] const engines::PrimitiveType primitive_type,
    [[maybe_unused]] const u32 start, [[maybe_unused]] const u32 count,
    [[maybe_unused]] const u32 base_vertex,
    [[maybe_unused]] const u32 base_instance,
    [[maybe_unused]] const u32 instance_count) {}

void Renderer::BeginCapture() {}
void Renderer::EndCapture() {}

} // namespace hydra::hw::tegra_x1::gpu::renderer::null