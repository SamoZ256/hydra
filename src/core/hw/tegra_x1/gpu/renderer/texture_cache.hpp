#pragma once

#include "core/hw/tegra_x1/gpu/renderer/texture_decoder.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu::renderer {

class ICommandBuffer;
class TextureBase;

typedef std::chrono::steady_clock TextureCacheClock;
typedef TextureCacheClock::time_point TextureCacheTimePoint;

struct TextureGroup {
    TextureBase* base{nullptr};
    SmallCache<u32, TextureBase*> view_cache;
    TextureCacheTimePoint update_timestamp{};

    void MarkUpdated() { update_timestamp = TextureCacheClock::now(); }
};

struct SparseTexture {
    SmallCache<uptr, TextureGroup> cache;
};

struct TextureMemInfo {
    TextureCacheTimePoint modified_timestamp{};
    TextureCacheTimePoint read_timestamp{};
    TextureCacheTimePoint written_timestamp{};

    void MarkModified() { modified_timestamp = TextureCacheClock::now(); }
    void MarkRead() { read_timestamp = TextureCacheClock::now(); }
    void MarkWritten() { written_timestamp = TextureCacheClock::now(); }
};

struct TextureMem {
    Range<uptr> range;
    TextureMemInfo info;
    SmallCache<u32, SparseTexture> cache;
};

// TODO: destroy textures
// TODO: texture readback
class TextureCache {
  public:
    ~TextureCache();

    TextureBase* Find(ICommandBuffer* command_buffer,
                      const TextureDescriptor& descriptor, TextureUsage usage);

    void InvalidateMemory(Range<uptr> range);

  private:
    std::mutex mutex;
    TextureDecoder texture_decoder;

    std::map<uptr, TextureMem> entries;

    TextureMem MergeMemories(const TextureMem& a, const TextureMem& b);
    TextureBase* AddToMemory(ICommandBuffer* command_buffer, TextureMem& mem,
                             const TextureDescriptor& descriptor,
                             TextureUsage usage);
    TextureBase* GetTexture(ICommandBuffer* command_buffer, TextureGroup& group,
                            TextureMem& mem,
                            const TextureDescriptor& descriptor,
                            TextureUsage usage);
    TextureBase* GetTextureView(TextureGroup& group,
                                const TextureViewDescriptor& descriptor);
    void Create(ICommandBuffer* command_buffer,
                const TextureDescriptor& descriptor, TextureGroup& group);
    void Update(ICommandBuffer* command_buffer, TextureGroup& group,
                TextureMem& mem, TextureUsage usage);

    // Helpers
    u32 GetDataHash(const TextureBase* texture);
    void DecodeTexture(ICommandBuffer* command_buffer, TextureGroup& group);
    // TODO: encode texture

  public:
    REF_GETTER(mutex, GetMutex);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
