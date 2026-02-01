#include "core/hw/tegra_x1/gpu/renderer/buffer_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

BufferCache::~BufferCache() {
    for (auto& entry : entries)
        delete entry.second.buffer;
}

BufferView BufferCache::Get(Range<uptr> range) {
    auto& entry = Find(range);
    if (entry.buffer) {
        // Check for memory invalidation
        if (entry.invalidation_range.has_value() &&
            entry.invalidation_range->Intersects(range)) {
            const auto invalidation_range = entry.invalidation_range.value();
            UpdateRange(entry, invalidation_range);
            entry.invalidation_range = std::nullopt;
        } else if (CONFIG_INSTANCE.GetCpuBackend() == CpuBackend::Dynarmic) {
            // HACK: force update all buffers on dynarmic
            UpdateRange(entry, entry.range);
        }
    } else {
        // Create new buffer
        entry.buffer = RENDERER_INSTANCE.CreateBuffer(entry.range.GetSize());
        UpdateRange(entry, entry.range);
    }

    return BufferView(entry.buffer, range.GetBegin() - entry.range.GetBegin(),
                      range.GetSize());
}

void BufferCache::InvalidateMemory(Range<uptr> range) {
    auto it = entries.upper_bound(range.GetBegin());
    if (it != entries.begin())
        it--;

    while (it != entries.end() &&
           it->second.range.GetBegin() < range.GetEnd()) {
        auto& entry = it->second;
        if (entry.range.GetEnd() > range.GetBegin()) {
            const auto invalidation_range = range.ClampedTo(entry.range);
            if (entry.invalidation_range.has_value()) {
                // Combine with an existing invalidation range if it exists
                entry.invalidation_range =
                    entry.invalidation_range.value().Union(invalidation_range);
            } else {
                // Clamp the range
                entry.invalidation_range = invalidation_range;
            }
        }
        it++;
    }
}

void BufferCache::UpdateRange(BufferEntry& entry, Range<uptr> range) {
    if (entry.inline_copy) {
        // Do an inline update if possible
        entry.buffer->CopyFrom(range.GetBegin(),
                               range.GetBegin() - entry.range.GetBegin(),
                               range.GetSize());
        entry.inline_copy = false;
    } else {
        // Copy from a temporary buffer
        auto tmp_buffer =
            RENDERER_INSTANCE.AllocateTemporaryBuffer(range.GetSize());
        tmp_buffer->CopyFrom(range.GetBegin());
        entry.buffer->CopyFrom(tmp_buffer,
                               range.GetBegin() - entry.range.GetBegin(), 0,
                               range.GetSize());
        RENDERER_INSTANCE.FreeTemporaryBuffer(tmp_buffer);
    }
}

BufferEntry& BufferCache::Find(Range<uptr> range) {
    // Check for containing interval
    auto it = entries.upper_bound(range.GetBegin());
    if (it != entries.begin()) {
        auto prev = std::prev(it);
        if (prev->second.range.GetEnd() >= range.GetEnd()) {
            // Fully contained
            return prev->second;
        }
    }

    // Insert and merge
    auto new_range = range;

    it = entries.lower_bound(range.GetBegin());

    // Merge with previous if overlapping/touching
    if (it != entries.begin()) {
        auto prev = std::prev(it);
        if (prev->second.range.GetEnd() >= new_range.GetBegin()) {
            new_range = Range<uptr>(
                prev->second.range.GetBegin(),
                std::max(new_range.GetEnd(), prev->second.range.GetEnd()));
            it = entries.erase(prev);
        }
    }

    // Merge with following entries
    while (it != entries.end() && it->first <= new_range.GetEnd()) {
        new_range = Range<uptr>(
            new_range.GetBegin(),
            std::max(new_range.GetEnd(), it->second.range.GetEnd()));
        it = entries.erase(it);
    }

    // Insert merged interval
    auto inserted =
        entries.emplace(new_range.GetBegin(),
                        BufferEntry{.buffer = nullptr, .range = new_range});

    return inserted.first->second;
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
