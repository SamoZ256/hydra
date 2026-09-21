#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/exit.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

void emitKil(DecoderContext& context, InstKil inst) {
    const auto conditional =
        handlePredCond(context.builder, inst.pred, inst.pred_inv);

    context.builder.opDiscard();

    if (conditional)
        context.builder.opEndIf();
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
