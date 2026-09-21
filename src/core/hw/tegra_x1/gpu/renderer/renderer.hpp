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
class ITexture;
class ITextureView;
class SamplerBase;
class RenderPassBase;
class PipelineBase;

struct Info {
    bool supports_quads_primitive;

    bool isPrimitiveSupported(engines::PrimitiveType primitive_type) const {
        // NOLINTNEXTLINE(readability-trivial-switch)
        switch (primitive_type) {
        case engines::PrimitiveType::Quads:
            return supports_quads_primitive;
        default:
            return true;
        }
    }
};

enum class MemoryInvalidationScope {
    None = 0,
    BufferCache = ZTD_BIT(0),
    TextureCache = ZTD_BIT(1),
    ShaderCache = ZTD_BIT(2),
};
ZTD_ENABLE_ENUM_BITWISE_OPERATORS(MemoryInvalidationScope)

class IRenderer {
  public:
    IRenderer()
        : buffer_cache(*this), texture_cache(*this), sampler_cache(*this),
          render_pass_cache(*this), shader_cache(*this), pipeline_cache(*this),
          index_cache(*this) {}
    virtual ~IRenderer() = default;

    void invalidateMemory(
        ztd::Range<uptr> range,
        MemoryInvalidationScope scope = MemoryInvalidationScope::BufferCache |
                                        MemoryInvalidationScope::TextureCache |
                                        MemoryInvalidationScope::ShaderCache) {
        if (any(scope & MemoryInvalidationScope::BufferCache)) {
            std::scoped_lock lock(buffer_cache.getMutex());
            buffer_cache.invalidateMemory(range);
        }

        if (any(scope & MemoryInvalidationScope::TextureCache)) {
            std::scoped_lock lock(texture_cache.getMutex());
            texture_cache.invalidateMemory(range);
        }

        if (any(scope & MemoryInvalidationScope::ShaderCache)) {
            // TODO
        }
    }

    // Surface
    virtual void setSurface(void* surface) = 0;
    virtual ISurfaceCompositor* acquireNextSurface() = 0;

    // Buffer
    virtual BufferBase* createBuffer(u64 size) = 0;
    virtual BufferBase* allocateTemporaryBuffer(const u64 size) = 0;
    virtual void freeTemporaryBuffer(BufferBase* buffer) = 0;

    // Texture
    virtual ITexture* createTexture(const TextureDescriptor& descriptor) = 0;
    virtual void blitTexture(ICommandBuffer* command_buffer, ITextureView* src,
                             float3 src_origin, uint3 src_size, u32 src_level,
                             u32 src_layer, ITextureView* dst,
                             float3 dst_origin, uint3 dst_size, u32 dst_level,
                             u32 dst_layer, u32 level_count,
                             u32 layer_count) = 0;

    // Sampler
    virtual SamplerBase* createSampler(const SamplerDescriptor& descriptor) = 0;

    // Command buffer
    virtual ICommandBuffer* createCommandBuffer() = 0;

    // Render pass
    virtual RenderPassBase*
    createRenderPass(const RenderPassDescriptor& descriptor) = 0;
    virtual void bindRenderPass(const RenderPassBase* render_pass) = 0;

    // Clear
    virtual void clearColor(ICommandBuffer* command_buffer,
                            u32 render_target_id, u32 layer, u8 mask,
                            const uint4 color) = 0;
    virtual void clearDepth(ICommandBuffer* command_buffer, u32 layer,
                            const float value) = 0;
    virtual void clearStencil(ICommandBuffer* command_buffer, u32 layer,
                              const u32 value) = 0;

    // Shader
    virtual ShaderBase* createShader(const ShaderDescriptor& descriptor) = 0;

    // Pipeline
    virtual PipelineBase*
    createPipeline(const PipelineDescriptor& descriptor) = 0;
    virtual void bindPipeline(const PipelineBase* pipeline) = 0;

    // Depth stencil
    virtual void setDepthTestEnabled(bool enabled) = 0;
    virtual void setDepthWriteEnabled(bool enabled) = 0;
    virtual void setDepthCompareOp(engines::CompareOp op) = 0;

    // Viewport and scissor
    virtual void setViewport(u32 index, const Viewport& viewport) = 0;
    virtual void setScissor(u32 index, const Scissor& scissor) = 0;

    // Resource binding
    virtual void bindVertexBuffer(const BufferView& buffer, u32 index) = 0;
    virtual void bindIndexBuffer(const BufferView& index_buffer,
                                 engines::IndexType index_type) = 0;
    virtual void bindUniformBuffer(const BufferView& buffer,
                                   ShaderType shader_type, u32 index) = 0;
    // TODO: storage buffers
    virtual void bindTexture(ITextureView* texture, SamplerBase* sampler,
                             ShaderType shader_type, u32 index) = 0;
    // TODO: images

    // Resource unbinding
    virtual void unbindUniformBuffers(ShaderType shader_type) = 0;
    virtual void unbindTextures(ShaderType shader_type) = 0;

    // Draw
    virtual void draw(ICommandBuffer* command_buffer,
                      const engines::PrimitiveType primitive_type,
                      const u32 start, const u32 count, const u32 base_instance,
                      const u32 instance_count) = 0;
    virtual void drawIndexed(ICommandBuffer* command_buffer,
                             const engines::PrimitiveType primitive_type,
                             const u32 start, const u32 count,
                             const u32 base_vertex, const u32 base_instance,
                             const u32 instance_count) = 0;

    // Debug
    void captureFrames(u32 count) { frames_to_capture = count; }
    void notifyDebugFrameBoundary() {
        if (frames_to_capture > 0) {
            if (capturing) {
                if (--frames_to_capture == 0) {
                    endCapture();
                    capturing = false;
                }
            } else {
                beginCapture();
                capturing = true;
            }
        }
    }

  protected:
    Info info{};

    // Capture
    virtual void beginCapture() = 0;
    virtual void endCapture() = 0;

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
    CONST_REF_GETTER(info, getInfo);
    REF_GETTER(buffer_cache, getBufferCache);
    REF_GETTER(texture_cache, getTextureCache);
    REF_GETTER(sampler_cache, getSamplerCache);
    REF_GETTER(render_pass_cache, getRenderPassCache);
    REF_GETTER(shader_cache, getShaderCache);
    REF_GETTER(pipeline_cache, getPipelineCache);
    REF_GETTER(index_cache, getIndexCache);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
