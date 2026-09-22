#include "core/hw/tegra_x1/gpu/renderer/texture_cache.hpp"

#include <cstddef>

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

ITextureView* TextureCache::find(ICommandBuffer* command_buffer,
                                 const TextureDescriptor& descriptor,
                                 TextureUsage usage) {
    return find(
        command_buffer, descriptor,
        TextureViewDescriptor(descriptor.type, descriptor.format,
                              ztd::Range<u32>(0, descriptor.level_count),
                              ztd::Range<u32>(0, descriptor.layer_count),
                              SwizzleChannels()),
        usage);
}

ITextureView* TextureCache::find(ICommandBuffer* command_buffer,
                                 const TextureDescriptor& descriptor,
                                 const TextureViewDescriptor& view_descriptor,
                                 TextureUsage usage) {
    const auto range = descriptor.getRange();

    // Check for containing interval
    auto it = entries.upper_bound(range.getBegin());
    if (it != entries.begin()) {
        auto prev = std::prev(it);
        auto& prev_mem = prev->second;
        if (prev_mem.range.getEnd() >= range.getEnd()) {
            // Fully contained
            return addToMemory(command_buffer, prev_mem, descriptor,
                               view_descriptor, usage);
        }
    }

    // Insert and merge
    TextureMem mem{.range = range};

    it = entries.lower_bound(range.getBegin());

    // Merge with previous if overlapping
    if (it != entries.begin()) {
        auto prev = std::prev(it);
        auto& prev_mem = prev->second;
        if (prev_mem.range.getEnd() > mem.range.getBegin()) {
            mergeMemories(mem, prev_mem);
            it = entries.erase(prev);
        }
    }

    // Merge with following entries
    while (it != entries.end() && it->first < mem.range.getEnd()) {
        auto& crnt_mem = it->second;
        mergeMemories(mem, crnt_mem);
        it = entries.erase(it);
    }

    // Insert merged interval
    auto inserted = entries.emplace(mem.range.getBegin(), std::move(mem));
    return addToMemory(command_buffer, inserted.first->second, descriptor,
                       view_descriptor, usage);
}

void TextureCache::invalidateMemory(ztd::Range<uptr> range) {
    auto it = entries.upper_bound(range.getBegin());
    if (it != entries.begin())
        it--;

    for (; it != entries.end() && it->first < range.getEnd(); it++) {
        auto& mem = it->second;

        // We assume that textures that have been written to by the GPU are
        // never modified by the CPU
        if (mem.info.written_timestamp != TextureCacheTimePoint{})
            continue;

        // Check if its in the range
        if (mem.range.getEnd() > range.getBegin())
            mem.info.markModified();
    }
}

void TextureCache::mergeMemories(TextureMem& mem, TextureMem& other) {
    mem.range = mem.range.merged(other.range);
    mem.info = {
        .modified_timestamp = std::max(mem.info.modified_timestamp,
                                       other.info.modified_timestamp),
        .read_timestamp =
            std::max(mem.info.read_timestamp, other.info.read_timestamp),
        .written_timestamp =
            std::max(mem.info.written_timestamp, other.info.written_timestamp),
    };

    for (auto& [group_key, other_group] : other.cache) {
        auto group_opt = mem.cache.find(group_key);
        auto& group =
            (group_opt.has_value() ? **group_opt : mem.cache.insert(group_key));
        for (auto& [storage_key, storage] : other_group.cache) {
            group.cache.insert(storage_key, std::move(storage));
        }
    }
}

