#pragma once

#include "core/hw/tegra_x1/gpu/renderer/command_buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

enum class EncoderType {
    None,
    Render,
    Compute,
    Blit,
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
               static_cast<usize>(ShaderType::Count)>
        buffers{};
    std::array<std::array<MTL::Texture*, TEXTURE_COUNT>,
               static_cast<usize>(ShaderType::Count)>
        textures{};
    std::array<std::array<MTL::SamplerState*, TEXTURE_COUNT>,
               static_cast<usize>(ShaderType::Count)>
        samplers{};
};

struct EncoderState {
    MTL::RenderPassDescriptor* render_pass{nullptr};
    EncoderRenderState render{};
};

class CommandBuffer final : public ICommandBuffer {
  public:
    explicit CommandBuffer(MTL::CommandQueue* command_queue);
    ~CommandBuffer() override;

    MTL::RenderCommandEncoder* getRenderCommandEncoderUnchecked() {
        ASSERT_DEBUG(encoder_type == EncoderType::Render, MetalRenderer,
                     "Render command encoder not active");
        return static_cast<MTL::RenderCommandEncoder*>(command_encoder);
    }
    MTL::RenderCommandEncoder*
    getRenderCommandEncoder(MTL::RenderPassDescriptor* render_pass_descriptor);
    MTL::RenderCommandEncoder* createRenderCommandEncoder(
        MTL::RenderPassDescriptor* render_pass_descriptor);

    MTL::BlitCommandEncoder* getBlitCommandEncoderUnchecked() {
        ASSERT_DEBUG(encoder_type == EncoderType::Blit, MetalRenderer,
                     "Blit command encoder not active");
        return static_cast<MTL::BlitCommandEncoder*>(command_encoder);
    }
    MTL::BlitCommandEncoder* getBlitCommandEncoder();

    void endEncoding();

    // Encoder state setting
    void setRenderPipelineState(MTL::RenderPipelineState* pipeline);
    void setDepthStencilState(MTL::DepthStencilState* depth_stencil_state);
    void setCullMode(MTL::CullMode cull_mode);
    void setFrontFaceWinding(MTL::Winding front_face_winding);
    void setBuffer(MTL::Buffer* buffer, u64 offset, ShaderType shader_type,
                   u32 index);
    void setTexture(MTL::Texture* texture, ShaderType shader_type, u32 index);
    void setSampler(MTL::SamplerState* sampler, ShaderType shader_type,
                    u32 index);

  private:
    MTL::CommandBuffer* command_buffer{nullptr};
    MTL::CommandEncoder* command_encoder{nullptr};
    EncoderType encoder_type{EncoderType::None};

    EncoderState encoder_state{};

  public:
    GETTER(command_buffer, getCommandBuffer);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
