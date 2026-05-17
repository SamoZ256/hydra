#include "core/hw/tegra_x1/gpu/renderer/metal/sampler.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

Sampler::Sampler(MTL::Device* device, const SamplerDescriptor& descriptor)
    : SamplerBase(descriptor) {
    MTL::SamplerDescriptor* desc = MTL::SamplerDescriptor::alloc()->init();
    desc->setMinFilter(to_mtl_sampler_min_mag_filter(descriptor.min_filter));
    desc->setMagFilter(to_mtl_sampler_min_mag_filter(descriptor.mag_filter));
    desc->setMipFilter(to_mtl_sampler_mip_filter(descriptor.mip_filter));
    desc->setSAddressMode(
        to_mtl_sampler_address_mode(descriptor.address_mode_s));
    desc->setTAddressMode(
        to_mtl_sampler_address_mode(descriptor.address_mode_t));
    desc->setRAddressMode(
        to_mtl_sampler_address_mode(descriptor.address_mode_r));
    if (descriptor.depth_compare_op != engines::CompareOp::Invalid)
        desc->setCompareFunction(
            to_mtl_compare_func(descriptor.depth_compare_op));

    // Border color
    const auto border_color = std::bit_cast<float4>(descriptor.border_color_u);
    MTL::SamplerBorderColor mtl_border_color;
    // TODO: emulate unsupported border colors
    if (border_color.w() == 1.0f) {
        if (border_color.x() == 1.0f || border_color.y() == 1.0f ||
            border_color.z() == 1.0f) {
            mtl_border_color = MTL::SamplerBorderColorOpaqueWhite;
        } else {
            if (border_color.x() != 0.0f || border_color.y() != 0.0f ||
                border_color.z() != 0.0f) {
                LOG_WARN(
                    MetalRenderer,
                    "Unsupported border color {}, defaulting to opaque black",
                    border_color);
            }
            mtl_border_color = MTL::SamplerBorderColorOpaqueBlack;
        }
    } else {
        if (border_color.x() != 0.0f || border_color.y() != 0.0f ||
            border_color.z() != 0.0f) {
            LOG_WARN(
                MetalRenderer,
                "Unsupported border color {}, defaulting to transparent black",
                border_color);
        }
        mtl_border_color = MTL::SamplerBorderColorTransparentBlack;
    }
    desc->setBorderColor(mtl_border_color);

    mtl_sampler = device->newSamplerState(desc);
}

Sampler::~Sampler() { mtl_sampler->release(); }

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
