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

    void MarkUpdated() { update_timestamp = TextureCacheClock::now(); }
};

struct TextureGroup {
    SmallCache<u32, TextureStorage> cache;

    // Debug
    usize GetStorageCount() const { return cache.GetCount(); }

    const TextureStorage& GetStorage(u32 index) const {
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

    void MarkModified() { modified_timestamp = TextureCacheClock::now(); }
    void MarkRead() { read_timestamp = TextureCacheClock::now(); }
    void MarkWritten() { written_timestamp = TextureCacheClock::now(); }
};

struct TextureMem {
    ztd::Range<uptr> range;
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
    TextureCache(IRenderer& renderer_) : renderer{renderer_} {}
    ~TextureCache();

    ITextureView* Find(ICommandBuffer* command_buffer,
                       const TextureDescriptor& descriptor, TextureUsage usage);
    ITextureView* Find(ICommandBuffer* command_buffer,
                       const TextureDescriptor& descriptor,
                       const TextureViewDescriptor& view_descriptor,
                       TextureUsage usage);

    void InvalidateMemory(ztd::Range<uptr> range);

    // Debug
    usize GetMemoryCount() const { return entries.size(); }

    const TextureMem& GetMemory(u32 index) const {
        auto it = entries.begin();
        std::advance(it, index);
        return it->second;
    }

  private:
    IRenderer& renderer;

    std::mutex mutex;
    std::map<uptr, TextureMem> entries;

    static void MergeMemories(TextureMem& mem, TextureMem& other);
    ITextureView* AddToMemory(ICommandBuffer* command_buffer, TextureMem& mem,
                              const TextureDescriptor& descriptor,
                              const TextureViewDescriptor& view_descriptor,
                              TextureUsage usage);
    void UpdateStorage(ICommandBuffer* command_buffer, TextureStorage& storage,
                       TextureMem& mem, const TextureDescriptor& descriptor,
                       TextureUsage usage);
    static ITextureView* GetTextureView(TextureStorage& storage,
                                 const TextureViewDescriptor& view_descriptor);
    ITextureView* GetTextureView(ICommandBuffer* command_buffer,
                                 TextureStorage& storage, TextureMem& mem,
                                 const TextureViewDescriptor& view_descriptor,
                                 TextureUsage usage);
    ITextureView* GetTexture(ICommandBuffer* command_buffer,
                             TextureStorage& storage, TextureMem& mem,
                             const TextureDescriptor& descriptor,
                             const TextureViewDescriptor& view_descriptor,
                             TextureUsage usage);
    void Update(ICommandBuffer* command_buffer, TextureStorage& storage,
                TextureMem& mem, TextureUsage usage);

    // Data synchronization
    static void Synchronize2DWith2D(ICommandBuffer* command_buffer,
                             TextureStorage& storage,
                             TextureStorage& other_storage);
    static void Synchronize3DWith3D(ICommandBuffer* command_buffer,
                             TextureStorage& storage,
                             TextureStorage& other_storage);

    // Helpers
    static u32 GetDataHash(const ITexture* texture);
    void DecodeTexture(ICommandBuffer* command_buffer, TextureStorage& storage);
    // TODO: encode texture

  public:
    REF_GETTER(mutex, GetMutex);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
