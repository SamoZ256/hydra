#pragma once

#include "core/horizon/filesystem/file.hpp"

namespace hydra::horizon::filesystem {

struct SparseFileEntry {
    u64 offset;
    IFile* file;
};

class SparseFile : public IFile {
  public:
    SparseFile(std::span<SparseFileEntry> entries_, usize size_) : size{size_} {
        // Sort the entries by offset
        entries.assign(entries_.begin(), entries_.end());
        std::sort(entries.begin(), entries.end(),
                  [](const SparseFileEntry& a, const SparseFileEntry& b) {
                      return a.offset < b.offset;
                  });

        // TODO: revisit this idea
        /*
        // Try to merge entries
        entries.reserve(entries_.size());
        u32 begin_index = 0;

        const auto push_merged_view = [&](u32 crnt_index) {
            // If begin and end are the same, just push the original entry
            if (begin_index == crnt_index - 1) {
                entries.push_back(entries_[begin_index]);
                return;
            }

            const auto& begin_entry = entries_[begin_index];
            const auto begin_view = static_cast<FileView*>(begin_entry.file);
            const auto end_view =
                static_cast<FileView*>(entries_[crnt_index - 1].file);

            const auto merged_view =
                new FileView(begin_view->GetBase(), begin_view->GetOffset(),
                             end_view->GetOffset() + end_view->GetSize() -
                                 begin_view->GetOffset());
            const auto merged_entry =
                SparseFileEntry{begin_entry.offset, merged_view};
            entries.push_back(merged_entry);

            begin_index = crnt_index;
        };

        for (u32 i = 0; i < entries_.size(); i++) {
            const auto& entry = entries_[i];
            if (const auto view = dynamic_cast<FileView*>(entry.file)) {
                if (begin_index != i) {
                    const auto& prev_entry = entries_[i - 1];
                    const auto prev_view =
                        static_cast<FileView*>(prev_entry.file);
                    if (entry.offset - prev_entry.offset ==
                        view->GetOffset() - prev_view->GetOffset()) {
                        // Can be merged
                        continue;
                    } else {
                        push_merged_view(i);
                    }
                }
            } else {
                if (begin_index != i) {
                    push_merged_view(i);
                }

                entries.push_back(entries_[i]);
                begin_index = i + 1;
            }
        }

        // Check if we have pushed all entries
        if (begin_index != entries_.size()) {
            push_merged_view(static_cast<u32>(entries_.size()));
        }
        */
    }

    io::IStream* Open(FileOpenFlags flags) override {
        std::vector<io::SparseStreamEntry> streams;
        streams.reserve(entries.size());
        for (const auto& entry : entries) {
            streams.push_back(
                {range(entry.offset, entry.offset + entry.file->GetSize()),
                 entry.file->Open(flags)});
        }

        return new io::OwnedSparseStream(std::move(streams), size);
    }

    usize GetSize() override { return size; }

  private:
    std::vector<SparseFileEntry> entries;
    usize size;
};

} // namespace hydra::horizon::filesystem
