#pragma once

#include "hw/tegra_x1/gpu/renderer/texture_decoder.hpp"

namespace Hydra::HW::TegraX1::CPU {
class MMUBase;
}

namespace Hydra::HW::TegraX1::GPU::Renderer {

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
    u8 scratch_buffer[0x4000 * 0x4000 * 0x4]; // TODO: allocate dynamically

    // Helpers
    void DecodeTexture(TextureBase* texture);
    // TODO: encode texture
};

} // namespace Hydra::HW::TegraX1::GPU::Renderer
