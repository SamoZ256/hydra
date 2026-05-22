#include "core/hw/tegra_x1/gpu/renderer/texture_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/memory_util.hpp"
#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_view.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

TextureCache::~TextureCache() {
    for (auto& [mem_key, mem] : entries) {
        for (auto& [group_key, group] : mem.cache) {
            for (auto& [storage_key, storage] : group.cache) {
                for (auto& [view_key, view] : storage.view_cache)
                    delete view;
                delete storage.base;
            }
        }
    }
}

ITextureView* TextureCache::Find(ICommandBuffer* command_buffer,
                                 const TextureDescriptor& descriptor,
                                 TextureUsage usage) {
    return Find(command_buffer, descriptor,
                TextureViewDescriptor(descriptor.type, descriptor.format,
                                      Range<u32>(0, descriptor.level_count),
                                      Range<u32>(0, descriptor.layer_count),
                                      SwizzleChannels()),
                usage);
}

ITextureView* TextureCache::Find(ICommandBuffer* command_buffer,
                                 const TextureDescriptor& descriptor,
                                 const TextureViewDescriptor& view_descriptor,
                                 TextureUsage usage) {
    const auto range = descriptor.GetRange();

    // Check for containing interval
    auto it = entries.upper_bound(range.GetBegin());
    if (it != entries.begin()) {
        auto prev = std::prev(it);
        auto& prev_mem = prev->second;
        if (prev_mem.range.GetEnd() >= range.GetEnd()) {
            // Fully contained
            return AddToMemory(command_buffer, prev_mem, descriptor,
                               view_descriptor, usage);
        }
    }

    // Insert and merge
    TextureMem mem{.range = range};

    it = entries.lower_bound(range.GetBegin());

    // Merge with previous if overlapping
    if (it != entries.begin()) {
        auto prev = std::prev(it);
        auto& prev_mem = prev->second;
        if (prev_mem.range.GetEnd() > mem.range.GetBegin()) {
            MergeMemories(mem, prev_mem);
            it = entries.erase(prev);
        }
    }

    // Merge with following entries
    while (it != entries.end() && it->first < mem.range.GetEnd()) {
        auto& crnt_mem = it->second;
        MergeMemories(mem, crnt_mem);
        it = entries.erase(it);
    }

    // Insert merged interval
    auto inserted = entries.emplace(mem.range.GetBegin(), std::move(mem));
    return AddToMemory(command_buffer, inserted.first->second, descriptor,
                       view_descriptor, usage);
}

void TextureCache::InvalidateMemory(Range<uptr> range) {
    auto it = entries.upper_bound(range.GetBegin());
    if (it != entries.begin())
        it--;

    for (; it != entries.end() && it->first < range.GetEnd(); it++) {
        auto& mem = it->second;

        // We assume that textures that have been written to by the GPU are
        // never modified by the CPU
        if (mem.info.written_timestamp != TextureCacheTimePoint{})
            continue;

        // Check if its in the range
        if (mem.range.GetEnd() > range.GetBegin())
            mem.info.MarkModified();
    }
}

void TextureCache::MergeMemories(TextureMem& mem, TextureMem& other) {
    mem.range = mem.range.Union(other.range);
    mem.info = {
        .modified_timestamp = std::max(mem.info.modified_timestamp,
                                       other.info.modified_timestamp),
        .read_timestamp =
            std::max(mem.info.read_timestamp, other.info.read_timestamp),
        .written_timestamp =
            std::max(mem.info.written_timestamp, other.info.written_timestamp),
    };

    for (auto& [group_key, other_group] : other.cache) {
        auto group_opt = mem.cache.Find(group_key);
        auto& group =
            (group_opt.has_value() ? **group_opt : mem.cache.Add(group_key));
        for (auto& [storage_key, storage] : other_group.cache) {
            group.cache.Add(storage_key, std::move(storage));
        }
    }
}

