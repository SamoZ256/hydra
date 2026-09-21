#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/warp.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

void emitShuffle(DecoderContext& context, pred_t pred, bool pred_inv,
                 ShuffleMode mode, reg_t dst, pred_t dst_pred, reg_t src_a,
                 ir::Value src_b, ir::Value src_c) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    const auto src_a_v = ir::Value::createRegister(src_a);

    // TODO: implement
    LOG_FUNC_WITH_ARGS_NOT_IMPLEMENTED(
        ShaderDecompiler, "mode: {}, src_a: {}, src_b: {}, src_c: {}", mode,
        src_a_v, src_b, src_c);
    // HACK
    const auto res = src_a_v;
    const auto pred_res = ir::Value::createConstantB(true);

    context.builder.opCopy(ir::Value::createRegister(dst), res);
    context.builder.opCopy(ir::Value::createPredicate(dst_pred), pred_res);

    if (conditional)
        context.builder.opEndIf();
}

} // namespace

void emitShfl(DecoderContext& context, InstShfl inst) {
    emitShuffle(context, inst.pred, inst.pred_inv, inst.mode, inst.dst,
                inst.dst_pred, inst.src_a,
                inst.b_is_imm ? ir::Value::createConstantU(inst.src_b_imm5)
                              : ir::Value::createRegister(inst.src_b_r),
                inst.c_is_imm ? ir::Value::createConstantU(inst.src_c_imm13)
                              : ir::Value::createRegister(inst.src_c_r));
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
