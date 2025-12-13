#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/exit.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

void EmitKil(DecoderContext& context, InstKil inst) {
    const auto conditional =
        HandlePredCond(context.builder, inst.pred, inst.pred_inv);

    context.builder.OpDiscard();

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
