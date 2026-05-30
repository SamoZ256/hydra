#include "core/hw/tegra_x1/gpu/renderer/null/texture.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::null {

Texture::Texture(const TextureDescriptor& descriptor) : ITexture(descriptor) {}
Texture::~Texture() {}

ITextureView*
Texture::CreateView(const TextureViewDescriptor& view_descriptor) {
    return new TextureView(this, view_descriptor);
}

void Texture::CopyFrom([[maybe_unused]] ICommandBuffer* command_buffer,
                       [[maybe_unused]] const BufferBase* src,
                       [[maybe_unused]] const Range<u32> dst_levels,
                       [[maybe_unused]] const Range<u32> dst_layers) {}
void Texture::CopyFrom([[maybe_unused]] ICommandBuffer* command_buffer,
                       [[maybe_unused]] const ITexture* src,
                       [[maybe_unused]] const u32 src_level,
                       [[maybe_unused]] const u32 src_layer,
                       [[maybe_unused]] const u32 dst_level,
                       [[maybe_unused]] const u32 dst_layer,
                       [[maybe_unused]] const u32 level_count,
                       [[maybe_unused]] const u32 layer_count) {}
void Texture::CopyFrom([[maybe_unused]] ICommandBuffer* command_buffer,
                       [[maybe_unused]] const ITexture* src,
                       [[maybe_unused]] const uint3 src_origin,
                       [[maybe_unused]] const u32 src_level,
                       [[maybe_unused]] const u32 src_layer,
                       [[maybe_unused]] const uint3 dst_origin,
                       [[maybe_unused]] const u32 dst_level,
                       [[maybe_unused]] const u32 dst_layer,
                       [[maybe_unused]] const uint3 size,
                       [[maybe_unused]] const u32 layer_count) {}

TextureView::TextureView(Texture* base, const TextureViewDescriptor& descriptor)
    : ITextureView(base, descriptor) {}

} // namespace hydra::hw::tegra_x1::gpu::renderer::null
