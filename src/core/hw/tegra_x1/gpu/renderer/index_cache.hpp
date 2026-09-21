#pragma once

#include "core/hw/tegra_x1/gpu/renderer/buffer_view.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class BufferBase;
class IRenderer;

struct IndexDescriptor {
    engines::IndexType type;
    engines::PrimitiveType primitive_type;
    u32 count;
    std::optional<ztd::Range<uptr>> mem_range{std::nullopt};
};

// TODO: memory invalidation
class IndexCache {
  public:
    explicit IndexCache(IRenderer& renderer_) : renderer{renderer_} {}
    ~IndexCache();

    BufferView decode(ICommandBuffer* command_buffer,
                      const IndexDescriptor& descriptor,
                      engines::IndexType& out_type,
                      engines::PrimitiveType& out_primitive_type,
                      u32& out_count);
    static u32 hash(const IndexDescriptor& descriptor);

  private:
    IRenderer& renderer;

    std::map<u32, BufferBase*> cache;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
