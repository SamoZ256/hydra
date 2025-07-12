#include "core/hw/tegra_x1/gpu/renderer/texture_cache.hpp"

#include "common/time.hpp"
#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_base.hpp"

// HACK
bool g_uses_gpu = false;

namespace hydra::hw::tegra_x1::gpu::renderer {

TextureCache::~TextureCache() {
    // TODO: uncomment
    /*
    for (auto& [key, value] : texture_mem_map) {
        for (auto& [key, value] : value.cache) {
            delete value.base;
            for (auto& [key, value] : value.view_cache)
                delete value;
        }
    }
    */
}

TextureBase* TextureCache::GetTextureView(const TextureDescriptor& descriptor) {
    auto& texture_mem = texture_mem_map[descriptor.ptr];
    auto& tex = texture_mem.cache.Find(GetTextureHash(descriptor));
    if (!tex.base)
        tex.base = Create(descriptor);

    Update(tex, texture_mem.last_modified);

    // If the formats match and swizzle is the default swizzle, return base
    if (descriptor.format == tex.base->GetDescriptor().format &&
        descriptor.swizzle_channels ==
            tex.base->GetDescriptor().swizzle_channels) {
        return tex.base;
    }

    // Otherwise, get a texture view
    auto view_desc = TextureViewDescriptor{
        .format = descriptor.format,
        .swizzle_channels = descriptor.swizzle_channels,
    };
    auto& view = tex.view_cache.Find(view_desc.GetHash());
    if (view)
        return view;

    view = tex.base->CreateView(view_desc);

    return view;
}

void TextureCache::NotifyGuestModifiedData(const range<uptr> mem_range) {
    // TODO: mark all overlapping memories as modified
    auto it = texture_mem_map.find(mem_range.begin);
    if (it == texture_mem_map.end())
        return;

    it->second.MarkModified();
}

TextureBase* TextureCache::Create(const TextureDescriptor& descriptor) {
    auto desc = descriptor;
    desc.swizzle_channels =
        get_texture_format_default_swizzle_channels(desc.format);
    auto texture = RENDERER_INSTANCE->CreateTexture(desc);
    DecodeTexture(texture);

    return texture;
}

void TextureCache::Update(Tex& tex, const ModifyInfo& mem_last_modified) {
    bool force_upload = false;

    // HACK: if homebrew
    if (/*KERNEL_INSTANCE.GetTitleID() == 0xffffffffffffffff && */
        !g_uses_gpu) {
        force_upload = true;
        ONCE(LOG_WARN(
            Gpu, "Homebrew framebuffer API detected, forcing texture upload"));
    }

    /*
    // HACK: if Sonic Mania
    if (KERNEL_INSTANCE.GetTitleID() == 0x01009aa000faa000 &&
        tex.base->GetDescriptor().width == 512 &&
        tex.base->GetDescriptor().height == 256) {
        force_upload = true;
        ONCE(LOG_WARN(Gpu, "Sonic Mania detected, forcing texture upload"));
    }

    // HACK: if flog
    if (KERNEL_INSTANCE.GetTitleID() == 0x01008bb00013c000 &&
        tex.base->GetDescriptor().width == 3712 &&
        tex.base->GetDescriptor().height == 2160) {
        force_upload = true;
        ONCE(LOG_WARN(Gpu, "Flog detected, forcing texture upload"));
    }
    */

    if (tex.upload_timestamp < mem_last_modified.timestamp || force_upload) {
        DecodeTexture(tex.base);
        tex.upload_timestamp = get_absolute_time();
    }
}

u64 TextureCache::GetTextureHash(const TextureDescriptor& descriptor) {
    u64 hash = 0;
    hash += descriptor.ptr;
    hash = std::rotl(hash, 7);
    hash += descriptor.width;
    hash = std::rotl(hash, 11);
    hash += descriptor.height;
    hash = std::rotl(hash, 11);

    // View compatbility hash
    // TODO: get format info from the renderer instead
    hash += is_texture_format_compressed(descriptor.format);
    hash = std::rotl(hash, 1);
    hash += is_texture_format_depth_or_stencil(descriptor.format);
    hash = std::rotl(hash, 1);

    return hash;
}

void TextureCache::DecodeTexture(TextureBase* texture) {
    scratch_buffer.resize(texture->GetDescriptor().stride *
                          texture->GetDescriptor().height);
    auto data = scratch_buffer.data();
    texture_decoder.Decode(texture->GetDescriptor(), data);

    auto tmp_buffer = RENDERER_INSTANCE->AllocateTemporaryBuffer(
        texture->GetDescriptor().stride * texture->GetDescriptor().height);
    tmp_buffer->CopyFrom(reinterpret_cast<uptr>(data));
    texture->CopyFrom(tmp_buffer);
    RENDERER_INSTANCE->FreeTemporaryBuffer(tmp_buffer);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
