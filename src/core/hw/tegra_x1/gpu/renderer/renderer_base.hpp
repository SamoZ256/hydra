#pragma once

#include "core/hw/tegra_x1/gpu/renderer/buffer_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/buffer_view.hpp"
#include "core/hw/tegra_x1/gpu/renderer/index_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/pipeline_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/render_pass_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/sampler_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_cache.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class ICommandBuffer;
class ISurfaceCompositor;
class TextureBase;
class SamplerBase;
class RenderPassBase;
class PipelineBase;

struct Info {
    bool supports_quads_primitive;

    bool IsPrimitiveSupported(engines::PrimitiveType primitive_type) const {
        switch (primitive_type) {
        case engines::PrimitiveType::Quads:
            return supports_quads_primitive;
        default:
            return true;
        }
    }
};

class RendererBase {
  public:
    virtual ~RendererBase() {}

    // TODO: make this thread safe
    void InvalidateMemory(Range<uptr> range) {
        buffer_cache.InvalidateMemory(range);
        texture_cache.InvalidateMemory(range);
        // TODO: shader cache
    }

    // Surface
    virtual void SetSurface(void* surface) = 0;
    virtual ISurfaceCompositor* AcquireNextSurface() = 0;

    // Buffer
    virtual BufferBase* CreateBuffer(u64 size) = 0;
    virtual BufferBase* AllocateTemporaryBuffer(const u64 size) = 0;
    virtual void FreeTemporaryBuffer(BufferBase* buffer) = 0;

    // Texture
    virtual TextureBase* CreateTexture(const TextureDescriptor& descriptor) = 0;

    // Sampler
    virtual SamplerBase* CreateSampler(const SamplerDescriptor& descriptor) = 0;

    // Command buffer
    virtual ICommandBuffer* CreateCommandBuffer() = 0;

    // Render pass
    virtual RenderPassBase*
    CreateRenderPass(const RenderPassDescriptor& descriptor) = 0;
    virtual void BindRenderPass(const RenderPassBase* render_pass) = 0;

    // Clear
    virtual void ClearColor(ICommandBuffer* command_buffer,
                            u32 render_target_id, u32 layer, u8 mask,
                            const uint4 color) = 0;
    virtual void ClearDepth(ICommandBuffer* command_buffer, u32 layer,
                            const float value) = 0;
    virtual void ClearStencil(ICommandBuffer* command_buffer, u32 layer,
                              const u32 value) = 0;

    // Viewport and scissor
    virtual void SetViewport(u32 index, const Viewport& viewport) = 0;
    virtual void SetScissor(u32 index, const Scissor& scissor) = 0;

    // Shader
    virtual ShaderBase* CreateShader(const ShaderDescriptor& descriptor) = 0;

    // Pipeline
    virtual PipelineBase*
    CreatePipeline(const PipelineDescriptor& descriptor) = 0;
    virtual void BindPipeline(const PipelineBase* pipeline) = 0;

    // Resource binding
    virtual void BindVertexBuffer(const BufferView& buffer, u32 index) = 0;
    virtual void BindIndexBuffer(const BufferView& index_buffer,
                                 engines::IndexType index_type) = 0;
    virtual void BindUniformBuffer(const BufferView& buffer,
                                   ShaderType shader_type, u32 index) = 0;
    // TODO: storage buffers
    virtual void BindTexture(TextureBase* texture, SamplerBase* sampler,
                             ShaderType shader_type, u32 index) = 0;
    // TODO: images

    // Resource unbinding
    virtual void UnbindUniformBuffers(ShaderType shader_type) = 0;
    virtual void UnbindTextures(ShaderType shader_type) = 0;

    // Draw
    virtual void Draw(ICommandBuffer* command_buffer,
                      const engines::PrimitiveType primitive_type,
                      const u32 start, const u32 count, const u32 base_instance,
                      const u32 instance_count) = 0;
    virtual void DrawIndexed(ICommandBuffer* command_buffer,
                             const engines::PrimitiveType primitive_type,
                             const u32 start, const u32 count,
                             const u32 base_vertex, const u32 base_instance,
                             const u32 instance_count) = 0;

    // Debug
    void CaptureFrames(u32 count) { frames_to_capture = count; }
    void NotifyDebugFrameBoundary() {
        if (frames_to_capture > 0) {
            if (capturing) {
                if (--frames_to_capture == 0) {
                    EndCapture();
                    capturing = false;
                }
            } else {
                BeginCapture();
                capturing = true;
            }
        }
    }

  protected:
    Info info{};

    // Capture
    virtual void BeginCapture() = 0;
    virtual void EndCapture() = 0;

  private:
    // Caches
    BufferCache buffer_cache;
    TextureCache texture_cache;
    SamplerCache sampler_cache;
    RenderPassCache render_pass_cache;
    ShaderCache shader_cache;
    PipelineCache pipeline_cache;
    IndexCache index_cache;

    // Capture
    u32 frames_to_capture{0};
    bool capturing{false};

  public:
    CONST_REF_GETTER(info, GetInfo);
    REF_GETTER(buffer_cache, GetBufferCache);
    REF_GETTER(texture_cache, GetTextureCache);
    REF_GETTER(sampler_cache, GetSamplerCache);
    REF_GETTER(render_pass_cache, GetRenderPassCache);
    REF_GETTER(shader_cache, GetShaderCache);
    REF_GETTER(pipeline_cache, GetPipelineCache);
    REF_GETTER(index_cache, GetIndexCache);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
