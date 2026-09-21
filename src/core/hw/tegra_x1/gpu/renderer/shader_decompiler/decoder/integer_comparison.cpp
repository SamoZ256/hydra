#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/integer_comparison.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

ir::ScalarType getDataType(bool is_signed) {
    return is_signed ? ir::ScalarType::I32 : ir::ScalarType::U32;
}

ir::Value getIntCmp(DecoderContext& context, IntCmpOp op, ir::Value a,
                    ir::Value b) {
    switch (op) {
    case IntCmpOp::F:
        return ir::Value::createConstantB(false);
    case IntCmpOp::T:
        return ir::Value::createConstantB(true);
    case IntCmpOp::Lt:
        return context.builder.opCompareLess(a, b);
    case IntCmpOp::Le:
        return context.builder.opCompareLessOrEqual(a, b);
    case IntCmpOp::Gt:
        return context.builder.opCompareGreater(a, b);
    case IntCmpOp::Ge:
        return context.builder.opCompareGreaterOrEqual(a, b);
    case IntCmpOp::Eq:
        return context.builder.opCompareEqual(a, b);
    case IntCmpOp::Ne:
        return context.builder.opCompareNotEqual(a, b);
    }
}

// TODO: x, write_cc
void emitIntSet(DecoderContext& context, pred_t pred, bool pred_inv,
                IntCmpOp op, BoolOp b_op, bool is_signed, reg_t dst,
                reg_t src_a, ir::Value src_b, pred_t src_pred,
                bool src_pred_inv, bool b_float) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = ir::Value::createRegister(src_a, getDataType(is_signed));
    auto res = getIntCmp(context, op, src_a_v, src_b);

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

// TODO: x, write_cc
void emitIntSetPredicate(DecoderContext& context, pred_t pred, bool pred_inv,
                         IntCmpOp op, BoolOp b_op, bool is_signed,
                         pred_t dst_pred, pred_t dst_inv_pred, reg_t src_a,
                         ir::Value src_b, pred_t src_pred, bool src_pred_inv) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = ir::Value::createRegister(src_a, getDataType(is_signed));
    auto res0 = getIntCmp(context, op, src_a_v, src_b);
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

void emitIsetR(DecoderContext& context, InstIsetR inst) {
    emitIntSet(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.is_signed, inst.base.dst, inst.base.src_a,
        ir::Value::createRegister(inst.src_b, getDataType(inst.base.is_signed)),
        inst.base.src_pred, inst.base.src_pred_inv, inst.base.b_float);
}

void emitIsetC(DecoderContext& context, InstIsetC inst) {
    emitIntSet(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.is_signed, inst.base.dst, inst.base.src_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            getDataType(inst.base.is_signed)),
        inst.base.src_pred, inst.base.src_pred_inv, inst.base.b_float);
}

void emitIsetI(DecoderContext& context, InstIsetI inst) {
    emitIntSet(context, inst.base.pred, inst.base.pred_inv, inst.base.op,
               inst.base.b_op, inst.base.is_signed, inst.base.dst,
               inst.base.src_a,
               ir::Value::createConstant(
                   getIntImm20(inst.imm20_0, inst.imm20_19, true),
                   getDataType(inst.base.is_signed)),
               inst.base.src_pred, inst.base.src_pred_inv, inst.base.b_float);
}

void emitIsetpR(DecoderContext& context, InstIsetpR inst) {
    emitIntSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.is_signed, inst.base.dst_pred,
        inst.base.dst_inv_pred, inst.base.src_a,
        ir::Value::createRegister(inst.src_b, getDataType(inst.base.is_signed)),
        inst.base.src_pred, inst.base.src_pred_inv);
}

void emitIsetpC(DecoderContext& context, InstIsetpC inst) {
    emitIntSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.is_signed, inst.base.dst_pred,
        inst.base.dst_inv_pred, inst.base.src_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            getDataType(inst.base.is_signed)),
        inst.base.src_pred, inst.base.src_pred_inv);
}

void emitIsetpI(DecoderContext& context, InstIsetpI inst) {
    emitIntSetPredicate(context, inst.base.pred, inst.base.pred_inv,
                        inst.base.op, inst.base.b_op, inst.base.is_signed,
                        inst.base.dst_pred, inst.base.dst_inv_pred,
                        inst.base.src_a,
                        ir::Value::createConstant(
                            getIntImm20(inst.imm20_0, inst.imm20_19, true),
                            getDataType(inst.base.is_signed)),
                        inst.base.src_pred, inst.base.src_pred_inv);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
