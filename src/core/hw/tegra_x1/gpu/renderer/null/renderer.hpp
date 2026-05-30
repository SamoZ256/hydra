#pragma once

#include "core/hw/tegra_x1/gpu/renderer/command_buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/pipeline_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/render_pass_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/renderer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/sampler_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::null {

class Buffer;
class TextureView;

class CommandBuffer final : public ICommandBuffer {
  public:
    CommandBuffer();
    ~CommandBuffer() override;
};

class Sampler final : public SamplerBase {
  public:
    Sampler(const SamplerDescriptor& descriptor);
    ~Sampler() override;
};

class RenderPass final : public RenderPassBase {
  public:
    RenderPass(const RenderPassDescriptor& descriptor);
    ~RenderPass() override;
};

class Pipeline final : public PipelineBase {
  public:
    Pipeline(const PipelineDescriptor& descriptor);
    ~Pipeline() override;
};

class Shader final : public ShaderBase {
  public:
    Shader(const ShaderDescriptor& descriptor);
    ~Shader() override;
};

class Renderer : public IRenderer {
  public:
    Renderer();
    ~Renderer() override;

    // Surface
    void SetSurface(void* surface) override;
    ISurfaceCompositor* AcquireNextSurface() override;

    // Buffer
    BufferBase* CreateBuffer(u64 size) override;
    BufferBase* AllocateTemporaryBuffer(const u64 size) override;
    void FreeTemporaryBuffer(BufferBase* buffer) override;

    // Texture
    ITexture* CreateTexture(const TextureDescriptor& descriptor) override;
    void BlitTexture(ICommandBuffer* command_buffer, ITextureView* src,
                     float3 src_origin, uint3 src_size, u32 src_level,
                     u32 src_layer, ITextureView* dst, float3 dst_origin,
                     uint3 dst_size, u32 dst_level, u32 dst_layer,
                     u32 level_count, u32 layer_count) override;

    // Sampler
    SamplerBase* CreateSampler(const SamplerDescriptor& descriptor) override;

    // Command buffer
    ICommandBuffer* CreateCommandBuffer() override;

    // Render pass
    RenderPassBase*
    CreateRenderPass(const RenderPassDescriptor& descriptor) override;
    void BindRenderPass(const RenderPassBase* render_pass) override;

    // Clear
    void ClearColor(ICommandBuffer* command_buffer, u32 render_target_id,
                    u32 layer, u8 mask, const uint4 color) override;
    void ClearDepth(ICommandBuffer* command_buffer, u32 layer,
                    const float value) override;
    void ClearStencil(ICommandBuffer* command_buffer, u32 layer,
                      const u32 value) override;

    // Shader
    ShaderBase* CreateShader(const ShaderDescriptor& descriptor) override;

    // Pipeline
    PipelineBase* CreatePipeline(const PipelineDescriptor& descriptor) override;
    void BindPipeline(const PipelineBase* pipeline) override;

    // Depth stencil
    void SetDepthTestEnabled(bool enabled) override;
    void SetDepthWriteEnabled(bool enabled) override;
    void SetDepthCompareOp(engines::CompareOp op) override;

    // Viewport and scissor
    void SetViewport(u32 index, const Viewport& viewport) override;
    void SetScissor(u32 index, const Scissor& scissor) override;

    // Resource binding
    void BindVertexBuffer(const BufferView& buffer, u32 index) override;
    void BindIndexBuffer(const BufferView& index_buffer,
                         engines::IndexType index_type) override;
    void BindUniformBuffer(const BufferView& buffer, ShaderType shader_type,
                           u32 index) override;
    void BindTexture(ITextureView* texture, SamplerBase* sampler,
                     ShaderType shader_type, u32 index) override;

    // Resource unbinding
    void UnbindUniformBuffers(ShaderType shader_type) override;
    void UnbindTextures(ShaderType shader_type) override;

    // Draw
    void Draw(ICommandBuffer* command_buffer,
              const engines::PrimitiveType primitive_type, const u32 start,
              const u32 count, const u32 base_instance,
              const u32 instance_count) override;
    void DrawIndexed(ICommandBuffer* command_buffer,
                     const engines::PrimitiveType primitive_type,
                     const u32 start, const u32 count, const u32 base_vertex,
                     const u32 base_instance,
                     const u32 instance_count) override;

  protected:
    // Capture
    void BeginCapture() override;
    void EndCapture() override;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::null