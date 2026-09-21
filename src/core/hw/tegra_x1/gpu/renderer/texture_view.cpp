#include "core/hw/tegra_x1/gpu/renderer/texture_view.hpp"

#include "core/hw/tegra_x1/gpu/renderer/texture.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

void ITextureView::copyFrom(ICommandBuffer* command_buffer,
                            const BufferBase* src,
                            const ztd::Range<u32> dst_levels,
                            const ztd::Range<u32> dst_layers) {
    base->copyFrom(command_buffer, src,
                   ztd::Range<u32>::fromSize(descriptor.levels.getBegin() +
                                                 dst_levels.getBegin(),
                                             dst_levels.getSize()),
                   ztd::Range<u32>::fromSize(descriptor.layers.getBegin() +
                                                 dst_layers.getBegin(),
                                             dst_layers.getSize()));
}

void ITextureView::copyFrom(ICommandBuffer* command_buffer,
                            const BufferBase* src) {
    copyFrom(command_buffer, src,
             ztd::Range<u32>(0, descriptor.levels.getSize()),
             ztd::Range<u32>(0, descriptor.layers.getSize()));
}

void ITextureView::copyFrom(ICommandBuffer* command_buffer,
                            const ITextureView* src, const u32 src_level,
                            const u32 src_layer, const u32 dst_level,
                            const u32 dst_layer, const u32 level_count,
                            const u32 layer_count) {
    base->copyFrom(command_buffer, src->getBase(),
                   src->getDescriptor().levels.getBegin() + src_level,
                   src->getDescriptor().layers.getBegin() + src_layer,
                   descriptor.levels.getBegin() + dst_level,
                   descriptor.layers.getBegin() + dst_layer, level_count,
                   layer_count);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
