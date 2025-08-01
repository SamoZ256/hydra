#include "core/hw/tegra_x1/gpu/engines/2d.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_base.hpp"

namespace hydra::hw::tegra_x1::gpu::engines {

namespace {

constexpr u32 DIFF_FRACT_BITS = 15;
constexpr u32 SRC_FRACT_BITS = 4;

// TODO: use f64?
// TODO: correct?
#define GET_FLOAT(x, fract_bits)                                               \
    (static_cast<f32>(x.integer) +                                             \
     (static_cast<f32>(x.fractional >> fract_bits) /                           \
      static_cast<f32>(1 << fract_bits)))
#define GET_DIFF_FLOAT(x) GET_FLOAT(x, DIFF_FRACT_BITS)
#define GET_SRC_FLOAT(x) GET_FLOAT(x, SRC_FRACT_BITS)

} // namespace

DEFINE_METHOD_TABLE(TwoD, 0x237, 1, Copy, u32)

void TwoD::Copy(const u32 index, const u32 pixels_from_memory_src_y0_int) {
    auto& pixels = regs.pixels_from_memory;
    pixels.src_y0.integer = pixels_from_memory_src_y0_int;

    // TODO: can these also not be textures?
    auto src = GetTexture(regs.src);
    auto dst = GetTexture(regs.dst);

    f32 dudx = GET_DIFF_FLOAT(pixels.dudx);
    f32 dvdy = GET_DIFF_FLOAT(pixels.dvdy);
    // TODO: why are these params messed up?
    f32 src_x0 = GET_SRC_FLOAT(pixels.src_x0);
    f32 src_y0 = GET_SRC_FLOAT(pixels.src_y0);

    usize src_width = pixels.dst_width * dudx;
    usize src_height = pixels.dst_height * dvdy;

    // TODO: src origin
    dst->BlitFrom(
        src, regs.src.layer, {0, 0, 0}, {src_width, src_height, 1},
        regs.dst.layer,
        {static_cast<f32>(pixels.dst_x0), static_cast<f32>(pixels.dst_y0), 0},
        {pixels.dst_width, pixels.dst_height, 1});
}

renderer::TextureBase* TwoD::GetTexture(const Texture2DInfo& info) {
    const renderer::TextureDescriptor descriptor(
        UNMAP_ADDR(info.addr), renderer::to_texture_format(info.format),
        NvKind::Pitch, // TODO: correct?
        static_cast<usize>(info.width), static_cast<usize>(info.height),
        0, // HACK
           /*static_cast<usize>(info.stride)*/
        renderer::get_texture_format_stride(
            renderer::to_texture_format(info.format), info.width) // HACK
    );

    return RENDERER_INSTANCE.GetTextureCache().GetTextureView(descriptor);
}

} // namespace hydra::hw::tegra_x1::gpu::engines
