#include "core/hw/tegra_x1/gpu/renderer/texture_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

TextureCache::~TextureCache() {
    for (auto& [key, mem] : entries) {
        for (const auto& [key, sparse_tex] : mem.cache) {
            for (const auto& [key, group] : sparse_tex.cache) {
                delete group.base;
                for (const auto& [key, view] : group.view_cache)
                    delete view;
            }
        }
    }
}

TextureBase* TextureCache::Find(ICommandBuffer* command_buffer,
                                const TextureDescriptor& descriptor,
                                TextureUsage usage) {
    const auto range = descriptor.GetRange();

    // Check for containing interval
    auto it = entries.upper_bound(range.GetBegin());
    if (it != entries.begin()) {
        auto prev = std::prev(it);
        auto& prev_mem = prev->second;
        if (prev_mem.range.GetEnd() >= range.GetEnd()) {
            // Fully contained
            return AddToMemory(command_buffer, prev_mem, descriptor, usage);
        }
    }

    // Insert and merge
    TextureMem mem{.range = range};

    it = entries.lower_bound(range.GetBegin());

    // Merge with previous if overlapping
    if (it != entries.begin()) {
        auto prev = std::prev(it);
        const auto& prev_mem = prev->second;
        if (prev_mem.range.GetEnd() > mem.range.GetBegin()) {
            mem = MergeMemories(mem, prev_mem);
            it = entries.erase(prev);
        }
    }

    // Merge with following entries
    while (it != entries.end() && it->first < mem.range.GetEnd()) {
        const auto& crnt_mem = it->second;
        mem = MergeMemories(mem, crnt_mem);
        it = entries.erase(it);
    }

    // Insert merged interval
    auto inserted = entries.emplace(mem.range.GetBegin(), mem);
    return AddToMemory(command_buffer, inserted.first->second, descriptor,
                       usage);
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

TextureMem TextureCache::MergeMemories(const TextureMem& a,
                                       const TextureMem& b) {
    TextureMem res;
    res.range = a.range.Union(b.range);
    res.info = {
        .modified_timestamp =
            std::max(a.info.modified_timestamp, b.info.modified_timestamp),
        .read_timestamp =
            std::max(a.info.read_timestamp, b.info.read_timestamp),
        .written_timestamp =
            std::max(a.info.written_timestamp, b.info.written_timestamp),
    };

    // HACK
    for (const auto& [key, tex] : const_cast<TextureMem&>(a).cache)
        res.cache.Add(key, tex);

    for (const auto& [key, tex] : const_cast<TextureMem&>(b).cache)
        res.cache.Add(key, tex);

    return res;
}

TextureBase* TextureCache::AddToMemory(ICommandBuffer* command_buffer,
                                       TextureMem& mem,
                                       const TextureDescriptor& descriptor,
                                       TextureUsage usage) {
    const auto range = descriptor.GetRange();
    const auto layer_size = descriptor.GetLayerSizeInBytes();

    // Check if it is a new entry
    auto sparse_tex_opt = mem.cache.Find(descriptor.GetHash());
    if (!sparse_tex_opt.has_value()) {
        auto& sparse_tex = mem.cache.Add(descriptor.GetHash());
        auto& group = sparse_tex.cache.Add(descriptor.ptr);
        return GetTexture(command_buffer, group, mem, descriptor, usage);
    }

    auto& sparse_tex = **sparse_tex_opt;

    // Check if it is just a view with smaller layer count
    auto group_opt = sparse_tex.cache.Find(descriptor.ptr);
    if (group_opt) {
        auto& group = **group_opt;
        if (group.base->GetDescriptor().GetRange().Contains(range))
            return GetTexture(command_buffer, group, mem, descriptor, usage);
        else
            sparse_tex.cache.Remove(descriptor.ptr);
    }

    // Check if it is a proper layer view
    for (const auto& [key, group] : sparse_tex.cache) {
        if (group.base->GetDescriptor().GetRange().Contains(range)) {
            const auto offset = static_cast<u32>(
                range.GetBegin() - group.base->GetDescriptor().ptr);
            ASSERT_ALIGNMENT_DEBUG(offset, layer_size, Gpu,
                                   "texture view offset");
            const auto layers =
                Range<u32>::FromSize(offset / layer_size, descriptor.depth);
            return GetTextureView(
                group, TextureViewDescriptor(descriptor.format,
                                             descriptor.swizzle_channels,
                                             Range<u32>(0, 1), layers));
        }
    }

    // HACK: create a new texture
    auto& group = sparse_tex.cache.Add(descriptor.ptr);
    return GetTexture(command_buffer, group, mem, descriptor, usage);

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

TextureBase* TextureCache::GetTexture(ICommandBuffer* command_buffer,
                                      TextureGroup& group, TextureMem& mem,
                                      const TextureDescriptor& descriptor,
                                      TextureUsage usage) {
    if (!group.base)
        Create(command_buffer, descriptor, group);

    Update(command_buffer, group, mem, usage);

    // If the formats match and swizzle is the default swizzle,
    // return base
    if (descriptor.format == group.base->GetDescriptor().format &&
        descriptor.swizzle_channels == SwizzleChannels()) {
        return group.base;
    }

    // Otherwise, get a texture view
    auto view_desc = TextureViewDescriptor(
        descriptor.format, descriptor.swizzle_channels, Range<u32>(0, 1),
        Range<u32>(0, descriptor.depth));
    return GetTextureView(group, view_desc);
}

TextureBase*
TextureCache::GetTextureView(TextureGroup& group,
                             const TextureViewDescriptor& descriptor) {
    auto view_opt = group.view_cache.Find(descriptor.GetHash());
    if (view_opt.has_value())
        return **view_opt;

    auto view = group.base->CreateView(descriptor);
    group.view_cache.Add(descriptor.GetHash(), view);
    return view;
}

void TextureCache::Create(ICommandBuffer* command_buffer,
                          const TextureDescriptor& descriptor,
                          TextureGroup& group) {
    auto desc = descriptor;
    desc.swizzle_channels =
        get_texture_format_default_swizzle_channels(desc.format);
    group.base = RENDERER_INSTANCE.CreateTexture(desc);
    DecodeTexture(command_buffer, group);
}

void TextureCache::Update(ICommandBuffer* command_buffer, TextureGroup& group,
                          TextureMem& mem, TextureUsage usage) {
    bool sync = false;
    if (group.update_timestamp < mem.info.modified_timestamp) {
        // If modified by the guest
        sync = true;
    } else if (group.update_timestamp < mem.info.written_timestamp) {
        // Other textures in this memory changed, let's copy them
        const auto base = group.base;
        const auto& descriptor = base->GetDescriptor();
        const auto range = descriptor.GetRange();
        const auto layer_size = descriptor.GetLayerSizeInBytes();
        for (const auto& [key, sparse_tex] : mem.cache) {
            // TODO: skip this sparse texture

            for (const auto& [key, other_group] : sparse_tex.cache) {
                const auto other_base = other_group.base;
                const auto& other_descriptor = other_base->GetDescriptor();
                const auto other_range = other_descriptor.GetRange();

                // Check if the textures can actually be copied
                if (other_descriptor.width != descriptor.width ||
                    other_descriptor.height != descriptor.height ||
                    other_descriptor.stride != descriptor.stride)
                    continue;

                if (range.Intersects(other_range)) {
                    const auto copy_range = range.ClampedTo(other_range);
                    const auto dst_offset =
                        copy_range.GetBegin() - range.GetBegin();

                    // Check if the textures are aligned properly
                    if (dst_offset % layer_size != 0x0)
                        continue;

                    // Now copy
                    const auto src_layer = static_cast<u32>(
                        (copy_range.GetBegin() - other_range.GetBegin()) /
                        layer_size);
                    const auto dst_layer =
                        static_cast<u32>(dst_offset / layer_size);
                    const auto layer_count =
                        static_cast<u32>(copy_range.GetSize() / layer_size);

                    // TODO: make sure the formats match
                    base->CopyFrom(command_buffer, other_base,
                                   uint3({0, 0, src_layer}),
                                   uint3({0, 0, dst_layer}),
                                   usize3({descriptor.width, descriptor.height,
                                           layer_count}));
                }
            }
        }

        group.MarkUpdated();
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
        DecodeTexture(command_buffer, group);

    if (usage == TextureUsage::Read)
        mem.info.MarkRead();
    else if (usage == TextureUsage::Write)
        mem.info.MarkWritten();

    if (usage == TextureUsage::Write || sync)
        group.MarkUpdated();
}

u32 TextureCache::GetDataHash(const TextureBase* texture) {
    constexpr u32 SAMPLE_COUNT = 37;

    const auto& descriptor = texture->GetDescriptor();
    u64 mem_range = descriptor.stride * descriptor.height;
    u64 mem_step = std::max(mem_range / SAMPLE_COUNT, 1ull);

    HashCode hash;
    for (u64 offset = 0; offset < mem_range; offset += mem_step)
        hash.Add(*reinterpret_cast<u64*>(descriptor.ptr + offset));

    return hash.ToHashCode();
}

void TextureCache::DecodeTexture(ICommandBuffer* command_buffer,
                                 TextureGroup& group) {
    const auto& descriptor = group.base->GetDescriptor();

    // Align the height to 16 bytes (TODO: why 16?)
    auto tmp_buffer = RENDERER_INSTANCE.AllocateTemporaryBuffer(
        descriptor.depth * align(descriptor.height, 16u) * descriptor.stride);
    texture_decoder.Decode(descriptor, (u8*)tmp_buffer->GetPtr());
    group.base->CopyFrom(command_buffer, tmp_buffer);
    RENDERER_INSTANCE.FreeTemporaryBuffer(tmp_buffer);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
