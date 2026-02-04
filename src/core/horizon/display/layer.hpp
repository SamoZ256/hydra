#pragma once

#include "core/horizon/display/binder.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {
class TextureBase;
class ICommandBuffer;
class ISurfaceCompositor;
} // namespace hydra::hw::tegra_x1::gpu::renderer

namespace hydra::horizon::display {

#define LAYER_SIZE_AUTO                                                        \
    uint2 { 0, 0 }

class Layer {
  public:
    Layer(kernel::Process* process_, u32 binder_id_)
        : process{process_}, binder_id{binder_id_} {}

    // TODO
    void Open() {}
    void Close() {}

    bool AcquirePresentTexture(
        hw::tegra_x1::gpu::renderer::ICommandBuffer* command_buffer);
    void Present(hw::tegra_x1::gpu::renderer::ICommandBuffer* command_buffer,
                 hw::tegra_x1::gpu::renderer::ISurfaceCompositor* compositor,
                 FloatRect2D dst_rect, f32 dst_scale, bool transparent);

    // Time
    AccumulatedTime GetAccumulatedDT();

  private:
    kernel::Process* process;
    u32 binder_id;

    float2 position{0, 0};
    uint2 size{LAYER_SIZE_AUTO};
    i64 z{0};

    // Present
    hw::tegra_x1::gpu::renderer::TextureBase* present_texture{nullptr};
    IntRect2D src_rect;

  public:
    GETTER(process, GetProcess);
    GETTER(binder_id, GetBinderID);
    SETTER(position, SetPosition);
    SETTER(size, SetSize);
    GETTER_AND_SETTER(z, GetZ, SetZ);
    GETTER(present_texture, GetPresentTexture);
    CONST_REF_GETTER(src_rect, GetSrcRect);
};

} // namespace hydra::horizon::display
