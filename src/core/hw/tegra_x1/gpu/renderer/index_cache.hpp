#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class BufferBase;

struct IndexDescriptor {
    engines::IndexType type;
    engines::PrimitiveType primitive_type;
    u32 count;
    BufferBase* src_index_buffer;
};

class IndexCache {
  public:
    ~IndexCache();

    BufferBase* Decode(const IndexDescriptor& descriptor,
                       engines::IndexType& out_type,
                       engines::PrimitiveType& out_primitive_type,
                       u32& out_count);
    u64 Hash(const IndexDescriptor& descriptor);

  private:
    std::map<u64, BufferBase*> cache;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
