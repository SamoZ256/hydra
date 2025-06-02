#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class TextureDecoder {
  public:
    TextureDecoder();
    ~TextureDecoder();

    void Decode(const TextureDescriptor& descriptor, u8*& out_data);

  private:
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
