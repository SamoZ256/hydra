#pragma once

#include "core/hw/tegra_x1/gpu/renderer/texture_decoder.hpp"

namespace hydra::hw::tegra_x1::cpu {
class MMUBase;
}

namespace hydra::hw::tegra_x1::gpu::renderer {

class TextureBase;

struct Tex {
    TextureBase* base;
    small_cache<u32, TextureBase*> view_cache;
};

class TextureCache : public CacheBase<TextureCache, Tex, TextureDescriptor> {
  public:
    void Destroy() {}

    TextureBase* GetTextureView(const TextureDescriptor& descriptor);

    Tex Create(const TextureDescriptor& descriptor);
    void Update(Tex& texture);
    u64 Hash(const TextureDescriptor& descriptor);

    void DestroyElement(Tex& texture);

  private:
    TextureDecoder texture_decoder;

    // Buffers
    std::vector<u8> scratch_buffer;

    // Helpers
    void DecodeTexture(TextureBase* texture);
    // TODO: encode texture
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
