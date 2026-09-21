#include "core/hw/tegra_x1/gpu/renderer/null/renderer.hpp"

#include "core/hw/tegra_x1/gpu/renderer/null/buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/null/surface_compositor.hpp"
#include "core/hw/tegra_x1/gpu/renderer/null/texture.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::null {

CommandBuffer::CommandBuffer() = default;
CommandBuffer::~CommandBuffer() = default;

Sampler::Sampler(const SamplerDescriptor& descriptor)
    : SamplerBase(descriptor) {}
Sampler::~Sampler() = default;

RenderPass::RenderPass(const RenderPassDescriptor& descriptor)
    : RenderPassBase(descriptor) {}
RenderPass::~RenderPass() = default;

Pipeline::Pipeline(const PipelineDescriptor& descriptor)
    : PipelineBase(descriptor) {}
Pipeline::~Pipeline() = default;

Shader::Shader(const ShaderDescriptor& descriptor) : ShaderBase(descriptor) {}
Shader::~Shader() = default;

Renderer::Renderer() = default;
Renderer::~Renderer() = default;

void Renderer::setSurface([[maybe_unused]] void* surface) {}

ISurfaceCompositor* Renderer::acquireNextSurface() {
    return new SurfaceCompositor();
}

BufferBase* Renderer::createBuffer(u64 size) { return new Buffer(size); }

BufferBase* Renderer::allocateTemporaryBuffer(const u64 size) {
    return new Buffer(size);
}

void Renderer::freeTemporaryBuffer(BufferBase* buffer) {
    auto buffer_impl = static_cast<Buffer*>(buffer);
    delete buffer_impl;
}

ITexture* Renderer::createTexture(const TextureDescriptor& descriptor) {
    return new Texture(descriptor);
}

void Renderer::blitTexture(
    [[maybe_unused]] ICommandBuffer* command_buffer,
    [[maybe_unused]] ITextureView* src, [[maybe_unused]] float3 src_origin,
    [[maybe_unused]] uint3 src_size, [[maybe_unused]] u32 src_level,
    [[maybe_unused]] u32 src_layer, [[maybe_unused]] ITextureView* dst,
    [[maybe_unused]] float3 dst_origin, [[maybe_unused]] uint3 dst_size,
    [[maybe_unused]] u32 dst_level, [[maybe_unused]] u32 dst_layer,
    [[maybe_unused]] u32 level_count, [[maybe_unused]] u32 layer_count) {}

SamplerBase* Renderer::createSampler(const SamplerDescriptor& descriptor) {
    return new Sampler(descriptor);
}

ICommandBuffer* Renderer::createCommandBuffer() { return new CommandBuffer(); }

RenderPassBase*
Renderer::createRenderPass(const RenderPassDescriptor& descriptor) {
    return new RenderPass(descriptor);
}

void Renderer::bindRenderPass(
    [[maybe_unused]] const RenderPassBase* render_pass) {}

void Renderer::clearColor([[maybe_unused]] ICommandBuffer* command_buffer,
                          [[maybe_unused]] u32 render_target_id,
                          [[maybe_unused]] u32 layer, [[maybe_unused]] u8 mask,
                          [[maybe_unused]] const uint4 color) {}

void Renderer::clearDepth([[maybe_unused]] ICommandBuffer* command_buffer,
                          [[maybe_unused]] u32 layer,
                          [[maybe_unused]] const float value) {}

void Renderer::clearStencil([[maybe_unused]] ICommandBuffer* command_buffer,
                            [[maybe_unused]] u32 layer,
                            [[maybe_unused]] const u32 value) {}

ShaderBase* Renderer::createShader(const ShaderDescriptor& descriptor) {
    return new Shader(descriptor);
}

PipelineBase* Renderer::createPipeline(const PipelineDescriptor& descriptor) {
    return new Pipeline(descriptor);
}

void Renderer::bindPipeline([[maybe_unused]] const PipelineBase* pipeline) {}

void Renderer::setDepthTestEnabled([[maybe_unused]] bool enabled) {}
void Renderer::setDepthWriteEnabled([[maybe_unused]] bool enabled) {}
void Renderer::setDepthCompareOp([[maybe_unused]] engines::CompareOp op) {}

// Viewport and scissor
void Renderer::setViewport([[maybe_unused]] u32 index,
                           [[maybe_unused]] const Viewport& viewport) {}
void Renderer::setScissor([[maybe_unused]] u32 index,
                          [[maybe_unused]] const Scissor& scissor) {}

// Resource binding
void Renderer::bindVertexBuffer([[maybe_unused]] const BufferView& buffer,
                                [[maybe_unused]] u32 index) {}
void Renderer::bindIndexBuffer([[maybe_unused]] const BufferView& index_buffer,
                               [[maybe_unused]] engines::IndexType index_type) {
}
void Renderer::bindUniformBuffer([[maybe_unused]] const BufferView& buffer,
                                 [[maybe_unused]] ShaderType shader_type,
                                 [[maybe_unused]] u32 index) {}
void Renderer::bindTexture([[maybe_unused]] ITextureView* texture,
                           [[maybe_unused]] SamplerBase* sampler,
                           [[maybe_unused]] ShaderType shader_type,
                           [[maybe_unused]] u32 index) {}

// Resource unbinding
void Renderer::unbindUniformBuffers([[maybe_unused]] ShaderType shader_type) {}
void Renderer::unbindTextures([[maybe_unused]] ShaderType shader_type) {}

// Draw
void Renderer::draw(
    [[maybe_unused]] ICommandBuffer* command_buffer,
    [[maybe_unused]] const engines::PrimitiveType primitive_type,
    [[maybe_unused]] const u32 start, [[maybe_unused]] const u32 count,
    [[maybe_unused]] const u32 base_instance,
    [[maybe_unused]] const u32 instance_count) {}
void Renderer::drawIndexed(
    [[maybe_unused]] ICommandBuffer* command_buffer,
    [[maybe_unused]] const engines::PrimitiveType primitive_type,
    [[maybe_unused]] const u32 start, [[maybe_unused]] const u32 count,
    [[maybe_unused]] const u32 base_vertex,
    [[maybe_unused]] const u32 base_instance,
    [[maybe_unused]] const u32 instance_count) {}

void Renderer::beginCapture() {}
void Renderer::endCapture() {}

} // namespace hydra::hw::tegra_x1::gpu::renderer::null