namespace {

bool calculateLevelAndLayer(const TextureDescriptor& base_descriptor, uptr ptr,
                            u32& out_level, u32& out_layer) {
    const auto offset = static_cast<u32>(ptr - base_descriptor.ptr);

    // Layer
    out_layer = 0;
    u32 layer_offset = 0;
    if (base_descriptor.layer_count > 1) {
        out_layer = offset / base_descriptor.layer_size;
        layer_offset = out_layer * base_descriptor.layer_size;
    }

    // Level
    out_level = 0;
    u32 crnt_level_offset = 0;
    const u32 level_offset = offset - layer_offset;
    for (; crnt_level_offset < level_offset;
         crnt_level_offset += base_descriptor.getLevelSize(out_level++)) {
    }

    // Check if level is aligned
    return crnt_level_offset == level_offset;
}

bool calculateLevelAndLayer(const TextureDescriptor& base_descriptor,
                            const TextureDescriptor& view_descriptor,
                            u32& out_level, u32& out_layer) {
    if (!calculateLevelAndLayer(base_descriptor, view_descriptor.ptr, out_level,
                                out_layer))
        return false;

    if (base_descriptor.getLevelDimensions(out_level) !=
        uint3({view_descriptor.width, view_descriptor.height,
               view_descriptor.depth}))
        return false;

    return true;
}

bool calculateLevelAndLayer(const TextureDescriptor& base1_descriptor,
                            const TextureDescriptor& base2_descriptor, uptr ptr,
                            u32& out_level1, u32& out_layer1, u32& out_level2,
                            u32& out_layer2) {
    if (!calculateLevelAndLayer(base1_descriptor, ptr, out_level1, out_layer1))
        return false;

    if (!calculateLevelAndLayer(base2_descriptor, ptr, out_level2, out_layer2))
        return false;

    if (base1_descriptor.getLevelDimensions(out_level1) !=
        base2_descriptor.getLevelDimensions(out_level2))
        return false;

    return true;
}

bool calculateLevelAndSlice(const TextureDescriptor& base_descriptor, uptr ptr,
                            u32& out_level, u32& out_slice) {
    const auto offset = static_cast<u32>(ptr - base_descriptor.ptr);

    // Level
    out_level = 0;
    u32 level_offset = 0;
    while (level_offset < offset) {
        const u32 level_size = base_descriptor.getLevelSize(out_level);
        if (level_offset + level_size >= offset)
            break;

        level_offset += level_size;
        out_level++;
    }

    // Slice
    const u32 slice_offset = offset - level_offset;
    const u32 slice_size = base_descriptor.size / base_descriptor.depth;
    out_slice = slice_offset / slice_size;

    // Check if slice is aligned
    return out_slice * slice_size == slice_offset;
}

bool calculateLevelAndSlice(const TextureDescriptor& base1_descriptor,
                            const TextureDescriptor& base2_descriptor, uptr ptr,
                            u32& out_level1, u32& out_slice1, u32& out_level2,
                            u32& out_slice2) {
    if (!calculateLevelAndSlice(base1_descriptor, ptr, out_level1, out_slice1))
        return false;

    if (!calculateLevelAndSlice(base2_descriptor, ptr, out_level2, out_slice2))
        return false;

    const auto dims1 = base1_descriptor.getLevelDimensions(out_level1);
    const auto dims2 = base2_descriptor.getLevelDimensions(out_level2);
    // Z can differ
    return dims1.x() == dims2.x() && dims1.y() == dims2.y();
}

} // namespace

