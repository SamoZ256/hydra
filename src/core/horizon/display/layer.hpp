#pragma once

#include "core/horizon/display/binder.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {
class ITextureView;
class ICommandBuffer;
class ISurfaceCompositor;
} // namespace hydra::hw::tegra_x1::gpu::renderer

namespace hydra::horizon::display {

class Driver;

#define LAYER_SIZE_AUTO uint2{0, 0}

class Layer {
  public:
    Layer(System& system_, kernel::Process* process_, Handle binder_handle_)
        : system{system_}, process{process_}, binder_handle{binder_handle_} {}

    // TODO
    void open() {}
    void close() {}

    bool acquirePresentTexture(
        hw::tegra_x1::gpu::renderer::ICommandBuffer* command_buffer);
    void present(hw::tegra_x1::gpu::renderer::ICommandBuffer* command_buffer,
                 hw::tegra_x1::gpu::renderer::ISurfaceCompositor* compositor,
                 FloatRect2D dst_rect, f32 dst_scale, bool transparent);

    // Time
    AccumulatedTime getAccumulatedDt();

  private:
    System& system;
    kernel::Process* process;
    Handle binder_handle;

    float2 position{0, 0};
    uint2 size{LAYER_SIZE_AUTO};
    i64 z{0};

    // Present
    std::optional<hw::tegra_x1::gpu::renderer::ITextureView*> present_texture;
    IntRect2D src_rect;

  public:
    GETTER(process, getProcess);
    GETTER(binder_handle, getBinderHandle);
    SETTER(position, setPosition);
    SETTER(size, setSize);
    GETTER_AND_SETTER(z, getZ, setZ);
    GETTER(present_texture, getPresentTexture);
    CONST_REF_GETTER(src_rect, getSrcRect);
};

} // namespace hydra::horizon::display
