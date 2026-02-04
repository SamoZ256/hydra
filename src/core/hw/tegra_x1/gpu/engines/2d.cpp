#include "core/hw/tegra_x1/gpu/engines/2d.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_base.hpp"

namespace hydra::hw::tegra_x1::gpu::engines {

DEFINE_METHOD_TABLE(TwoD, 0x237, 1, Copy, u32)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

void TwoD::Copy(const u32 index, const u32 pixels_from_memory_src_y0_int) {
    auto& pixels = regs.pixels_from_memory;
    pixels.src_y0.integer = pixels_from_memory_src_y0_int;

    // TODO: can these also not be textures?
    auto src = GetTexture(regs.src, renderer::TextureUsage::Read);
    auto dst = GetTexture(regs.dst, renderer::TextureUsage::Write);

    const auto dudx = static_cast<f64>(pixels.dudx);
    const auto dvdy = static_cast<f64>(pixels.dvdy);
    // TODO: why are these params messed up?
    const auto src_x0 = static_cast<f64>(pixels.src_x0);
    const auto src_y0 = static_cast<f64>(pixels.src_y0);

    const auto src_width = static_cast<u32>(pixels.dst_width * dudx);
    const auto src_height = static_cast<u32>(pixels.dst_height * dvdy);

    dst->BlitFrom(tls_crnt_command_buffer, src,
                  {static_cast<f32>(src_x0), static_cast<f32>(src_y0),
                   static_cast<f32>(regs.src.layer)},
                  {src_width, src_height, 1},
                  {static_cast<f32>(pixels.dst_x0),
                   static_cast<f32>(pixels.dst_y0),
                   static_cast<f32>(regs.dst.layer)},
                  {pixels.dst_width, pixels.dst_height, 1});
}

#pragma GCC diagnostic pop

renderer::TextureBase* TwoD::GetTexture(const Texture2DInfo& info,
                                        renderer::TextureUsage usage) {
    const renderer::TextureDescriptor descriptor(
        tls_crnt_gmmu->UnmapAddr(info.addr), renderer::TextureType::_2D,
        renderer::to_texture_format(info.format),
        NvKind::Pitch, // TODO: correct?
        u32(info.width), u32(info.height), 1,
        0, // HACK
           /*u32(info.stride)*/
        renderer::get_texture_format_stride(
            renderer::to_texture_format(info.format), info.width) // HACK
    );

    return RENDERER_INSTANCE.GetTextureCache().Find(tls_crnt_command_buffer,
                                                    descriptor, usage);
}

} // namespace hydra::hw::tegra_x1::gpu::engines
