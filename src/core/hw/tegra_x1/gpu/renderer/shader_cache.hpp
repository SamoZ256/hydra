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
    ShaderCache(IRenderer& renderer_) : renderer{renderer_} {}

    void Destroy() {}

    ShaderBase* Create(const GuestShaderDescriptor& descriptor);
    void Update([[maybe_unused]] ShaderBase* shader) {}
    u32 Hash(const GuestShaderDescriptor& descriptor);

    void DestroyElement(ShaderBase* shader);

  private:
    IRenderer& renderer;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
