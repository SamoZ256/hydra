#include "core/hw/tegra_x1/gpu/renderer/metal/sampler.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

Sampler::Sampler(MTL::Device* device, const SamplerDescriptor& descriptor)
    : SamplerBase(descriptor) {
    MTL::SamplerDescriptor* desc = MTL::SamplerDescriptor::alloc()->init();
    desc->setMinFilter(toMtlSamplerMinMagFilter(descriptor.min_filter));
    desc->setMagFilter(toMtlSamplerMinMagFilter(descriptor.mag_filter));
    desc->setMipFilter(toMtlSamplerMipFilter(descriptor.mip_filter));
    desc->setSAddressMode(toMtlSamplerAddressMode(descriptor.address_mode_s));
    desc->setTAddressMode(toMtlSamplerAddressMode(descriptor.address_mode_t));
    desc->setRAddressMode(toMtlSamplerAddressMode(descriptor.address_mode_r));
    if (descriptor.depth_compare_op != engines::CompareOp::Invalid)
        desc->setCompareFunction(toMtlCompareFunc(descriptor.depth_compare_op));

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
