#include "hw/tegra_x1/gpu/renderer/pipeline_cache.hpp"

#include "hw/tegra_x1/gpu/gpu.hpp"
#include "hw/tegra_x1/gpu/renderer/pipeline_base.hpp"

// HACK
template <typename T> T rotl(T v, u64 shift) {
    return (v << shift) | (v >> (32 - shift));
}

namespace Hydra::HW::TegraX1::GPU::Renderer {

PipelineBase* PipelineCache::Create(const PipelineDescriptor& descriptor) {
    return RENDERER->CreatePipeline(descriptor);
}

u64 PipelineCache::Hash(const PipelineDescriptor& descriptor) {
    // TODO: implement
    return 0;
}

void PipelineCache::DestroyElement(PipelineBase* pipeline) { delete pipeline; }

} // namespace Hydra::HW::TegraX1::GPU::Renderer
