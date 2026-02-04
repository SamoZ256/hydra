#pragma once

#include "core/hw/tegra_x1/gpu/renderer/buffer_view.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class BufferBase;

struct IndexDescriptor {
    engines::IndexType type;
    engines::PrimitiveType primitive_type;
    u32 count;
    std::optional<Range<uptr>> mem_range{std::nullopt};
};

// TODO: memory invalidation
class IndexCache {
  public:
    ~IndexCache();

    BufferView Decode(ICommandBuffer* command_buffer,
                      const IndexDescriptor& descriptor,
                      engines::IndexType& out_type,
                      engines::PrimitiveType& out_primitive_type,
                      u32& out_count);
    u32 Hash(const IndexDescriptor& descriptor);

  private:
    std::map<u32, BufferBase*> cache;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
