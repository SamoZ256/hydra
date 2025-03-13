#pragma once

#include "Metal/MTLRenderPass.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "hw/tegra_x1/gpu/renderer/renderer_base.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer::Metal {

class RenderPass;

enum class EncoderType {
    None,
    Render,
    Compute,
    Blit,
};

class Renderer : public RendererBase {
  public:
    static Renderer& GetInstance();

    Renderer();
    ~Renderer() override;

    void SetSurface(void* surface) override;

    void Present(TextureBase* texture) override;

    // Texture
    TextureBase* CreateTexture(const TextureDescriptor& descriptor) override;
    void UploadTexture(TextureBase* texture, void* data) override;

    // Render pass
    RenderPassBase*
    CreateRenderPass(const RenderPassDescriptor& descriptor) override;
    void BindRenderPass(const RenderPassBase* render_pass) override;

    // Getters
    MTL::Device* GetDevice() const { return device; }

  private:
    MTL::Device* device;
    MTL::CommandQueue* command_queue;

    CA::MetalLayer* layer;

    // Pipelines
    MTL::RenderPipelineState* present_pipeline_state;

    // Samplers
    MTL::SamplerState* nearest_sampler;
    MTL::SamplerState* linear_sampler;

    // Command buffer
    MTL::CommandBuffer* command_buffer{nullptr};
    MTL::CommandEncoder* command_encoder{nullptr};
    EncoderType encoder_type{EncoderType::None};

    // State
    struct {
        const RenderPass* render_pass = nullptr;
    } state;

    struct {
        const RenderPass* render_pass = nullptr;
    } encoder_state;

    // Helpers

    // Command buffer
    MTL::CommandBuffer* GetCommandBuffer();
    void CommitCommandBuffer();

    // Command encoders
    MTL::RenderCommandEncoder* GetRenderCommandEncoder();

    MTL::RenderCommandEncoder* CreateRenderCommandEncoder(
        MTL::RenderPassDescriptor* render_pass_descriptor);
    void EndEncoding();
};

} // namespace Hydra::HW::TegraX1::GPU::Renderer::Metal