namespace {

bool CalculateLevelAndLayer(const auto& descriptor,
                            const auto& other_descriptor, u32& out_level,
                            u32& out_layer) {
    const auto offset = static_cast<u32>(other_descriptor.ptr - descriptor.ptr);

    // Layer
    out_layer = 0;
    u32 layer_offset = 0;
    if (descriptor.layer_count > 1) {
        out_layer = offset / descriptor.layer_size;
        layer_offset = out_layer * descriptor.layer_size;
    }
    const u32 level_offset = offset - layer_offset;

    // Level
    out_level = 0;
    u32 crnt_level_offset = 0;
    for (; crnt_level_offset < level_offset;
         crnt_level_offset += descriptor.GetLevelSize(out_level++)) {
    }

    return crnt_level_offset == level_offset;
}

} // namespace

ITextureView*
TextureCache::AddToMemory(ICommandBuffer* command_buffer, TextureMem& mem,
                          const TextureDescriptor& descriptor,
                          const TextureViewDescriptor& view_descriptor,
                          TextureUsage usage) {
    const auto range = descriptor.GetRange();
    const auto group_hash = descriptor.GetGroupHash();
    const auto storage_hash = descriptor.GetStorageHash();

    // Check if it is a new entry
    auto group_opt = mem.cache.Find(group_hash);
    if (!group_opt.has_value()) {
        auto& group = mem.cache.Add(group_hash);
        auto& storage = group.cache.Add(storage_hash);
        return GetTexture(command_buffer, storage, mem, descriptor,
                          view_descriptor, usage);
    }

    auto& group = **group_opt;

    // Check if the storage already exists
    auto storage_opt = group.cache.Find(storage_hash);
    if (storage_opt) {
        auto& storage = **storage_opt;
        return GetTextureView(command_buffer, storage, mem, view_descriptor,
                              usage);
    }

    // ---------------- View ----------------
    for (auto& [key, storage] : group.cache) {
        const auto& other_descriptor = storage.base->GetDescriptor();
        const auto other_range = other_descriptor.GetRange();
        if (other_range.Contains(range)) {
            u32 level, layer;
            if (!CalculateLevelAndLayer(other_descriptor, descriptor, level,
                                        layer)) {
                LOG_WARN(Gpu, "Misaligned textures (existing: ({}), new: ({}))",
                         other_descriptor, descriptor);
                continue;
            }

            auto actual_storage = &storage;

            // Check if the base texture has enough levels
            const u32 min_levels = level + descriptor.level_count;
            if (other_descriptor.level_count < min_levels) {
                // Remove the old storage
                group.cache.Remove(key);

                // Create a new storage
                auto new_descriptor = other_descriptor;
                new_descriptor.level_count = min_levels;
                auto& new_storage =
                    group.cache.Add(new_descriptor.GetStorageHash());
                UpdateStorage(command_buffer, new_storage, mem, new_descriptor,
                              usage);

                // Copy the old storage to the new one
                new_storage.base->CopyFrom(command_buffer, storage.base, 0, 0,
                                           0, 0, other_descriptor.level_count,
                                           other_descriptor.layer_count);

                // TODO: destroy the old storage

                actual_storage = &new_storage;
            }

            return GetTextureView(
                command_buffer, *actual_storage, mem,
                TextureViewDescriptor(
                    view_descriptor.type, view_descriptor.format,
                    Range<u32>::FromSize(level +
                                             view_descriptor.levels.GetBegin(),
                                         view_descriptor.levels.GetSize()),
                    Range<u32>::FromSize(layer +
                                             view_descriptor.layers.GetBegin(),
                                         view_descriptor.layers.GetSize()),
                    view_descriptor.swizzle_channels),
                usage);
        }
    }

    // ---------------- New storage ----------------

    // Find overlapping storages
    struct OverlappingStorage {
        TextureStorage storage;
        u32 level;
        u32 layer;
    };

    std::vector<OverlappingStorage> overlapping_storages;
    u32 level_count = descriptor.level_count;
    u32 layer_count = descriptor.layer_count;
    for (auto it = group.cache.begin(); it != group.cache.end();) {
        auto& storage = it->second;
        const auto& other_descriptor = storage.base->GetDescriptor();
        const auto other_range = other_descriptor.GetRange();
        if (range.Intersects(other_range)) {
            u32 layer = 0;
            u32 level = 0;
            if (other_range.GetBegin() >= range.GetBegin()) {
                if (!CalculateLevelAndLayer(descriptor, other_descriptor, level,
                                            layer)) {
                    LOG_WARN(Gpu,
                             "Misaligned textures (existing: ({}), new: ({}))",
                             other_descriptor, descriptor);
                    ++it;
                    continue;
                }

                // Make sure the new storage can fit the levels and layers of
                // all the old storages
                level_count =
                    std::max(level_count, level + other_descriptor.level_count);
                layer_count =
                    std::max(layer_count, layer + other_descriptor.layer_count);
            } else {
                // TODO
                LOG_NOT_IMPLEMENTED(
                    Gpu, "Texture combining (existing: ({}), new: ({}))",
                    other_descriptor, descriptor);
                ++it;
                continue;
            }

            overlapping_storages.emplace_back(std::move(storage), level, layer);
            it = group.cache.Remove(it);
        } else {
            ++it;
        }
    }

    // Create new descriptor
    auto new_descriptor = descriptor;
    new_descriptor.level_count = level_count;
    new_descriptor.layer_count = layer_count;

    // Create a new storage
    auto& storage = group.cache.Add(storage_hash);
    UpdateStorage(command_buffer, storage, mem, new_descriptor, usage);

    // Copy overlapping storages
    for (auto& overlapping_storage : overlapping_storages) {
        const auto other_base = overlapping_storage.storage.base;
        const auto& other_descriptor = other_base->GetDescriptor();
        storage.base->CopyFrom(
            command_buffer, other_base, 0, 0, overlapping_storage.level,
            overlapping_storage.layer, other_descriptor.level_count,
            other_descriptor.layer_count);
    }

    // TODO: destroy overlapping storages

    // Return view
    return GetTextureView(storage, view_descriptor);
}

