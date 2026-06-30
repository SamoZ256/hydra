#pragma once

#include "common/io/stream.hpp"

namespace hydra::io {

class SparseStream : public IStream {
  public:
    struct Entry {
        ztd::Range<u64> range;
        IStream* stream;
    };

    // Entries must be sorted by offset
    SparseStream(std::vector<Entry> entries_, u64 size_)
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
            ASSERT_DEBUG(seek + buffer.size() <= size, Common,
                         "Seek out of bounds ({} > {})", seek + buffer.size(),
                         size);

            const auto entry = GetEntry(seek);
            const auto max_read_size = std::min(
                entry.range.getEnd() - seek, static_cast<u64>(buffer.size()));
            if (entry.stream != nullptr) {
                entry.stream->SeekTo(seek - entry.range.getBegin());
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
            ASSERT_DEBUG(seek + buffer.size() <= size, Common,
                         "Seek out of bounds ({} > {})", seek + buffer.size(),
                         size);

            const auto entry = GetEntry(seek);
            const auto max_write_size = std::min(
                entry.range.getEnd() - seek, static_cast<u64>(buffer.size()));
            if (entry.stream != nullptr) {
                entry.stream->SeekTo(seek - entry.range.getBegin());
                entry.stream->WriteRaw(buffer.subspan(0, max_write_size));
            }

            seek += max_write_size;
            buffer = buffer.subspan(max_write_size);
        }
    }

  protected:
    std::vector<Entry> entries;

  private:
    u64 size;

    u64 seek{0};
    std::optional<Entry> cached_entry{std::nullopt};

    // Helpers
    Entry GetEntry(u64 offset) {
        // First, check if the entry has been cached
        if (cached_entry.has_value()) {
            const auto entry = cached_entry.value();
            if (entry.range.contains(offset))
                return entry;
        }

        // Find the entry that contains the offset
        auto next_it =
            std::upper_bound(entries.begin(), entries.end(), offset,
                             [](u64 offset, const Entry& entry) {
                                 return offset < entry.range.getBegin();
                             });

        // If the offset is before the first entry, return an empty entry
        if (next_it == entries.begin())
            return {.range = {0, next_it->range.getBegin()}, .stream = nullptr};

        auto it = std::prev(next_it);

        // Check if entry is past the range
        if (!it->range.contains(offset)) {
            if (next_it == entries.end())
                return {.range = {it->range.getEnd(), size - offset},
                        .stream = nullptr};

            return {.range = {it->range.getEnd(), next_it->range.getBegin()},
                    .stream = nullptr};
        }

        // Cache the entry and return it
        cached_entry = *it;
        return *it;
    }
};

// TODO: remove
class OwnedSparseStream : public SparseStream {
  public:
    using SparseStream::SparseStream;

    ~OwnedSparseStream() override {
        for (auto entry : entries)
            delete entry.stream;
    }

    ZTD_MAKE_NON_COPYABLE(OwnedSparseStream);
};

} // namespace hydra::io
