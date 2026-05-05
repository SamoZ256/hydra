#include "core/hw/tegra_x1/gpu/renderer/texture_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/memory_util.hpp"
#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_view.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

TextureCache::~TextureCache() {
    for (auto& [key, mem] : entries) {
        for (auto& [key, group] : mem.cache) {
            for (auto& [key, storage] : group.cache) {
                delete storage.base;
                for (auto& [key, view] : storage.view_cache)
                    delete view;
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

    for (auto& [key, tex] : other.cache)
        mem.cache.Add(key, std::move(tex));
}

ITextureView*
TextureCache::AddToMemory(ICommandBuffer* command_buffer, TextureMem& mem,
                          const TextureDescriptor& descriptor,
                          const TextureViewDescriptor& view_descriptor,
                          TextureUsage usage) {
    const auto range = descriptor.GetRange();

    // Check if it is a new entry
    auto group_opt = mem.cache.Find(descriptor.GetHash());
    if (!group_opt.has_value()) {
        auto& group = mem.cache.Add(descriptor.GetHash());
        auto& storage = group.cache.Add(descriptor.ptr);
        return GetTexture(command_buffer, storage, mem, descriptor,
                          view_descriptor, usage);
    }

    auto& group = **group_opt;

    // Check if it is just a view with smaller layer count
    auto storage_opt = group.cache.Find(descriptor.ptr);
    if (storage_opt) {
        auto& storage = **storage_opt;
        if (storage.base->GetDescriptor().GetRange().Contains(range))
            return GetTextureView(command_buffer, storage, mem, view_descriptor,
                                  usage);
        else
            group.cache.Remove(descriptor.ptr);
    }

    // Check if it is a proper layer view
    for (auto& [key, storage] : group.cache) {
        if (storage.base->GetDescriptor().GetRange().Contains(range)) {
            const auto offset = static_cast<u32>(
                range.GetBegin() - storage.base->GetDescriptor().ptr);
            ASSERT_ALIGNMENT_DEBUG(offset, descriptor.layer_size, Gpu,
                                   "texture view offset");
            const u32 layer_offset = offset / descriptor.layer_size;
            return GetTextureView(
                command_buffer, storage, mem,
                TextureViewDescriptor(
                    view_descriptor.type, view_descriptor.format,
                    view_descriptor.levels,
                    Range<u32>::FromSize(layer_offset +
                                             view_descriptor.layers.GetBegin(),
                                         view_descriptor.layers.GetSize()),
                    view_descriptor.swizzle_channels),
                usage);
        }
    }

    // HACK: create a new texture
    auto& storage = group.cache.Add(descriptor.ptr);
    return GetTexture(command_buffer, storage, mem, descriptor, view_descriptor,
                      usage);

    /*
    // Create a new entry and merge it with others
    auto new_range = range;
    std::vector<TextureBase*> removed_textures;
    for (auto it = sparse_tex.cache.begin(); it != sparse_tex.cache.end();) {
        const auto& group = (*it).second;
        const auto crnt_range = group.base->GetDescriptor().GetRange();
        if (crnt_range.Intersects(range)) {
            // If the texture pointer difference is a multiple of the layer
            // size, merge the ranges
            const auto diff =
                (new_range.GetBegin() > crnt_range.GetBegin()
                     ? new_range.GetBegin() - crnt_range.GetBegin()
                     : crnt_range.GetBegin() - new_range.GetBegin());
            if (diff % layer_size == 0) {
                new_range = new_range.Union(crnt_range);
                removed_textures.push_back(group.base);
                // TODO: queue for deletion
                it = sparse_tex.cache.Remove(it);
            } else {
                LOG_WARN(Gpu, "Merging {:#x} with {:#x}", new_range,
                         crnt_range);
                LOG_WARN(
                    Gpu,
                    "[TEX 1] Ptr: {:#x}, format: {}, width: {}, height: {}, "
                    "depth: {}, stride: {:#x}",
                    descriptor.ptr, descriptor.format, descriptor.width,
                    descriptor.height, descriptor.depth, descriptor.stride);
                LOG_WARN(
                    Gpu,
                    "[TEX 2] Ptr: {:#x}, format: {}, width: {}, height: {}, "
                    "depth: {}, stride: {:#x}",
                    group.base->GetDescriptor().ptr,
                    group.base->GetDescriptor().format,
                    group.base->GetDescriptor().width,
                    group.base->GetDescriptor().height,
                    group.base->GetDescriptor().depth,
                    group.base->GetDescriptor().stride);
                ++it;
            }
        } else {
            ++it;
        }
    }

    // Create new group
    auto new_descriptor = descriptor;
    new_descriptor.ptr = new_range.GetBegin();
    ASSERT_ALIGNMENT_DEBUG(new_range.GetSize(), layer_size, Gpu,
                           "merged range");
    new_descriptor.depth = static_cast<u32>(new_range.GetSize() / layer_size);
    auto& group = sparse_tex.cache.Add(new_descriptor.ptr);
    auto new_tex = GetTexture(group, mem.info, new_descriptor, usage);

    LOG_INFO(Gpu, "ADDED GROUP {:#x} AT {:#x} TO SPARSE TEX {:#x}",
             (u64)(&group), new_descriptor.ptr, (u64)(&sparse_tex));

    // Copy the old textures to the new one
    for (const auto tex : removed_textures) {
        const auto offset =
            static_cast<u32>(tex->GetDescriptor().ptr - new_range.GetBegin());
        ASSERT_ALIGNMENT_DEBUG(offset, layer_size, Gpu,
                               "removed texture offset");
        // TODO: make sure the formats match
        new_tex->CopyFrom(
            tex, 0, uint3({0, 0, 0}), offset / layer_size, uint3({0, 0, 0}),
            usize3({descriptor.width, descriptor.height, descriptor.depth}));
    }

    // TODO: return a view
    return new_tex;
    */
}

ITextureView* TextureCache::GetTexture(
    ICommandBuffer* command_buffer, TextureStorage& storage, TextureMem& mem,
    const TextureDescriptor& descriptor,
    const TextureViewDescriptor& view_descriptor, TextureUsage usage) {
    if (!storage.base) {
        storage.base = RENDERER_INSTANCE.CreateTexture(descriptor);
        DecodeTexture(command_buffer, storage);
    }

    return GetTextureView(command_buffer, storage, mem, view_descriptor, usage);
}

ITextureView* TextureCache::GetTextureView(
    ICommandBuffer* command_buffer, TextureStorage& storage, TextureMem& mem,
    const TextureViewDescriptor& view_descriptor, TextureUsage usage) {
    Update(command_buffer, storage, mem, usage);

    auto view_opt = storage.view_cache.Find(view_descriptor.GetHash());
    if (view_opt.has_value())
        return **view_opt;

    auto view = storage.base->CreateView(view_descriptor);
    storage.view_cache.Add(view_descriptor.GetHash(), view);
    return view;
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
        for (auto& [key, sparse_tex] : mem.cache) {
            // TODO: skip this sparse texture

            for (auto& [key, other_group] : sparse_tex.cache) {
                const auto other_base = other_group.base;
                const auto& other_descriptor = other_base->GetDescriptor();
                const auto other_range = other_descriptor.GetRange();

                // Check if the textures can actually be copied
                if (other_descriptor.width != descriptor.width ||
                    other_descriptor.height != descriptor.height)
                    continue;

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
                        base->CopyFrom(
                            command_buffer, other_base, uint3({0, 0, 0}), 0,
                            src_layer, uint3({0, 0, 0}), 0, dst_layer,
                            usize3({descriptor.width, descriptor.height, 1}),
                            std::min(descriptor.level_count,
                                     other_descriptor.level_count),
                            layer_count);
                    } else if (descriptor.type == TextureType::_3D &&
                               other_descriptor.type ==
                                   TextureType::_3D) { // Both 3D
                        const auto slice_size =
                            descriptor.height *
                            align(get_texture_format_stride(descriptor.format,
                                                            descriptor.width),
                                  64u); // TODO: calculate properly

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
                                       usize3({descriptor.width,
                                               descriptor.height, z_count}),
                                       std::min(descriptor.level_count,
                                                other_descriptor.level_count),
                                       1);
                    } else if (descriptor.type == TextureType::_3D &&
                               other_descriptor.type ==
                                   TextureType::_2D) { // HACK: special case
                        const auto slice_size =
                            descriptor.height *
                            align(get_texture_format_stride(descriptor.format,
                                                            descriptor.width),
                                  64u); // TODO: calculate properly

                        // Z
                        const auto dst_z =
                            static_cast<u32>(dst_offset / slice_size);

                        // Copy
                        // TODO: make sure the formats match
                        base->CopyFrom(
                            command_buffer, other_base, uint3({0, 0, 0}), 0, 0,
                            uint3({0, 0, dst_z}), 0, 0,
                            usize3({descriptor.width, descriptor.height, 1}),
                            std::min(descriptor.level_count,
                                     other_descriptor.level_count),
                            1);
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
    u64 mem_range = descriptor.GetSize();
    u64 mem_step = std::max(mem_range / SAMPLE_COUNT, 1ull);

    HashCode hash;
    for (u64 offset = 0; offset < mem_range; offset += mem_step)
        hash.Add(*reinterpret_cast<u64*>(descriptor.ptr + offset));

    return hash.ToHashCode();
}

void TextureCache::DecodeTexture(ICommandBuffer* command_buffer,
                                 TextureStorage& storage) {
    const auto& descriptor = storage.base->GetDescriptor();

    // Align the height to 16 bytes (TODO: why 16?)
    auto tmp_buffer =
        RENDERER_INSTANCE.AllocateTemporaryBuffer(descriptor.GetSize());

    u8* in_data = reinterpret_cast<u8*>(descriptor.ptr);
    u8* out_data = reinterpret_cast<u8*>(tmp_buffer->GetPtr());
    if (descriptor.is_linear) {
        std::memcpy(out_data, in_data, descriptor.GetSize());
    } else {
        // HACK
        decode_generic_16bx2(align(get_texture_format_stride(descriptor.format,
                                                             descriptor.width),
                                   64u),
                             descriptor.layer_count * descriptor.depth *
                                 descriptor.height,
                             descriptor.block_height_log2, in_data, out_data);
    }

    storage.base->CopyFrom(command_buffer, tmp_buffer);
    RENDERER_INSTANCE.FreeTemporaryBuffer(tmp_buffer);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
