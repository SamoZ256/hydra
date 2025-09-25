#pragma once

#include "core/hw/tegra_x1/gpu/renderer/buffer_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/index_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/pipeline_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/render_pass_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/sampler_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_cache.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class BufferBase;
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

    // Mutex
    void LockMutex() { mutex.lock(); }
    void UnlockMutex() { mutex.unlock(); }

    // Surface
    virtual void SetSurface(void* surface) = 0;
    virtual bool AcquireNextSurface() = 0;
    virtual void BeginSurfaceRenderPass() = 0;
    // Uses lower left origin
    virtual void DrawTextureToSurface(const TextureBase* texture,
                                      const FloatRect2D src_rect,
                                      const FloatRect2D dst_rect,
                                      bool transparent, f32 opacity = 1.0f) = 0;
    virtual void EndSurfaceRenderPass() = 0;
    void PresentSurface() {
        PresentSurfaceImpl();

        if (is_capturing) {
            EndCapture();
            is_capturing = false;
        }

        if (begin_capture) {
            BeginCapture();
            is_capturing = true;
            begin_capture = false;
        }
    }

    // Buffer
    virtual BufferBase* CreateBuffer(const BufferDescriptor& descriptor) = 0;
    virtual BufferBase* AllocateTemporaryBuffer(const usize size) = 0;
    virtual void FreeTemporaryBuffer(BufferBase* buffer) = 0;

    // Texture
    virtual TextureBase* CreateTexture(const TextureDescriptor& descriptor) = 0;

    // Sampler
    virtual SamplerBase* CreateSampler(const SamplerDescriptor& descriptor) = 0;

    // Command buffer
    virtual void EndCommandBuffer() = 0;

    // Render pass
    virtual RenderPassBase*
    CreateRenderPass(const RenderPassDescriptor& descriptor) = 0;
    virtual void BindRenderPass(const RenderPassBase* render_pass) = 0;

    // Clear
    virtual void ClearColor(u32 render_target_id, u32 layer, u8 mask,
                            const uint4 color) = 0;
    virtual void ClearDepth(u32 layer, const float value) = 0;
    virtual void ClearStencil(u32 layer, const u32 value) = 0;

    // Shader
    virtual ShaderBase* CreateShader(const ShaderDescriptor& descriptor) = 0;

    // Pipeline
    virtual PipelineBase*
    CreatePipeline(const PipelineDescriptor& descriptor) = 0;
    virtual void BindPipeline(const PipelineBase* pipeline) = 0;

    // Resource binding
    virtual void BindVertexBuffer(BufferBase* buffer, u32 index) = 0;
    virtual void BindIndexBuffer(BufferBase* index_buffer,
                                 engines::IndexType index_type) = 0;
    virtual void BindUniformBuffer(BufferBase* buffer, ShaderType shader_type,
                                   u32 index) = 0;
    // TODO: storage buffers
    virtual void BindTexture(TextureBase* texture, SamplerBase* sampler,
                             ShaderType shader_type, u32 index) = 0;
    // TODO: images

    // Resource unbinding
    virtual void UnbindTextures(ShaderType shader_type) = 0;

    // Draw
    virtual void Draw(const engines::PrimitiveType primitive_type,
                      const u32 start, const u32 count, const u32 base_instance,
                      const u32 instance_count) = 0;
    virtual void DrawIndexed(const engines::PrimitiveType primitive_type,
                             const u32 start, const u32 count,
                             const u32 base_vertex, const u32 base_instance,
                             const u32 instance_count) = 0;

    // Capture
    void CaptureFrame() { begin_capture = true; }

  protected:
    Info info{};

    // Surface
    virtual void PresentSurfaceImpl() = 0;

    // Capture
    virtual void BeginCapture() = 0;
    virtual void EndCapture() = 0;

  private:
    std::mutex mutex;

    // Caches
    BufferCache buffer_cache;
    TextureCache texture_cache;
    SamplerCache sampler_cache;
    RenderPassCache render_pass_cache;
    ShaderCache shader_cache;
    PipelineCache pipeline_cache;
    IndexCache index_cache;

    // Capture
    bool begin_capture{false};
    bool is_capturing{false};

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
