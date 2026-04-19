#include "core/hw/tegra_x1/gpu/renderer/metal/texture_view.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/texture.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

TextureView::TextureView(Texture* base, const TextureViewDescriptor& descriptor)
    : ITextureView(base, descriptor) {
    const auto& pixel_format_info = to_mtl_pixel_format_info(descriptor.format);

    // Swizzle
    // TODO: remove component indices
    MTL::TextureSwizzle swizzle_components[] = {
        to_mtl_swizzle(descriptor.swizzle_channels.r),
        to_mtl_swizzle(descriptor.swizzle_channels.g),
        to_mtl_swizzle(descriptor.swizzle_channels.b),
        to_mtl_swizzle(descriptor.swizzle_channels.a)};
    MTL::TextureSwizzleChannels swizzle_channels_mtl(
        swizzle_components[pixel_format_info.component_indices[0]],
        swizzle_components[pixel_format_info.component_indices[1]],
        swizzle_components[pixel_format_info.component_indices[2]],
        swizzle_components[pixel_format_info.component_indices[3]]);

    // TODO: levels
    texture = base->GetTexture()->newTextureView(
        to_mtl_pixel_format(descriptor.format),
        ToMtlTextureType(descriptor.type),
        NS::Range(/*descriptor.levels.GetBegin()*/ 0,
                  /*descriptor.levels.GetSize()*/ 1),
        NS::Range(descriptor.layers.GetBegin(), descriptor.layers.GetSize()),
        swizzle_channels_mtl);
}

TextureView::~TextureView() { texture->release(); }

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
