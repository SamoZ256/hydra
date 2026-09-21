#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/float_min_max.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: ftz, write_cc
void emitFloatMinMax(DecoderContext& context, pred_t pred, bool pred_inv,
                     reg_t dst, pred_t src_pred, bool src_pred_inv, reg_t src_a,
                     bool abs_a, bool neg_a, ir::Value src_b, bool abs_b,
                     bool neg_b) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = absNegIf(
        context.builder, ir::Value::createRegister(src_a, ir::ScalarType::F32),
        abs_a, neg_a);
    auto src_b_v = absNegIf(context.builder, src_b, abs_b, neg_b);
    auto min_v = context.builder.opMin(src_a_v, src_b_v);
    auto max_v = context.builder.opMax(src_a_v, src_b_v);
    auto res = context.builder.opSelect(
        notIf(context.builder, ir::Value::createPredicate(src_pred),
              src_pred_inv),
        min_v, max_v);
    context.builder.opCopy(ir::Value::createRegister(dst, ir::ScalarType::F32),
                           res);

    if (conditional)
        context.builder.opEndIf();
}

} // namespace

void emitFmnmxR(DecoderContext& context, InstFmnmxR inst) {
    emitFloatMinMax(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
                    inst.base.src_pred, inst.base.src_pred_inv, inst.base.src_a,
                    inst.base.abs_a, inst.base.neg_a,
                    ir::Value::createRegister(inst.src_b, ir::ScalarType::F32),
                    inst.base.abs_b, inst.base.neg_b);
}

void emitFmnmxC(DecoderContext& context, InstFmnmxC inst) {
    emitFloatMinMax(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        inst.base.src_pred, inst.base.src_pred_inv, inst.base.src_a,
        inst.base.abs_a, inst.base.neg_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::ScalarType::F32),
        inst.base.abs_b, inst.base.neg_b);
}

void emitFmnmxI(DecoderContext& context, InstFmnmxI inst) {
    emitFloatMinMax(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        inst.base.src_pred, inst.base.src_pred_inv, inst.base.src_a,
        inst.base.abs_a, inst.base.neg_a,
        ir::Value::createConstantF(getFloatImm20(inst.imm20_0, inst.imm20_19)),
        inst.base.abs_b, inst.base.neg_b);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
