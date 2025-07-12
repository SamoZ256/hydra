#pragma once

#include "core/hw/tegra_x1/gpu/renderer/texture_decoder.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu::renderer {

class TextureBase;

struct ModifyInfo {
    u64 timestamp;
};

struct Tex {
    TextureBase* base{nullptr};
    small_cache<u32, TextureBase*> view_cache;
    u64 upload_timestamp{0};
};

struct TextureMem {
    small_cache<u64, Tex> cache;
    ModifyInfo last_modified{0};

    void MarkModified() { last_modified = {get_absolute_time()}; }
};

// TODO: track Gpu modifications as well?
class TextureCache {
  public:
    ~TextureCache();

    TextureBase* GetTextureView(const TextureDescriptor& descriptor);

    void NotifyGuestModifiedData(const range<uptr> mem_range);

  private:
    TextureDecoder texture_decoder;

    // TODO: use a more memory lookup friendly data structure
    std::map<uptr, TextureMem> texture_mem_map;

    // Buffers
    std::vector<u8> scratch_buffer;

    TextureBase* Create(const TextureDescriptor& descriptor);
    void Update(Tex& tex, const ModifyInfo& mem_last_modified);

    // Helpers
    u64 GetTextureHash(const TextureDescriptor& descriptor);
    void DecodeTexture(TextureBase* texture);
    // TODO: encode texture
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
