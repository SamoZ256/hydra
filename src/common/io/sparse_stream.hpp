#pragma once

#include "common/io/stream.hpp"
#include "common/range.hpp"

namespace hydra::io {

struct SparseStreamEntry {
    Range<u64> range;
    IStream* stream;
};

class SparseStream : public IStream {
  public:
    enum class Error {
        SeekOutOfBounds,
    };

    // Entries must be sorted by offset
    SparseStream(const std::vector<SparseStreamEntry>& entries_, u64 size_)
        : entries{std::move(entries_)}, size{size_} {}

    u64 GetSeek() const override { return seek; }
    void SeekTo(u64 seek_) override { seek = seek_; }
    void SeekBy(u64 offset) override { seek += offset; }

    u64 GetSize() const override { return size; }

    void Flush() override {
        // TODO: only flush modified entries
        for (auto entry : entries)
            entry.stream->Flush();
    }

    void ReadRaw(std::span<u8> buffer) override {
        while (!buffer.empty()) {
            ASSERT_THROWING_DEBUG(seek <= size, Common, Error::SeekOutOfBounds,
                                  "Seek out of bounds");

            const auto entry = GetEntry(seek);
            const auto max_read_size = std::min(
                entry.range.GetEnd() - seek, static_cast<u64>(buffer.size()));
            if (entry.stream) {
                entry.stream->SeekTo(seek - entry.range.GetBegin());
                entry.stream->ReadRaw(buffer.subspan(0, max_read_size));
            } else {
                std::fill(buffer.begin(),
                          buffer.begin() + static_cast<i32>(max_read_size), 0);
            }

            seek += max_read_size;
            buffer = buffer.subspan(max_read_size);
        }
    }

    void WriteRaw(std::span<const u8> buffer) override {
        while (!buffer.empty()) {
            ASSERT_THROWING_DEBUG(seek <= size, Common, Error::SeekOutOfBounds,
                                  "Seek out of bounds");

            const auto entry = GetEntry(seek);
            const auto max_write_size = std::min(
                entry.range.GetEnd() - seek, static_cast<u64>(buffer.size()));
            if (entry.stream) {
                entry.stream->SeekTo(seek - entry.range.GetBegin());
                entry.stream->WriteRaw(buffer.subspan(0, max_write_size));
            }

            seek += max_write_size;
            buffer = buffer.subspan(max_write_size);
        }
    }

  protected:
    std::vector<SparseStreamEntry> entries;

  private:
    u64 size;

    u64 seek{0};
    std::optional<SparseStreamEntry> cached_entry{std::nullopt};

    // Helpers
    SparseStreamEntry GetEntry(u64 offset) {
        // First, check if the entry has been cached
        if (cached_entry.has_value()) {
            const auto entry = cached_entry.value();
            if (entry.range.Contains(offset))
                return entry;
        }

        // Find the entry that contains the offset
        auto next_it =
            std::upper_bound(entries.begin(), entries.end(), offset,
                             [](u64 offset, const SparseStreamEntry& entry) {
                                 return offset < entry.range.GetBegin();
                             });

        // If the offset is before the first entry, return an empty entry
        if (next_it == entries.begin())
            return {.stream = nullptr, .range = {0, next_it->range.GetBegin()}};

        auto it = std::prev(next_it);

        // Check if entry is past the range
        if (!it->range.Contains(offset)) {
            if (next_it == entries.end())
                return {.stream = nullptr,
                        .range = {it->range.GetEnd(), size - offset}};

            return {.stream = nullptr,
                    .range = {it->range.GetEnd(), next_it->range.GetBegin()}};
        }

        // Cache the entry and return it
        cached_entry = *it;
        return *it;
    }
};

class OwnedSparseStream : public SparseStream {
  public:
    using SparseStream::SparseStream;

    ~OwnedSparseStream() override {
        for (auto entry : entries)
            delete entry.stream;
    }
};

} // namespace hydra::io
