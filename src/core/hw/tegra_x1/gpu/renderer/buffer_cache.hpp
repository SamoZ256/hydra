#pragma once

#include "core/hw/tegra_x1/gpu/renderer/buffer_view.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu::renderer {

// TODO: also release the buffer
struct BufferEntry {
    BufferBase* buffer{nullptr};
    Range<uptr> range;
    std::optional<Range<uptr>> invalidation_range{};
    bool inline_copy{false}; // TODO: implement
};

// TODO: optional data hashing
class BufferCache {
  public:
    ~BufferCache();

    BufferView Get(Range<uptr> range);

    void InvalidateMemory(Range<uptr> range);

  private:
    std::map<uptr, BufferEntry> entries;

    // Helpers
    static void UpdateRange(BufferEntry& entry, Range<uptr> range);
    BufferEntry& Find(Range<uptr> range);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
