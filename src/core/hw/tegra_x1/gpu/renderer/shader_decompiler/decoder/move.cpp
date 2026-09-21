#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/move.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: mask
void emitMove(DecoderContext& context, pred_t pred, bool pred_inv, reg_t dst,
              ir::Value src) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    context.builder.opCopy(ir::Value::createRegister(dst), src);

    if (conditional)
        context.builder.opEndIf();
}

void emitSelect(DecoderContext& context, pred_t pred, bool pred_inv, reg_t dst,
                pred_t src_pred, bool src_pred_inv, reg_t src_a,
                ir::Value src_b) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    const auto src_pred_v = notIf(
        context.builder, ir::Value::createPredicate(src_pred), src_pred_inv);
    const auto res = context.builder.opSelect(
        src_pred_v, ir::Value::createRegister(src_a), src_b);
    context.builder.opCopy(ir::Value::createRegister(dst), res);

    if (conditional)
        context.builder.opEndIf();
}

} // namespace

void emitMovR(DecoderContext& context, InstMovR inst) {
    emitMove(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
             ir::Value::createRegister(inst.src));
}

void emitMovC(DecoderContext& context, InstMovC inst) {
    emitMove(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
             ir::Value::createConstMemory(CMem(
                 inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4))));
}

void emitMovI(DecoderContext& context, InstMovI inst) {
    emitMove(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
             ir::Value::createConstantU(
                 getIntImm20(inst.imm20_0, inst.imm20_19, false)));
}

void emitMov32I(DecoderContext& context, InstMov32I inst) {
    emitMove(context, inst.pred, inst.pred_inv, inst.dst,
             ir::Value::createConstantU(inst.imm));
}

void emitSelR(DecoderContext& context, InstSelR inst) {
    emitSelect(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
               inst.base.src_pred, inst.base.src_pred_inv, inst.base.src_a,
               ir::Value::createRegister(inst.src_b));
}

void emitSelC(DecoderContext& context, InstSelC inst) {
    emitSelect(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        inst.base.src_pred, inst.base.src_pred_inv, inst.base.src_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4))));
}

void emitSelI(DecoderContext& context, InstSelI inst) {
    emitSelect(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
               inst.base.src_pred, inst.base.src_pred_inv, inst.base.src_a,
               ir::Value::createConstantU(
                   getIntImm20(inst.imm20_0, inst.imm20_19, true)));
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
