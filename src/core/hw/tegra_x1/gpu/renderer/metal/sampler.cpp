#include "core/hw/tegra_x1/gpu/renderer/metal/sampler.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

Sampler::Sampler(const SamplerDescriptor& descriptor)
    : SamplerBase(descriptor) {
    MTL::SamplerDescriptor* desc = MTL::SamplerDescriptor::alloc()->init();
    desc->setMinFilter(to_mtl_sampler_min_mag_filter(descriptor.min_filter));
    desc->setMagFilter(to_mtl_sampler_min_mag_filter(descriptor.mag_filter));
    desc->setMipFilter(to_mtl_sampler_mip_filter(descriptor.mip_filter));
    desc->setRAddressMode(
        to_mtl_sampler_address_mode(descriptor.address_mode_r));
    desc->setSAddressMode(
        to_mtl_sampler_address_mode(descriptor.address_mode_s));
    desc->setTAddressMode(
        to_mtl_sampler_address_mode(descriptor.address_mode_t));

    mtl_sampler = METAL_RENDERER_INSTANCE.GetDevice()->newSamplerState(desc);
}

Sampler::~Sampler() { mtl_sampler->release(); }

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
