#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/blit_pipeline_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/clear_color_pipeline_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/clear_depth_pipeline_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/depth_stencil_state_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

class CommandBuffer;
class TextureView;
class Sampler;
class RenderPass;
class Pipeline;

struct CombinedTextureSampler {
    const TextureView* texture_view{nullptr};
    const Sampler* sampler{nullptr};
};

struct State {
    const RenderPass* render_pass{nullptr};
    const Pipeline* pipeline{nullptr};
    bool depth_test_enabled;
    bool depth_write_enabled;
    engines::CompareOp depth_compare_op;
    Viewport viewports[VIEWPORT_COUNT];
    Scissor scissors[VIEWPORT_COUNT];
    BufferView index_buffer;
    engines::IndexType index_type{engines::IndexType::None};
    std::array<BufferView, VERTEX_ARRAY_COUNT> vertex_buffers{};
    std::array<std::array<BufferView, CONST_BUFFER_BINDING_COUNT>,
               static_cast<usize>(ShaderType::Count)>
        uniform_buffers{};
    std::array<std::array<CombinedTextureSampler, TEXTURE_BINDING_COUNT>,
               static_cast<usize>(ShaderType::Count)>
        textures{};
    // TODO: images
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

    // Helpers
    MTL::RenderCommandEncoder*
    getRenderCommandEncoder(CommandBuffer* command_buffer) const;

    // Encoder state setting
    void setRenderPipelineState(CommandBuffer* command_buffer) const;
    void setDepthStencilState(CommandBuffer* command_buffer);
    void setVertexBuffer(CommandBuffer* command_buffer, u32 index);
    void setUniformBuffer(CommandBuffer* command_buffer, ShaderType shader_type,
                          u32 index);
    void setTexture(CommandBuffer* command_buffer, ShaderType shader_type,
                    u32 index);

  protected:
    // Capture
    void beginCapture() override;
    void endCapture() override;

  private:
    MTL::Device* device;
    MTL::CommandQueue* command_queue;

    // Caches
    DepthStencilStateCache depth_stencil_state_cache;
    BlitPipelineCache blit_pipeline_cache;
    ClearColorPipelineCache clear_color_pipeline_cache;
    ClearDepthPipelineCache clear_depth_pipeline_cache;

    // CA
    CA::MetalLayer* ca_layer{nullptr};
    CA::MetalDrawable* ca_drawable{nullptr};

    // Resources

    // Depth stencil states
    MTL::DepthStencilState* depth_stencil_state_always_and_write;

    // Samplers
    MTL::SamplerState* nearest_sampler;
    MTL::SamplerState* linear_sampler;

    // State
    State state;
    [[maybe_unused]] u32
        padding[0x100]; // HACK: for some reason, writing to some fields of the
                        // encoder_state corrupts the state

    // Helpers
    bool canDraw() const;
    void bindDrawState(CommandBuffer* command_buffer);

  public:
    GETTER(device, getDevice);
    REF_GETTER(blit_pipeline_cache, getBlitPipelineCache);
    GETTER(linear_sampler, getLinearSampler);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
