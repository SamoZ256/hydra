#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu::renderer {

class ICommandBuffer;
class ITexture;
class ITextureView;

typedef std::chrono::steady_clock TextureCacheClock;
typedef TextureCacheClock::time_point TextureCacheTimePoint;

struct TextureStorage {
    ITexture* base{nullptr};
    SmallCache<u32, ITextureView*> view_cache;
    TextureCacheTimePoint update_timestamp{};

    void MarkUpdated() { update_timestamp = TextureCacheClock::now(); }
};

struct TextureGroup {
    SmallCache<uptr, TextureStorage> cache;

    // Debug
    usize GetStorageCount() const { return cache.GetCount(); }

    const TextureStorage& GetStorage(u32 index) const {
        // HACK: const cast
        auto it = const_cast<SmallCache<uptr, TextureStorage>&>(cache).begin();
        std::advance(it, index);
        return it->second;
    }
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
    SmallCache<u32, TextureGroup> cache;

    // Debug
    usize GetTextureGroupCount() const { return cache.GetCount(); }

    const TextureGroup& GetTextureGroup(u32 index) const {
        // HACK: const cast
        auto it = const_cast<SmallCache<u32, TextureGroup>&>(cache).begin();
        std::advance(it, index);
        return it->second;
    }
};

// TODO: destroy textures
// TODO: texture readback
class TextureCache {
  public:
    ~TextureCache();

    ITextureView* Find(ICommandBuffer* command_buffer,
                       const TextureDescriptor& descriptor, TextureUsage usage);
    ITextureView* Find(ICommandBuffer* command_buffer,
                       const TextureDescriptor& descriptor,
                       const TextureViewDescriptor& view_descriptor,
                       TextureUsage usage);

    void InvalidateMemory(Range<uptr> range);

    // Debug
    usize GetMemoryCount() const { return entries.size(); }

    const TextureMem& GetMemory(u32 index) const {
        auto it = entries.begin();
        std::advance(it, index);
        return it->second;
    }

  private:
    std::mutex mutex;

    std::map<uptr, TextureMem> entries;

    void MergeMemories(TextureMem& mem, TextureMem& other);
    ITextureView* AddToMemory(ICommandBuffer* command_buffer, TextureMem& mem,
                              const TextureDescriptor& descriptor,
                              const TextureViewDescriptor& view_descriptor,
                              TextureUsage usage);
    ITextureView* GetTexture(ICommandBuffer* command_buffer,
                             TextureStorage& storage, TextureMem& mem,
                             const TextureDescriptor& descriptor,
                             const TextureViewDescriptor& view_descriptor,
                             TextureUsage usage);
    ITextureView* GetTextureView(ICommandBuffer* command_buffer,
                                 TextureStorage& storage, TextureMem& mem,
                                 const TextureViewDescriptor& view_descriptor,
                                 TextureUsage usage);
    void Update(ICommandBuffer* command_buffer, TextureStorage& storage,
                TextureMem& mem, TextureUsage usage);

    // Helpers
    u32 GetDataHash(const ITexture* texture);
    void DecodeTexture(ICommandBuffer* command_buffer, TextureStorage& storage);
    // TODO: encode texture

  public:
    REF_GETTER(mutex, GetMutex);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
