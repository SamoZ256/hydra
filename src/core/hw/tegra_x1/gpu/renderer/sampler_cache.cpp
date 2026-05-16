#include "core/hw/tegra_x1/gpu/renderer/sampler_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/sampler_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

SamplerBase* SamplerCache::Create(const SamplerDescriptor& descriptor) {
    auto sampler = renderer.CreateSampler(descriptor);
    return sampler;
}

u32 SamplerCache::Hash(const SamplerDescriptor& descriptor) {
    HashCode hash;
    hash.Add(descriptor.min_filter);
    hash.Add(descriptor.mag_filter);
    hash.Add(descriptor.mip_filter);
    hash.Add(descriptor.address_mode_s);
    hash.Add(descriptor.address_mode_t);
    hash.Add(descriptor.address_mode_r);
    hash.Add(descriptor.depth_compare_op);
    hash.Add(descriptor.border_color_u);

    return hash.ToHashCode();
}

void SamplerCache::DestroyElement(SamplerBase* sampler) { delete sampler; }

} // namespace hydra::hw::tegra_x1::gpu::renderer
