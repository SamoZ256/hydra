#pragma once

#include "core/hw/tegra_x1/gpu/engines/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class ShaderBase;

struct GuestShaderState {
    engines::VertexAttribState vertex_attrib_states[VERTEX_ATTRIB_COUNT];
    TextureFormat color_target_formats[COLOR_TARGET_COUNT] = {
        TextureFormat::Invalid};
};

struct GuestShaderDescriptor {
    engines::ShaderStage stage;
    uptr code_ptr;
    GuestShaderState state;
};

class ShaderCache
    : public CacheBase<ShaderCache, ShaderBase*, GuestShaderDescriptor> {
  public:
    void Destroy() {}

    ShaderBase* Create(const GuestShaderDescriptor& descriptor);
    void Update(ShaderBase* shader) {}
    u64 Hash(const GuestShaderDescriptor& descriptor);

    void DestroyElement(ShaderBase* shader);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
