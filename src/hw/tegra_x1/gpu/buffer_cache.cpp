#include "hw/tegra_x1/gpu/buffer_cache.hpp"

#include "hw/tegra_x1/gpu/gpu.hpp"
#include "hw/tegra_x1/gpu/renderer/buffer_base.hpp"

// HACK
template <typename T> T rotl(T v, u64 shift) {
    return (v << shift) | (v >> (32 - shift));
}

namespace Hydra::HW::TegraX1::GPU {

Renderer::BufferBase*
BufferCache::Create(const Renderer::BufferDescriptor& descriptor) {
    auto texture = RENDERER->CreateBuffer(descriptor);
    // TODO: upload buffer

    return texture;
}

void BufferCache::Update(Renderer::BufferBase* buffer) {
    // TODO: if data changed
    if (false)
        ; // TODO: upload buffer
}

u64 BufferCache::Hash(const Renderer::BufferDescriptor& descriptor) {
    u64 hash = 0;
    hash += descriptor.ptr;
    hash = rotl(hash, 19);
    hash += descriptor.size;

    return hash;
}

void BufferCache::DestroyElement(Renderer::BufferBase* buffer) {
    delete buffer;
}

} // namespace Hydra::HW::TegraX1::GPU
