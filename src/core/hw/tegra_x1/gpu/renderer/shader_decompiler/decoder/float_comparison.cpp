#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/float_comparison.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: write_cc, ftz
void emitFloatSet(DecoderContext& context, pred_t pred, bool pred_inv,
                  FloatCmpOp op, BoolOp b_op, reg_t dst, reg_t src_a,
                  bool abs_a, bool neg_a, ir::Value src_b, bool abs_b,
                  bool neg_b, pred_t src_pred, bool src_pred_inv,
                  bool b_float) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = absNegIf(
        context.builder, ir::Value::createRegister(src_a, ir::ScalarType::F32),
        abs_a, neg_a);
    auto src_b_v = absNegIf(context.builder, src_b, abs_b, neg_b);
    auto res = getFloatCmp(context, op, src_a_v, src_b_v);

    const auto pred_v = notIf(
        context.builder, ir::Value::createPredicate(src_pred), src_pred_inv);
    res = getLogical(context, b_op, res, pred_v);

    if (b_float) {
        res = context.builder.opSelect(res, ir::Value::createConstantF(1.0f),
                                       ir::Value::createConstantF(0.0f));
        context.builder.opCopy(
            ir::Value::createRegister(dst, ir::ScalarType::F32), res);
    } else {
        context.builder.opCopy(ir::Value::createRegister(dst), res);
    }

    if (conditional)
        context.builder.opEndIf();
}

// TODO: ftz
void emitFloatSetPredicate(DecoderContext& context, pred_t pred, bool pred_inv,
                           FloatCmpOp op, BoolOp b_op, pred_t dst_pred,
                           pred_t dst_inv_pred, reg_t src_a, bool abs_a,
                           bool neg_a, ir::Value src_b, bool abs_b, bool neg_b,
                           pred_t src_pred, bool src_pred_inv) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = absNegIf(
        context.builder, ir::Value::createRegister(src_a, ir::ScalarType::F32),
        abs_a, neg_a);
    auto src_b_v = absNegIf(context.builder, src_b, abs_b, neg_b);
    auto res0 = getFloatCmp(context, op, src_a_v, src_b_v);
    auto res1 = context.builder.opNot(res0);

    const auto pred_v = notIf(
        context.builder, ir::Value::createPredicate(src_pred), src_pred_inv);
    res0 = getLogical(context, b_op, res0, pred_v);
    res1 = getLogical(context, b_op, res1, pred_v);

    context.builder.opCopy(ir::Value::createPredicate(dst_pred), res0);
    context.builder.opCopy(ir::Value::createPredicate(dst_inv_pred), res1);

    if (conditional)
        context.builder.opEndIf();
}

} // namespace

void emitFsetR(DecoderContext& context, InstFsetR inst) {
    emitFloatSet(context, inst.base.pred, inst.base.pred_inv, inst.base.op,
                 inst.base.b_op, inst.base.dst, inst.base.src_a,
                 inst.base.abs_a, inst.base.neg_a,
                 ir::Value::createRegister(inst.src_b, ir::ScalarType::F32),
                 inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
                 inst.base.src_pred_inv, inst.base.b_float);
}

void emitFsetC(DecoderContext& context, InstFsetC inst) {
    emitFloatSet(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst, inst.base.src_a, inst.base.abs_a,
        inst.base.neg_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::ScalarType::F32),
        inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
        inst.base.src_pred_inv, inst.base.b_float);
}

void emitFsetI(DecoderContext& context, InstFsetI inst) {
    emitFloatSet(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst, inst.base.src_a, inst.base.abs_a,
        inst.base.neg_a,
        ir::Value::createConstantF(getFloatImm20(inst.imm20_0, inst.imm20_19)),
        inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
        inst.base.src_pred_inv, inst.base.b_float);
}

void emitFsetpR(DecoderContext& context, InstFsetpR inst) {
    emitFloatSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst_pred, inst.base.dst_inv_pred,
        inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
        ir::Value::createRegister(inst.src_b, ir::ScalarType::F32),
        inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
        inst.base.src_pred_inv);
}

void emitFsetpC(DecoderContext& context, InstFsetpC inst) {
    emitFloatSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst_pred, inst.base.dst_inv_pred,
        inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::ScalarType::F32),
        inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
        inst.base.src_pred_inv);
}

void emitFsetpI(DecoderContext& context, InstFsetpI inst) {
    emitFloatSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst_pred, inst.base.dst_inv_pred,
        inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
        ir::Value::createConstantF(getFloatImm20(inst.imm20_0, inst.imm20_19)),
        inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
        inst.base.src_pred_inv);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
