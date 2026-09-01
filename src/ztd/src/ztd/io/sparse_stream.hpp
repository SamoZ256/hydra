#pragma once

// TODO: remove dependency on std::vector
#include <vector>

#include "ztd/io/stream.hpp"
#include "ztd/range.hpp"

namespace ztd::io {

class SparseStream : public IStream {
  public:
    struct Entry {
        Range<u64> range;
        IStream* stream;
    };

    // Entries must be sorted by offset
    SparseStream(std::vector<Entry>& entries_, u64 size_)
        : entries{entries_}, size{size_} {}

    [[nodiscard]] auto getSeek() const noexcept -> u64 override { return seek; }
    auto seekTo(u64 seek_) noexcept -> void override { seek = seek_; }
    auto seekBy(u64 offset) noexcept -> void override { seek += offset; }

    [[nodiscard]] auto getSize() const noexcept -> u64 override { return size; }

    auto flush() noexcept -> void override {
        // TODO: only flush modified entries
        for (auto& entry : entries)
            entry.stream->flush();
    }

    auto readRaw(std::span<u8> buffer) noexcept -> void override {
        while (!buffer.empty()) {
            // TODO: check for bounds

            const auto entry = getEntry(seek);
            const auto max_read_size = std::min(
                entry.range.getEnd() - seek, static_cast<u64>(buffer.size()));
            if (entry.stream != nullptr) {
                entry.stream->seekTo(seek - entry.range.getBegin());
                entry.stream->readRaw(buffer.subspan(0, max_read_size));
            } else {
                std::fill(buffer.begin(),
                          buffer.begin() + static_cast<i32>(max_read_size), 0);
            }

            seek += max_read_size;
            buffer = buffer.subspan(max_read_size);
        }
    }

    auto writeRaw(std::span<const u8> buffer) noexcept -> void override {
        while (!buffer.empty()) {
            // TODO: check for bounds

            const auto entry = getEntry(seek);
            const auto max_write_size = std::min(
                entry.range.getEnd() - seek, static_cast<u64>(buffer.size()));
            if (entry.stream != nullptr) {
                entry.stream->seekTo(seek - entry.range.getBegin());
                entry.stream->writeRaw(buffer.subspan(0, max_write_size));
            }

            seek += max_write_size;
            buffer = buffer.subspan(max_write_size);
        }
    }

  private:
    std::vector<Entry>& entries;
    u64 size;

    u64 seek{0};
    std::optional<Entry> cached_entry{std::nullopt};

    // Helpers
    [[nodiscard]] auto getEntry(u64 offset) noexcept -> Entry {
        // First, check if the entry has been cached
        if (cached_entry.has_value()) {
            const auto entry = cached_entry.value();
            if (entry.range.contains(offset))
                return entry;
        }

        // Find the entry that contains the offset
        auto next_it = std::ranges::upper_bound(
            entries, offset, {},
            [](const Entry& entry) -> u64 { return entry.range.getBegin(); });

        // If the offset is before the first entry, return an empty entry
        if (next_it == entries.begin())
            return {.range = {0, next_it->range.getBegin()}, .stream = nullptr};

        auto it = std::prev(next_it);

        // Check if entry is past the range
        if (!it->range.contains(offset)) {
            if (next_it == entries.end())
                return {
                    .range = {it->range.getEnd(), size - offset},
                    .stream = nullptr,
                };

            return {
                .range = {it->range.getEnd(), next_it->range.getBegin()},
                .stream = nullptr,
            };
        }

        // Cache the entry and return it
        cached_entry = *it;
        return *it;
    }
};

} // namespace ztd::io
