#pragma once

#include "core/hw/tegra_x1/gpu/renderer/command_buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/pipeline_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/render_pass_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/renderer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/sampler_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::null {

class CommandBuffer final : public ICommandBuffer {
  public:
    CommandBuffer();
    ~CommandBuffer() override;
};

class Sampler final : public SamplerBase {
  public:
    explicit Sampler(const SamplerDescriptor& descriptor);
    ~Sampler() override;
};

class RenderPass final : public RenderPassBase {
  public:
    explicit RenderPass(const RenderPassDescriptor& descriptor);
    ~RenderPass() override;
};

class Pipeline final : public PipelineBase {
  public:
    explicit Pipeline(const PipelineDescriptor& descriptor);
    ~Pipeline() override;
};

class Shader final : public ShaderBase {
  public:
    explicit Shader(const ShaderDescriptor& descriptor);
    ~Shader() override;
};

class Renderer : public IRenderer {
  public:
    Renderer();
    ~Renderer() override;

    // Surface
    void setSurface(void* surface) override;
    ISurfaceCompositor* acquireNextSurface() override;

    // Buffer
    BufferBase* createBuffer(u64 size) override;
    BufferBase* allocateTemporaryBuffer(const u64 size) override;
    void freeTemporaryBuffer(BufferBase* buffer) override;

    // Texture
    ITexture* createTexture(const TextureDescriptor& descriptor) override;
    void blitTexture(ICommandBuffer* command_buffer, ITextureView* src,
                     float3 src_origin, uint3 src_size, u32 src_level,
                     u32 src_layer, ITextureView* dst, float3 dst_origin,
                     uint3 dst_size, u32 dst_level, u32 dst_layer,
                     u32 level_count, u32 layer_count) override;

    // Sampler
    SamplerBase* createSampler(const SamplerDescriptor& descriptor) override;

    // Command buffer
    ICommandBuffer* createCommandBuffer() override;

    // Render pass
    RenderPassBase*
    createRenderPass(const RenderPassDescriptor& descriptor) override;
    void bindRenderPass(const RenderPassBase* render_pass) override;

    // Clear
    void clearColor(ICommandBuffer* command_buffer, u32 render_target_id,
                    u32 layer, u8 mask, const uint4 color) override;
    void clearDepth(ICommandBuffer* command_buffer, u32 layer,
                    const float value) override;
    void clearStencil(ICommandBuffer* command_buffer, u32 layer,
                      const u32 value) override;

    // Shader
    ShaderBase* createShader(const ShaderDescriptor& descriptor) override;

    // Pipeline
    PipelineBase* createPipeline(const PipelineDescriptor& descriptor) override;
    void bindPipeline(const PipelineBase* pipeline) override;

    // Depth stencil
    void setDepthTestEnabled(bool enabled) override;
    void setDepthWriteEnabled(bool enabled) override;
    void setDepthCompareOp(engines::CompareOp op) override;

    // Viewport and scissor
    void setViewport(u32 index, const Viewport& viewport) override;
    void setScissor(u32 index, const Scissor& scissor) override;

    // Resource binding
    void bindVertexBuffer(const BufferView& buffer, u32 index) override;
    void bindIndexBuffer(const BufferView& index_buffer,
                         engines::IndexType index_type) override;
    void bindUniformBuffer(const BufferView& buffer, ShaderType shader_type,
                           u32 index) override;
    void bindTexture(ITextureView* texture, SamplerBase* sampler,
                     ShaderType shader_type, u32 index) override;

    // Resource unbinding
    void unbindUniformBuffers(ShaderType shader_type) override;
    void unbindTextures(ShaderType shader_type) override;

    // Draw
    void draw(ICommandBuffer* command_buffer,
              const engines::PrimitiveType primitive_type, const u32 start,
              const u32 count, const u32 base_instance,
              const u32 instance_count) override;
    void drawIndexed(ICommandBuffer* command_buffer,
                     const engines::PrimitiveType primitive_type,
                     const u32 start, const u32 count, const u32 base_vertex,
                     const u32 base_instance,
                     const u32 instance_count) override;

  protected:
    // Capture
    void beginCapture() override;
    void endCapture() override;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::null