void TextureCache::UpdateStorage(ICommandBuffer* command_buffer,
                                 TextureStorage& storage, TextureMem& mem,
                                 const TextureDescriptor& descriptor,
                                 TextureUsage usage) {
    if (!storage.base) {
        storage.base = renderer.CreateTexture(descriptor);
        DecodeTexture(command_buffer, storage);
    }
    Update(command_buffer, storage, mem, usage);
}

ITextureView*
TextureCache::GetTextureView(TextureStorage& storage,
                             const TextureViewDescriptor& view_descriptor) {
    auto view_opt = storage.view_cache.Find(view_descriptor.GetHash());
    if (view_opt.has_value())
        return **view_opt;

    auto view = storage.base->CreateView(view_descriptor);
    storage.view_cache.Add(view_descriptor.GetHash(), view);
    return view;
}

ITextureView* TextureCache::GetTextureView(
    ICommandBuffer* command_buffer, TextureStorage& storage, TextureMem& mem,
    const TextureViewDescriptor& view_descriptor, TextureUsage usage) {
    Update(command_buffer, storage, mem, usage);
    return GetTextureView(storage, view_descriptor);
}

ITextureView* TextureCache::GetTexture(
    ICommandBuffer* command_buffer, TextureStorage& storage, TextureMem& mem,
    const TextureDescriptor& descriptor,
    const TextureViewDescriptor& view_descriptor, TextureUsage usage) {
    UpdateStorage(command_buffer, storage, mem, descriptor, usage);
    return GetTextureView(storage, view_descriptor);
}

