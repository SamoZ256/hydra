#pragma once

#include "core/hw/tegra_x1/gpu/engines/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class ShaderBase;
class IRenderer;

struct GuestShaderState {
    engines::VertexAttribState vertex_attrib_states[VERTEX_ATTRIB_COUNT];
    ColorDataType color_target_data_types[COLOR_TARGET_COUNT] = {
        ColorDataType::Invalid};
};

struct GuestShaderDescriptor {
    engines::ShaderStage stage;
    uptr code_ptr;
    GuestShaderState state;
};

class ShaderCache
    : public CacheBase<ShaderCache, ShaderBase*, GuestShaderDescriptor> {
  public:
    explicit ShaderCache(IRenderer& renderer_) : renderer{renderer_} {}

    void destroy() {}

    ShaderBase* create(const GuestShaderDescriptor& descriptor);
    void update([[maybe_unused]] ShaderBase* shader) {}
    static u32 hash(const GuestShaderDescriptor& descriptor);

    static void destroyElement(ShaderBase* shader);

  private:
    IRenderer& renderer;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
