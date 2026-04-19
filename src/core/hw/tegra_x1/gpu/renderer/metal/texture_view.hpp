#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_view.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

class Texture;

class TextureView final : public ITextureView {
  public:
    TextureView(Texture* base, const TextureViewDescriptor& descriptor);
    ~TextureView() override;

  private:
    MTL::Texture* texture;

  public:
    GETTER(texture, GetTexture);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
