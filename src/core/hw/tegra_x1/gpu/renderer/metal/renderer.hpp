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

class Buffer;
class Texture;
class Sampler;
class RenderPass;
class Pipeline;

enum class EncoderType {
    None,
    Render,
    Compute,
    Blit,
};

struct State {
    const RenderPass* render_pass{nullptr};
    const Pipeline* pipeline{nullptr};
    const Buffer* index_buffer{nullptr};
    engines::IndexType index_type{engines::IndexType::None};
    const Buffer* vertex_buffers[VERTEX_ARRAY_COUNT] = {nullptr};
    const Buffer* uniform_buffers[usize(ShaderType::Count)]
                                 [CONST_BUFFER_BINDING_COUNT];
    struct {
        const Texture* texture;
        const Sampler* sampler;
    } textures[usize(ShaderType::Count)][TEXTURE_BINDING_COUNT];
    // TODO: images
};

struct EncoderRenderState {
    MTL::RenderPipelineState* pipeline{nullptr};
    MTL::DepthStencilState* depth_stencil_state{nullptr};
    MTL::Buffer* buffers[usize(ShaderType::Count)][BUFFER_COUNT];
    MTL::Texture* textures[usize(ShaderType::Count)][TEXTURE_COUNT];
    MTL::SamplerState* samplers[usize(ShaderType::Count)][TEXTURE_COUNT];
};

struct EncoderState {
    MTL::RenderPassDescriptor* render_pass{nullptr};
    EncoderRenderState render{};
};

class Renderer : public RendererBase {
  public:
    static Renderer& GetInstance();

    Renderer();
    ~Renderer() override;

    void SetSurface(void* surface) override;

    void Present(const TextureBase* texture, const uint2 output_origin,
                 const uint2 output_size) override;

    // Buffer
    BufferBase* CreateBuffer(const BufferDescriptor& descriptor) override;
    BufferBase* AllocateTemporaryBuffer(const usize size) override;
    void FreeTemporaryBuffer(BufferBase* buffer) override;

    // Texture
    TextureBase* CreateTexture(const TextureDescriptor& descriptor) override;

    // Sampler
    SamplerBase* CreateSampler(const SamplerDescriptor& descriptor) override;

    // Command buffer
    void EndCommandBuffer() override;

    // Render pass
    RenderPassBase*
    CreateRenderPass(const RenderPassDescriptor& descriptor) override;
    void BindRenderPass(const RenderPassBase* render_pass) override;

    // Clear
    void ClearColor(u32 render_target_id, u32 layer, u8 mask,
                    const uint4 color) override;
    void ClearDepth(u32 layer, const float value) override;
    void ClearStencil(u32 layer, const u32 value) override;

    // Shader
    ShaderBase* CreateShader(const ShaderDescriptor& descriptor) override;

    // Pipeline
    PipelineBase* CreatePipeline(const PipelineDescriptor& descriptor) override;
    void BindPipeline(const PipelineBase* pipeline) override;

    // Resource binding
    void BindVertexBuffer(BufferBase* buffer, u32 index) override;
    void BindIndexBuffer(BufferBase* index_buffer,
                         engines::IndexType index_type) override;
    void BindUniformBuffer(BufferBase* buffer, ShaderType shader_type,
                           u32 index) override;
    void BindTexture(TextureBase* texture, SamplerBase* sampler,
                     ShaderType shader_type, u32 index) override;

    // Resource unbinding
    void UnbindTextures(ShaderType shader_type) override;

    // Draw
    void Draw(const engines::PrimitiveType primitive_type, const u32 start,
              const u32 count, const u32 base_vertex, const u32 base_instance,
              const u32 instance_count, bool indexed) override;

    // Helpers

    // Command buffer
    void EnsureCommandBuffer();

    void CommitCommandBuffer() {
        if (command_buffer) {
            EndEncoding();

            command_buffer->commit();
            // HACK: wait until completed so as to avoid sync issues
            command_buffer->waitUntilCompleted();
            command_buffer = nullptr;
        }
    }

    MTL::RenderCommandEncoder* GetRenderCommandEncoderUnchecked() {
        ASSERT_DEBUG(encoder_type == EncoderType::Render, MetalRenderer,
                     "Render command encoder not active");
        return static_cast<MTL::RenderCommandEncoder*>(command_encoder);
    }
    MTL::RenderCommandEncoder* GetRenderCommandEncoder();
    MTL::RenderCommandEncoder* CreateRenderCommandEncoder(
        MTL::RenderPassDescriptor* render_pass_descriptor);

    MTL::BlitCommandEncoder* GetBlitCommandEncoderUnchecked() {
        ASSERT_DEBUG(encoder_type == EncoderType::Blit, MetalRenderer,
                     "Blit command encoder not active");
        return static_cast<MTL::BlitCommandEncoder*>(command_encoder);
    }
    MTL::BlitCommandEncoder* GetBlitCommandEncoder();

    void EndEncoding();

    // Encoder state setting
    void SetRenderPipelineState(MTL::RenderPipelineState* mtl_pipeline);
    void SetRenderPipelineState();
    void SetDepthStencilState(MTL::DepthStencilState* mtl_depth_stencil_state);
    void SetDepthStencilState();
    void SetBuffer(MTL::Buffer* buffer, ShaderType shader_type, u32 index);
    void SetVertexBuffer(u32 index);
    void SetUniformBuffer(ShaderType shader_type, u32 index);
    void SetTexture(MTL::Texture* texture, ShaderType shader_type, u32 index);
    void SetSampler(MTL::SamplerState* sampler, ShaderType shader_type,
                    u32 index);
    void SetTexture(ShaderType shader_type, u32 index);

    // Other
    void BlitTexture(MTL::Texture* src, const float3 src_origin,
                     const usize3 src_size, MTL::Texture* dst,
                     const u32 dst_layer, const float3 dst_origin,
                     const usize3 dst_size);

    // Getters
    MTL::Device* GetDevice() const { return device; }

  private:
    MTL::Device* device;
    MTL::CommandQueue* command_queue;

    CA::MetalLayer* layer;

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

    // Command buffer
    MTL::CommandBuffer* command_buffer{nullptr};
    MTL::CommandEncoder* command_encoder{nullptr};
    EncoderType encoder_type{EncoderType::None};

    // State
    State state;
    u32 padding[0x100]; // HACK: for some reason, writing to some fields of the
                        // encoder_state corrupts the state
    EncoderState encoder_state;

    // Debug
    bool capturing = false;

    // Debug
    void BeginCapture();
    void EndCapture();
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
