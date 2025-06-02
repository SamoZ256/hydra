#include "core/hw/tegra_x1/gpu/renderer/texture_decoder.hpp"

#include "core/hw/tegra_x1/gpu/memory_util.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

TextureDecoder::TextureDecoder() {}
TextureDecoder::~TextureDecoder() {}

void TextureDecoder::Decode(const TextureDescriptor& descriptor,
                            u8*& out_data) {
    u8* in_data = reinterpret_cast<u8*>(descriptor.ptr);

    switch (descriptor.kind) {
    case NvKind::Pitch:
        // TODO: correct?
        out_data = in_data;
        break;
    case NvKind::Generic_16BX2:
        decode_generic_16bx2(descriptor.stride, descriptor.height,
                             descriptor.block_height_log2, in_data, out_data);
        break;
    default:
        LOG_ERROR(GPU, "Unimplemented texture kind {}", descriptor.kind);
        break;
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
