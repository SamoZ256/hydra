#include "core/hw/tegra_x1/gpu/renderer/buffer_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

BufferBase* BufferCache::Create(const BufferDescriptor& descriptor) {
    auto texture = RENDERER_INSTANCE->CreateBuffer(descriptor);
    // TODO: upload buffer

    return texture;
}

void BufferCache::Update(BufferBase* buffer) {
    // TODO: if data changed
    if (false)
        ; // TODO: upload buffer
}

u64 BufferCache::Hash(const BufferDescriptor& descriptor) {
    u64 hash = 0;
    hash += descriptor.ptr;
    hash = rotl(hash, 19);
    hash += descriptor.size;

    return hash;
}

void BufferCache::DestroyElement(BufferBase* buffer) { delete buffer; }

} // namespace hydra::hw::tegra_x1::gpu::renderer
