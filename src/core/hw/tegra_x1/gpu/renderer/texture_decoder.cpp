#include "core/hw/tegra_x1/gpu/renderer/texture_decoder.hpp"

#include "core/hw/tegra_x1/gpu/memory_util.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

TextureDecoder::TextureDecoder() {}
TextureDecoder::~TextureDecoder() {}

void TextureDecoder::Decode(const TextureDescriptor& descriptor, u8* out_data) {
    u8* in_data = reinterpret_cast<u8*>(descriptor.ptr);

    // TODO: correct?
    switch (descriptor.kind) {
    case NvKind::Pitch:
    case NvKind::PitchNoSwizzle:
        std::memcpy(out_data, in_data,
                    descriptor.depth * descriptor.height * descriptor.stride);
        break;
    default:
        decode_generic_16bx2(descriptor.stride,
                             descriptor.depth * descriptor.height,
                             descriptor.block_height_log2, in_data, out_data);
        break;
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
