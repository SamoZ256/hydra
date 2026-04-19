#include "core/hw/tegra_x1/gpu/renderer/texture_view.hpp"

#include "core/hw/tegra_x1/gpu/renderer/texture.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

void ITextureView::CopyFrom(ICommandBuffer* command_buffer,
                            const BufferBase* src, const uint3 dst_origin,
                            const usize3 dst_size, const Range<u32> dst_levels,
                            const Range<u32> dst_layers) {
    base->CopyFrom(command_buffer, src, dst_origin, dst_size,
                   Range<u32>::FromSize(descriptor.levels.GetBegin() +
                                            dst_levels.GetBegin(),
                                        dst_levels.GetSize()),
                   Range<u32>::FromSize(descriptor.layers.GetBegin() +
                                            dst_layers.GetBegin(),
                                        dst_layers.GetSize()));
}

void ITextureView::CopyFrom(ICommandBuffer* command_buffer,
                            const BufferBase* src) {
    CopyFrom(command_buffer, src, uint3({0, 0, 0}),
             usize3({base->GetDescriptor().width, base->GetDescriptor().height,
                     base->GetDescriptor().depth}),
             Range<u32>(0, descriptor.levels.GetSize()),
             Range<u32>(0, descriptor.layers.GetSize()));
}

void ITextureView::CopyFrom(ICommandBuffer* command_buffer,
                            const ITextureView* src, const uint3 src_origin,
                            const u32 src_level, const u32 src_layer,
                            const uint3 dst_origin, const u32 dst_level,
                            const u32 dst_layer, const usize3 size,
                            const u32 level_count, const u32 layer_count) {
    base->CopyFrom(command_buffer, src->GetBase(), src_origin,
                   src->GetDescriptor().levels.GetBegin() + src_level,
                   src->GetDescriptor().layers.GetBegin() + src_layer,
                   dst_origin, descriptor.levels.GetBegin() + dst_level,
                   descriptor.layers.GetBegin() + dst_layer, size, level_count,
                   layer_count);
}

void ITextureView::BlitFrom(ICommandBuffer* command_buffer,
                            const ITextureView* src, const float3 src_origin,
                            const usize3 src_size, const u32 src_level,
                            const u32 src_layer, const float3 dst_origin,
                            const usize3 dst_size, const u32 dst_level,
                            const u32 dst_layer, const u32 level_count,
                            const u32 layer_count) {
    base->BlitFrom(
        command_buffer, src->GetBase(), src_origin, src_size,
        src->GetDescriptor().levels.GetBegin() + src_level,
        src->GetDescriptor().layers.GetBegin() + src_layer, dst_origin,
        dst_size, descriptor.levels.GetBegin() + dst_level,
        descriptor.layers.GetBegin() + dst_layer, level_count, layer_count);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
