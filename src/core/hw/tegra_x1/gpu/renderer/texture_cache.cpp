#include "core/hw/tegra_x1/gpu/renderer/texture_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_base.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer {

TextureBase* TextureCache::GetTextureView(const TextureDescriptor& descriptor) {
    auto& tex = Find(descriptor);

    // If the formats match and swizzle is the default swizzle, return base
    if (descriptor.format == tex.base->GetDescriptor().format &&
        descriptor.swizzle_channels ==
            get_texture_format_default_swizzle_channels(descriptor.format)) {
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

Tex TextureCache::Create(const TextureDescriptor& descriptor) {
    auto texture = RENDERER->CreateTexture(descriptor);
    DecodeTexture(texture);

    return {texture};
}

void TextureCache::Update(Tex& texture) {
    // TODO: if data changed
    if (texture.base->GetDescriptor().kind == NvKind::Generic_16BX2) // HACK
        DecodeTexture(texture.base);
}

u64 TextureCache::Hash(const TextureDescriptor& descriptor) {
    u64 hash = 0;
    hash += descriptor.ptr;
    hash = rotl(hash, 7);
    hash += descriptor.width;
    hash = rotl(hash, 11);
    hash += descriptor.height;

    return hash;
}

void TextureCache::DestroyElement(Tex& texture) {
    // TODO: delete all views
    delete texture.base;
}

void TextureCache::DecodeTexture(TextureBase* texture) {
    u8* out_data = scratch_buffer + sizeof(scratch_buffer) / 2;
    texture_decoder.Decode(texture->GetDescriptor(), scratch_buffer, out_data);

    texture->CopyFrom(out_data);
}

} // namespace Hydra::HW::TegraX1::GPU::Renderer