void TextureCache::Update(ICommandBuffer* command_buffer,
                          TextureStorage& storage, TextureMem& mem,
                          TextureUsage usage) {
    bool sync = false;
    if (storage.update_timestamp < mem.info.modified_timestamp) {
        // If modified by the guest
        sync = true;
    } else if (storage.update_timestamp < mem.info.written_timestamp) {
        // Other textures in this memory changed, let's copy them
        const auto base = storage.base;
        const auto& descriptor = base->GetDescriptor();
        const auto range = descriptor.GetRange();
        for (auto& [group_key, group] : mem.cache) {
            // Skip this group
            if (group_key == storage.base->GetDescriptor().GetGroupHash())
                continue;

            for (auto& [storage_key, other_storage] : group.cache) {
                const auto other_base = other_storage.base;
                const auto& other_descriptor = other_base->GetDescriptor();
                const auto other_range = other_descriptor.GetRange();

                // Check if the textures can actually be copied
                if (other_descriptor.width != descriptor.width ||
                    other_descriptor.height != descriptor.height)
                    continue;

                // TODO: rework
                if (range.Intersects(other_range)) {
                    const auto copy_range = range.ClampedTo(other_range);
                    const auto dst_offset =
                        copy_range.GetBegin() - range.GetBegin();

                    if (descriptor.type != TextureType::_3D &&
                        other_descriptor.type !=
                            TextureType::_3D) { // Neither 3D
                        // Layer
                        const auto src_layer = static_cast<u32>(
                            (copy_range.GetBegin() - other_range.GetBegin()) /
                            descriptor.layer_size);
                        const auto dst_layer = static_cast<u32>(
                            dst_offset / descriptor.layer_size);
                        const auto layer_count = static_cast<u32>(
                            copy_range.GetSize() / descriptor.layer_size);

                        // Copy
                        // TODO: make sure the formats match
                        base->CopyFrom(command_buffer, other_base, 0, src_layer,
                                       0, dst_layer,
                                       std::min(descriptor.level_count,
                                                other_descriptor.level_count),
                                       layer_count);
                    } else if (descriptor.type == TextureType::_3D &&
                               other_descriptor.type ==
                                   TextureType::_3D) { // Both 3D
                        // TODO: what about multiple levels?

                        const auto slice_size = GetTextureFormatSliceStride(
                            descriptor.format, descriptor.width,
                            descriptor.height);

                        // Z
                        const auto src_z = static_cast<u32>(
                            (copy_range.GetBegin() - other_range.GetBegin()) /
                            slice_size);
                        const auto dst_z =
                            static_cast<u32>(dst_offset / slice_size);
                        const auto z_count =
                            static_cast<u32>(copy_range.GetSize() / slice_size);

                        // Copy
                        // TODO: make sure the formats match
                        base->CopyFrom(command_buffer, other_base,
                                       uint3({0, 0, src_z}), 0, 0,
                                       uint3({0, 0, dst_z}), 0, 0,
                                       uint3({descriptor.width,
                                              descriptor.height, z_count}),
                                       1);
                    } else if (descriptor.type == TextureType::_3D &&
                               other_descriptor.type ==
                                   TextureType::_2D) { // HACK: special case
                        // TODO: what about multiple levels?

                        const auto slice_size = GetTextureFormatSliceStride(
                            descriptor.format, descriptor.width,
                            descriptor.height);

                        // Z
                        const auto dst_z =
                            static_cast<u32>(dst_offset / slice_size);

                        // Copy
                        // TODO: make sure the formats match
                        base->CopyFrom(
                            command_buffer, other_base, uint3({0, 0, 0}), 0, 0,
                            uint3({0, 0, dst_z}), 0, 0,
                            uint3({descriptor.width, descriptor.height, 1}), 1);
                    } else {
                        LOG_WARN(Gpu,
                                 "Unimplemented texture copy (source: {}, "
                                 "destination: {})",
                                 descriptor.type, other_descriptor.type);
                    }
                }
            }
        }

        storage.MarkUpdated();
    } else if (mem.info.written_timestamp == TextureCacheTimePoint{}) {
        // Never written to
        if (usage == TextureUsage::Present) {
            // Presented, but never written to
            sync = true;
        } else if (usage == TextureUsage::Read) {
            // Read, but never written to
        }
    }

    if (sync)
        DecodeTexture(command_buffer, storage);

    if (usage == TextureUsage::Read)
        mem.info.MarkRead();
    else if (usage == TextureUsage::Write)
        mem.info.MarkWritten();

    if (usage == TextureUsage::Write || sync)
        storage.MarkUpdated();
}

