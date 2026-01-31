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

struct MtlBufferState {
    MTL::Buffer* buffer{nullptr};
    u64 offset{0};
};

struct EncoderRenderState {
    MTL::RenderPipelineState* pipeline{nullptr};
    MTL::DepthStencilState* depth_stencil_state{nullptr};
    MTL::CullMode cull_mode{MTL::CullModeNone};
    MTL::Winding front_face_winding{MTL::WindingClockwise};
    std::array<std::array<MtlBufferState, BUFFER_COUNT>,
               usize(ShaderType::Count)>
        buffers{};
    std::array<std::array<MTL::Texture*, TEXTURE_COUNT>,
               usize(ShaderType::Count)>
        textures{};
    std::array<std::array<MTL::SamplerState*, TEXTURE_COUNT>,
               usize(ShaderType::Count)>
        samplers{};
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
    void Draw(const engines::PrimitiveType primitive_type, const u32 start,
              const u32 count, const u32 base_instance,
              const u32 instance_count) override;
    void DrawIndexed(const engines::PrimitiveType primitive_type,
                     const u32 start, const u32 count, const u32 base_vertex,
                     const u32 base_instance,
                     const u32 instance_count) override;

    // Helpers

    // Command buffer
    void EnsureCommandBuffer();

    void CommitCommandBuffer() {
        if (command_buffer) {
            EndEncoding();

            command_buffer->commit();
            // HACK: wait until completed so as to avoid sync issues
            command_buffer->waitUntilCompleted();
            command_buffer->release();
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
    void SetRenderPipelineState(MTL::RenderPipelineState* pipeline);
    void SetRenderPipelineState();
    void SetDepthStencilState(MTL::DepthStencilState* depth_stencil_state);
    void SetDepthStencilState();
    void SetCullMode(MTL::CullMode cull_mode);
    void SetFrontFaceWinding(MTL::Winding front_face_winding);
    void SetCullState();
    void SetBuffer(MTL::Buffer* buffer, u64 offset, ShaderType shader_type,
                   u32 index);
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

    // Command buffer
    MTL::CommandBuffer* command_buffer{nullptr};
    MTL::CommandEncoder* command_encoder{nullptr};
    EncoderType encoder_type{EncoderType::None};

    // State
    State state;
    [[maybe_unused]] u32
        padding[0x100]; // HACK: for some reason, writing to some fields of the
                        // encoder_state corrupts the state
    EncoderState encoder_state;

    // Helpers
    bool CanDraw();
    void BindDrawState();

  public:
    GETTER(device, GetDevice);
    GETTER(blit_pipeline_cache, GetBlitPipelineCache);
    GETTER(linear_sampler, GetLinearSampler);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
