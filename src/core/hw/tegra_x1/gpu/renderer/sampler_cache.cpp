#include "core/hw/tegra_x1/gpu/renderer/sampler_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/sampler_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

SamplerBase* SamplerCache::Create(const SamplerDescriptor& descriptor) {
    auto sampler = renderer.CreateSampler(descriptor);
    return sampler;
}

u32 SamplerCache::Hash(const SamplerDescriptor& descriptor) {
    ztd::hash::XxHash32 hash;
    hash.add(descriptor.min_filter);
    hash.add(descriptor.mag_filter);
    hash.add(descriptor.mip_filter);
    hash.add(descriptor.address_mode_s);
    hash.add(descriptor.address_mode_t);
    hash.add(descriptor.address_mode_r);
    hash.add(descriptor.depth_compare_op);
    hash.add(descriptor.border_color_u);

    return hash.toHashCode();
}

void SamplerCache::DestroyElement(SamplerBase* sampler) { delete sampler; }

} // namespace hydra::hw::tegra_x1::gpu::renderer