u32 TextureCache::GetDataHash(const ITexture* texture) {
    constexpr u32 SAMPLE_COUNT = 37;

    const auto& descriptor = texture->GetDescriptor();
    u64 mem_range = descriptor.size;
    u64 mem_step = std::max(mem_range / SAMPLE_COUNT, 1ull);

    HashCode hash;
    for (u64 offset = 0; offset < mem_range; offset += mem_step)
        hash.Add(*reinterpret_cast<u64*>(descriptor.ptr + offset));

    return hash.ToHashCode();
}

void TextureCache::DecodeTexture(ICommandBuffer* command_buffer,
                                 TextureStorage& storage) {
    const auto& descriptor = storage.base->GetDescriptor();

    // Calculate size
    u32 size = 0;
    for (u32 level = 0; level < descriptor.level_count; level++) {
        const auto dims = descriptor.GetLevelDimensions(level);
        const u32 stride = GetTextureFormatStride(descriptor.format, dims.x());
        const u32 rows = GetTextureFormatRows(descriptor.format, dims.y());
        const u32 slice_stride = rows * stride;
        size += dims.z() * slice_stride;
    }
    size *= descriptor.layer_count;

    // Allocate temporary buffer
    auto tmp_buffer = renderer.AllocateTemporaryBuffer(size);

    const u8* in_data = reinterpret_cast<const u8*>(descriptor.ptr);
    u8* out_data = reinterpret_cast<u8*>(tmp_buffer->GetPtr());
    if (descriptor.is_linear) {
        const u32 stride =
            GetTextureFormatStride(descriptor.format, descriptor.width);
        const u32 rows =
            GetTextureFormatRows(descriptor.format, descriptor.height);
        for (u32 row = 0; row < rows; row++) {
            std::memcpy(out_data + row * stride,
                        in_data + row * descriptor.linear_stride, stride);
        }
    } else {
        u32 offset = 0;
        for (u32 layer = 0; layer < descriptor.layer_count; layer++) {
            for (u32 level = 0; level < descriptor.level_count; level++) {
                // Calculate sizes
                const auto dims = descriptor.GetLevelDimensions(level);
                const auto block_size_log2 =
                    descriptor.GetLevelBlockSizeLog2(level);
                const u32 stride =
                    GetTextureFormatStride(descriptor.format, dims.x());
                const u32 rows =
                    GetTextureFormatRows(descriptor.format, dims.y());
                const u32 slice_stride = rows * stride;

                // Convert
                ConvertBlockLinearToLinear(
                    stride, rows, dims.z(), block_size_log2.y(),
                    block_size_log2.z(), in_data + offset,
                    [=](const u8* in_gob, u32 gob_x, u32 gob_y, u32 gob_z) {
                        const u32 x = gob_x * GOB_WIDTH;
                        for (u32 local_y = 0; local_y < GOB_HEIGHT; local_y++) {
                            const u32 y = gob_y * GOB_HEIGHT + local_y;
                            if (y >= rows)
                                break;

                            const u32 crnt_offset =
                                offset + gob_z * slice_stride + y * stride + x;
                            std::memcpy(out_data + crnt_offset,
                                        in_gob + local_y * GOB_WIDTH,
                                        std::min(GOB_WIDTH, stride - x));
                        }
                    });

                // Add offset
                offset += dims.z() * slice_stride;
            }
        }
    }

    storage.base->CopyFrom(command_buffer, tmp_buffer);
    renderer.FreeTemporaryBuffer(tmp_buffer);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
