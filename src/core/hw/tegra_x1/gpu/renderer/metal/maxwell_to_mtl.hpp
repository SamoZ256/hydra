#pragma once

#include "core/hw/tegra_x1/gpu/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

struct PixelFormatInfo {
    MTL::PixelFormat pixel_format;
    bool has_stencil = false;
    // TODO: format decoder
};

const PixelFormatInfo& to_mtl_pixel_format_info(TextureFormat format);
inline MTL::PixelFormat to_mtl_pixel_format(TextureFormat format) {
    return to_mtl_pixel_format_info(format).pixel_format;
}

MTL::PrimitiveType
to_mtl_primitive_type(const engines::PrimitiveType primitive_type);
MTL::IndexType to_mtl_index_type(engines::IndexType index_type);

const MTL::VertexFormat to_mtl_vertex_format(engines::VertexAttribType type,
                                             engines::VertexAttribSize size,
                                             bool bgra);

MTL::CompareFunction
to_mtl_compare_func(engines::DepthTestFunc depth_test_func);

MTL::TextureSwizzle to_mtl_swizzle(const ImageSwizzle swizzle);

MTL::BlendOperation to_mtl_blend_operation(const BlendOperation blend_op);
MTL::BlendFactor to_mtl_blend_factor(const BlendFactor blend_factor);

MTL::SamplerMinMagFilter
to_mtl_sampler_min_mag_filter(const SamplerFilter filter);
MTL::SamplerMipFilter to_mtl_sampler_mip_filter(const SamplerMipFilter filter);
MTL::SamplerAddressMode
to_mtl_sampler_address_mode(const SamplerAddressMode address_mode);

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