ITextureView*
TextureCache::addToMemory(ICommandBuffer* command_buffer, TextureMem& mem,
                          const TextureDescriptor& descriptor,
                          const TextureViewDescriptor& view_descriptor,
                          TextureUsage usage) {
    const auto range = descriptor.getRange();
    const auto group_hash = descriptor.getGroupHash();
    const auto storage_hash = descriptor.getStorageHash();

    // Check if it is a new entry
    auto group_opt = mem.cache.find(group_hash);
    if (!group_opt.has_value()) {
        auto& group = mem.cache.insert(group_hash);
        auto& storage = group.cache.insert(storage_hash);
        return getTexture(command_buffer, storage, mem, descriptor,
                          view_descriptor, usage);
    }

    auto& group = **group_opt;

    // Check if the storage already exists
    auto storage_opt = group.cache.find(storage_hash);
    if (storage_opt) {
        auto& storage = **storage_opt;
        return getTextureView(command_buffer, storage, mem, view_descriptor,
                              usage);
    }

    // ---------------- View ----------------
    for (auto& [key, storage] : group.cache) {
        const auto& other_descriptor = storage.base->getDescriptor();
        const auto other_range = other_descriptor.getRange();
        if (other_range.contains(range)) {
            u32 level;
            u32 layer;
            if (!calculateLevelAndLayer(other_descriptor, descriptor, level,
                                        layer)) {
                LOG_DEBUG(Gpu,
                          "Misaligned textures (existing: ({}), new: ({}))",
                          other_descriptor, descriptor);
                continue;
            }

            auto actual_storage = &storage;

            // Check if the base texture has enough levels
            const u32 min_levels = level + descriptor.level_count;
            if (other_descriptor.level_count < min_levels) {
                // Remove the old storage
                group.cache.remove(key);

                // Create a new storage
                auto new_descriptor = other_descriptor;
                new_descriptor.level_count = min_levels;
                auto& new_storage =
                    group.cache.insert(new_descriptor.getStorageHash());
                updateStorage(command_buffer, new_storage, mem, new_descriptor,
                              usage);

                // Copy the old storage to the new one
                new_storage.base->copyFrom(command_buffer, storage.base, 0, 0,
                                           0, 0, other_descriptor.level_count,
                                           other_descriptor.layer_count);

                // TODO: destroy the old storage

                actual_storage = &new_storage;
            }

            return getTextureView(
                command_buffer, *actual_storage, mem,
                TextureViewDescriptor(
                    view_descriptor.type, view_descriptor.format,
                    ztd::Range<u32>::fromSize(
                        level + view_descriptor.levels.getBegin(),
                        view_descriptor.levels.getSize()),
                    ztd::Range<u32>::fromSize(
                        layer + view_descriptor.layers.getBegin(),
                        view_descriptor.layers.getSize()),
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
        const auto& other_descriptor = storage.base->getDescriptor();
        const auto other_range = other_descriptor.getRange();
        if (range.intersects(other_range)) {
            u32 layer = 0;
            u32 level = 0;
            if (other_range.getBegin() >= range.getBegin()) {
                if (!calculateLevelAndLayer(descriptor, other_descriptor, level,
                                            layer)) {
                    LOG_DEBUG(Gpu,
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
            it = group.cache.remove(it);
        } else {
            ++it;
        }
    }

    // Create new descriptor
    auto new_descriptor = descriptor;
    new_descriptor.level_count = level_count;
    new_descriptor.layer_count = layer_count;

    // Create a new storage
    auto& storage = group.cache.insert(storage_hash);
    updateStorage(command_buffer, storage, mem, new_descriptor, usage);

    // Copy overlapping storages
    for (auto& overlapping_storage : overlapping_storages) {
        const auto other_base = overlapping_storage.storage.base;
        const auto& other_descriptor = other_base->getDescriptor();
        storage.base->copyFrom(
            command_buffer, other_base, 0, 0, overlapping_storage.level,
            overlapping_storage.layer, other_descriptor.level_count,
            other_descriptor.layer_count);
    }

    // TODO: destroy overlapping storages

    // Return view
    return getTextureView(storage, view_descriptor);
}

void TextureCache::updateStorage(ICommandBuffer* command_buffer,
                                 TextureStorage& storage, TextureMem& mem,
                                 const TextureDescriptor& descriptor,
                                 TextureUsage usage) {
    if (storage.base == nullptr) {
        storage.base = renderer.createTexture(descriptor);
        decodeTexture(command_buffer, storage);
    }
    update(command_buffer, storage, mem, usage);
}

ITextureView*
TextureCache::getTextureView(TextureStorage& storage,
                             const TextureViewDescriptor& view_descriptor) {
    auto view_opt = storage.view_cache.find(view_descriptor.getHash());
    if (view_opt.has_value())
        return **view_opt;

    auto view = storage.base->createView(view_descriptor);
    storage.view_cache.insert(view_descriptor.getHash(), view);
    return view;
}

ITextureView* TextureCache::getTextureView(
    ICommandBuffer* command_buffer, TextureStorage& storage, TextureMem& mem,
    const TextureViewDescriptor& view_descriptor, TextureUsage usage) {
    update(command_buffer, storage, mem, usage);
    return getTextureView(storage, view_descriptor);
}

ITextureView* TextureCache::getTexture(
    ICommandBuffer* command_buffer, TextureStorage& storage, TextureMem& mem,
    const TextureDescriptor& descriptor,
    const TextureViewDescriptor& view_descriptor, TextureUsage usage) {
    updateStorage(command_buffer, storage, mem, descriptor, usage);
    return getTextureView(storage, view_descriptor);
}

void TextureCache::update(ICommandBuffer* command_buffer,
                          TextureStorage& storage, TextureMem& mem,
                          TextureUsage usage) {
    bool sync = false;
    if (storage.update_timestamp < mem.info.modified_timestamp) {
        // If modified by the guest
        sync = true;
    } else if (storage.update_timestamp < mem.info.written_timestamp) {
        // Other textures in this memory changed, let's copy them
        const auto base = storage.base;
        const auto& descriptor = base->getDescriptor();
        const auto range = descriptor.getRange();
        for (auto& [group_key, group] : mem.cache) {
            for (auto& [storage_key, other_storage] : group.cache) {
                // Skip this storage
                if (storage_key ==
                    storage.base->getDescriptor().getStorageHash())
                    continue;

                const auto& other_descriptor =
                    other_storage.base->getDescriptor();
                const auto other_range = other_descriptor.getRange();

                if (range.intersects(other_range)) {
                    const auto type_class =
                        getTextureTypeClass(descriptor.type);
                    const auto other_type_class =
                        getTextureTypeClass(other_descriptor.type);
                    if (type_class == TextureTypeClass::_2D &&
                        other_type_class == TextureTypeClass::_2D) {
                        synchronize2DWith2D(command_buffer, storage,
                                            other_storage);
                    } else if (type_class == TextureTypeClass::_3D &&
                               other_type_class == TextureTypeClass::_3D) {
                        synchronize3DWith3D(command_buffer, storage,
                                            other_storage);
                    } else {
                        LOG_WARN(Gpu,
                                 "Cannot synchronize textures ({}) and ({})",
                                 descriptor, other_descriptor);
                    }
                }
            }
        }

        storage.markUpdated();
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
        decodeTexture(command_buffer, storage);

    if (usage == TextureUsage::Read)
        mem.info.markRead();
    else if (usage == TextureUsage::Write)
        mem.info.markWritten();

    if (usage == TextureUsage::Write || sync)
        storage.markUpdated();
}

void TextureCache::synchronize2DWith2D(ICommandBuffer* command_buffer,
                                       TextureStorage& storage,
                                       TextureStorage& other_storage) {
    const auto& descriptor = storage.base->getDescriptor();
    const auto& other_descriptor = other_storage.base->getDescriptor();
    const auto copy_range =
        descriptor.getRange().clampedTo(other_descriptor.getRange());

    u32 level;
    u32 layer;
    u32 other_level;
    u32 other_layer;
    if (!calculateLevelAndLayer(descriptor, other_descriptor,
                                copy_range.getBegin(), level, layer,
                                other_level, other_layer)) {
        LOG_DEBUG(Gpu, "Cannot synchronize 2D textures ({}) and ({})",
                  descriptor, other_descriptor);
        return;
    }

    storage.base->copyFrom(
        command_buffer, other_storage.base, other_layer, level, layer,
        other_level,
        std::min(descriptor.level_count - level,
                 other_descriptor.level_count - other_level),
        std::min(descriptor.layer_count - layer,
                 other_descriptor.layer_count - other_layer));
}

void TextureCache::synchronize3DWith3D(ICommandBuffer* command_buffer,
                                       TextureStorage& storage,
                                       TextureStorage& other_storage) {
    const auto& descriptor = storage.base->getDescriptor();
    const auto& other_descriptor = other_storage.base->getDescriptor();
    const auto copy_range =
        descriptor.getRange().clampedTo(other_descriptor.getRange());

    u32 level;
    u32 slice;
    u32 other_level;
    u32 other_slice;
    if (!calculateLevelAndSlice(descriptor, other_descriptor,
                                copy_range.getBegin(), level, slice,
                                other_level, other_slice)) {
        LOG_DEBUG(Gpu, "Cannot synchronize 3D textures ({}) and ({})",
                  descriptor, other_descriptor);
        return;
    }

    const auto dims = descriptor.getLevelDimensions(level);
    const auto other_dims = other_descriptor.getLevelDimensions(other_level);
    const u32 level_count =
        std::min(descriptor.level_count - level,
                 other_descriptor.level_count - other_level);

    if (slice == 0 && other_slice == 0 && dims.z() == other_dims.z()) {
        // Do a simplified copy in case we are copying whole levels
        storage.base->copyFrom(command_buffer, other_storage.base, other_level,
                               0, level, 0, level_count, 1);
    } else {
        ASSERT_DEBUG(
            level_count == 1, Gpu,
            "Cannot copy multiple 3D levels with non-matching dimensions");
        const u32 slice_count =
            std::min(dims.z() - slice, other_dims.z() - other_slice);
        storage.base->copyFrom(command_buffer, other_storage.base,
                               uint3({0, 0, other_slice}), other_level, 0,
                               uint3({0, 0, slice}), level, 0,
                               uint3({dims.x(), dims.y(), slice_count}), 1);
    }
}

u32 TextureCache::getDataHash(const ITexture* texture) {
    constexpr u32 SAMPLE_COUNT = 37;

    const auto& descriptor = texture->getDescriptor();
    u64 mem_range = descriptor.size;
    u64 mem_step = std::max(mem_range / SAMPLE_COUNT, 1ull);

    ztd::hash::XxHash32 hash;
    for (u64 offset = 0; offset < mem_range; offset += mem_step)
        hash.add(*reinterpret_cast<u64*>(descriptor.ptr + offset));

    return hash.toHashCode();
}

void TextureCache::decodeTexture(ICommandBuffer* command_buffer,
                                 TextureStorage& storage) {
    const auto& descriptor = storage.base->getDescriptor();

    // Calculate size
    u32 size = 0;
    for (u32 level = 0; level < descriptor.level_count; level++) {
        const auto dims = descriptor.getLevelDimensions(level);
        const u32 stride = getTextureFormatStride(descriptor.format, dims.x());
        const u32 rows = getTextureFormatRows(descriptor.format, dims.y());
        const u32 slice_stride = rows * stride;
        size += dims.z() * slice_stride;
    }
    size *= descriptor.layer_count;

    // Allocate temporary buffer
    auto tmp_buffer = renderer.allocateTemporaryBuffer(size);

    const u8* in_data = reinterpret_cast<const u8*>(descriptor.ptr);
    u8* out_data = reinterpret_cast<u8*>(tmp_buffer->getPtr());
    if (descriptor.is_linear) {
        const u32 stride =
            getTextureFormatStride(descriptor.format, descriptor.width);
        const u32 rows =
            getTextureFormatRows(descriptor.format, descriptor.height);
        for (u32 row = 0; row < rows; row++) {
            std::memcpy(out_data + static_cast<usize>(row) * stride,
                        in_data +
                            static_cast<usize>(row) * descriptor.linear_stride,
                        stride);
        }
    } else {
        u32 offset = 0;
        for (u32 layer = 0; layer < descriptor.layer_count; layer++) {
            for (u32 level = 0; level < descriptor.level_count; level++) {
                // Calculate sizes
                const auto dims = descriptor.getLevelDimensions(level);
                const auto block_size_log2 =
                    descriptor.getLevelBlockSizeLog2(level);
                const u32 stride =
                    getTextureFormatStride(descriptor.format, dims.x());
                const u32 rows =
                    getTextureFormatRows(descriptor.format, dims.y());
                const u32 slice_stride = rows * stride;

                // Convert
                convertBlockLinearToLinear(
                    stride, rows, dims.z(), block_size_log2.y(),
                    block_size_log2.z(), in_data + offset,
                    [=](const u8* in_gob, u32 gob_x, u32 gob_y, u32 gob_z) {
                        const u32 x = gob_x * GOB_WIDTH;
                        for (u32 local_y = 0; local_y < GOB_HEIGHT; local_y++) {
                            const u32 y = gob_y * GOB_HEIGHT + local_y;
                            if (y >= rows)
                                break;

                            const usize crnt_offset =
                                offset +
                                static_cast<usize>(gob_z) * slice_stride +
                                static_cast<usize>(y) * stride + x;
                            std::memcpy(out_data + crnt_offset,
                                        in_gob + static_cast<usize>(local_y) *
                                                     GOB_WIDTH,
                                        std::min(GOB_WIDTH, stride - x));
                        }
                    });

                // Add offset
                offset += dims.z() * slice_stride;
            }
        }
    }

    storage.base->copyFrom(command_buffer, tmp_buffer);
    renderer.freeTemporaryBuffer(tmp_buffer);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
