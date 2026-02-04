#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/blit_pipeline_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/clear_color_pipeline_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/clear_depth_pipeline_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/depth_stencil_state_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/renderer_base.hpp"

#define METAL_RENDERER_INSTANCE                                                \
    hw::tegra_x1::gpu::renderer::metal::Renderer::GetInstance()

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

class CommandBuffer;
class Buffer;
class Texture;
class Sampler;
class RenderPass;
class Pipeline;

struct CombinedTextureSampler {
    const Texture* texture{nullptr};
    const Sampler* sampler{nullptr};
};

struct State {
    const RenderPass* render_pass{nullptr};
    Viewport viewports[VIEWPORT_COUNT];
    Scissor scissors[VIEWPORT_COUNT];
    const Pipeline* pipeline{nullptr};
    BufferView index_buffer{};
    engines::IndexType index_type{engines::IndexType::None};
    std::array<BufferView, VERTEX_ARRAY_COUNT> vertex_buffers{};
    std::array<std::array<BufferView, CONST_BUFFER_BINDING_COUNT>,
               usize(ShaderType::Count)>
        uniform_buffers{};
    std::array<std::array<CombinedTextureSampler, TEXTURE_BINDING_COUNT>,
               usize(ShaderType::Count)>
        textures{};
    // TODO: images
};

class Renderer : public RendererBase {
  public:
    static Renderer& GetInstance();

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
    TextureBase* CreateTexture(const TextureDescriptor& descriptor) override;

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

    // Viewport and scissor
    void SetViewport(u32 index, const Viewport& viewport) override;
    void SetScissor(u32 index, const Scissor& scissor) override;

    // Shader
    ShaderBase* CreateShader(const ShaderDescriptor& descriptor) override;

    // Pipeline
    PipelineBase* CreatePipeline(const PipelineDescriptor& descriptor) override;
    void BindPipeline(const PipelineBase* pipeline) override;

    // Resource binding
    void BindVertexBuffer(const BufferView& buffer, u32 index) override;
    void BindIndexBuffer(const BufferView& index_buffer,
                         engines::IndexType index_type) override;
    void BindUniformBuffer(const BufferView& buffer, ShaderType shader_type,
                           u32 index) override;
    void BindTexture(TextureBase* texture, SamplerBase* sampler,
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

    // Helpers
    MTL::RenderCommandEncoder*
    GetRenderCommandEncoder(CommandBuffer* command_buffer);

    // Encoder state setting
    void SetRenderPipelineState(CommandBuffer* command_buffer);
    void SetDepthStencilState(CommandBuffer* command_buffer);
    void SetVertexBuffer(CommandBuffer* command_buffer, u32 index);
    void SetUniformBuffer(CommandBuffer* command_buffer, ShaderType shader_type,
                          u32 index);
    void SetTexture(CommandBuffer* command_buffer, ShaderType shader_type,
                    u32 index);

    void BlitTexture(CommandBuffer* command_buffer, MTL::Texture* src,
                     const float3 src_origin, const usize3 src_size,
                     MTL::Texture* dst, const u32 dst_layer,
                     const float3 dst_origin, const usize3 dst_size);

  protected:
    // Capture
    void BeginCapture() override;
    void EndCapture() override;

  private:
    MTL::Device* device;
    MTL::CommandQueue* command_queue;

    CA::MetalLayer* layer{nullptr};
    CA::MetalDrawable* drawable{nullptr};

    // Resources

    // Depth stencil states
    MTL::DepthStencilState* depth_stencil_state_always_and_write;

    // Samplers
    MTL::SamplerState* nearest_sampler;
    MTL::SamplerState* linear_sampler;

    // Caches
    DepthStencilStateCache* depth_stencil_state_cache;
    BlitPipelineCache* blit_pipeline_cache;
    ClearColorPipelineCache* clear_color_pipeline_cache;
    ClearDepthPipelineCache* clear_depth_pipeline_cache;

    // Null
    MTL::Texture* null_texture;

    // State
    State state;
    [[maybe_unused]] u32
        padding[0x100]; // HACK: for some reason, writing to some fields of the
                        // encoder_state corrupts the state

    // Helpers
    bool CanDraw();
    void BindDrawState(CommandBuffer* command_buffer);

  public:
    GETTER(device, GetDevice);
    GETTER(blit_pipeline_cache, GetBlitPipelineCache);
    GETTER(linear_sampler, GetLinearSampler);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
