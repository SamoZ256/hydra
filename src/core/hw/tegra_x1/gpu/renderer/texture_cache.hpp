#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu::renderer {

class ICommandBuffer;
class ITexture;
class ITextureView;
class IRenderer;

using TextureCacheClock = std::chrono::steady_clock;
using TextureCacheTimePoint = TextureCacheClock::time_point;

struct TextureStorage {
    ITexture* base{nullptr};
    SmallCache<u32, ITextureView*> view_cache;
    TextureCacheTimePoint update_timestamp;

    void markUpdated() { update_timestamp = TextureCacheClock::now(); }
};

struct TextureGroup {
    SmallCache<u32, TextureStorage> cache;

    // Debug
    usize getStorageCount() const { return cache.getCount(); }

    const TextureStorage& getStorage(u32 index) const {
        // HACK: const cast
        auto it = const_cast<SmallCache<u32, TextureStorage>&>(cache).begin();
        std::advance(it, index);
        return it->second;
    }
};

struct TextureMemInfo {
    TextureCacheTimePoint modified_timestamp;
    TextureCacheTimePoint read_timestamp;
    TextureCacheTimePoint written_timestamp;

    void markModified() { modified_timestamp = TextureCacheClock::now(); }
    void markRead() { read_timestamp = TextureCacheClock::now(); }
    void markWritten() { written_timestamp = TextureCacheClock::now(); }
};

struct TextureMem {
    ztd::Range<uptr> range;
    TextureMemInfo info;
    SmallCache<u32, TextureGroup> cache;

    // Debug
    usize getTextureGroupCount() const { return cache.getCount(); }

    const TextureGroup& getTextureGroup(u32 index) const {
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
    explicit TextureCache(IRenderer& renderer_) : renderer{renderer_} {}
    ~TextureCache();

    ITextureView* find(ICommandBuffer* command_buffer,
                       const TextureDescriptor& descriptor, TextureUsage usage);
    ITextureView* find(ICommandBuffer* command_buffer,
                       const TextureDescriptor& descriptor,
                       const TextureViewDescriptor& view_descriptor,
                       TextureUsage usage);

    void invalidateMemory(ztd::Range<uptr> range);

    // Debug
    usize getMemoryCount() const { return entries.size(); }

    const TextureMem& getMemory(u32 index) const {
        auto it = entries.begin();
        std::advance(it, index);
        return it->second;
    }

  private:
    IRenderer& renderer;

    std::mutex mutex;
    std::map<uptr, TextureMem> entries;

    static void mergeMemories(TextureMem& mem, TextureMem& other);
    ITextureView* addToMemory(ICommandBuffer* command_buffer, TextureMem& mem,
                              const TextureDescriptor& descriptor,
                              const TextureViewDescriptor& view_descriptor,
                              TextureUsage usage);
    void updateStorage(ICommandBuffer* command_buffer, TextureStorage& storage,
                       TextureMem& mem, const TextureDescriptor& descriptor,
                       TextureUsage usage);
    static ITextureView*
    getTextureView(TextureStorage& storage,
                   const TextureViewDescriptor& view_descriptor);
    ITextureView* getTextureView(ICommandBuffer* command_buffer,
                                 TextureStorage& storage, TextureMem& mem,
                                 const TextureViewDescriptor& view_descriptor,
                                 TextureUsage usage);
    ITextureView* getTexture(ICommandBuffer* command_buffer,
                             TextureStorage& storage, TextureMem& mem,
                             const TextureDescriptor& descriptor,
                             const TextureViewDescriptor& view_descriptor,
                             TextureUsage usage);
    void update(ICommandBuffer* command_buffer, TextureStorage& storage,
                TextureMem& mem, TextureUsage usage);

    // Data synchronization
    static void synchronize2DWith2D(ICommandBuffer* command_buffer,
                                    TextureStorage& storage,
                                    TextureStorage& other_storage);
    static void synchronize3DWith3D(ICommandBuffer* command_buffer,
                                    TextureStorage& storage,
                                    TextureStorage& other_storage);

    // Helpers
    static u32 getDataHash(const ITexture* texture);
    void decodeTexture(ICommandBuffer* command_buffer, TextureStorage& storage);
    // TODO: encode texture

  public:
    REF_GETTER(mutex, getMutex);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
