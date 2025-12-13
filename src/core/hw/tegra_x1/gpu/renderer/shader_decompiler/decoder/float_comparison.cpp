#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/float_comparison.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

ir::Value GetFloatCmp(DecoderContext& context, FloatCmpOp op, ir::Value a,
                      ir::Value b) {
    // TODO: handle U versions differently?
    switch (op) {
    case FloatCmpOp::F:
        return ir::Value::Immediate(false);
    case FloatCmpOp::T:
        return ir::Value::Immediate(true);
    case FloatCmpOp::Lt:
    case FloatCmpOp::Ltu:
        return context.builder.OpCompareLess(a, b);
    case FloatCmpOp::Le:
    case FloatCmpOp::Leu:
        return context.builder.OpCompareLessOrEqual(a, b);
    case FloatCmpOp::Gt:
    case FloatCmpOp::Gtu:
        return context.builder.OpCompareGreater(a, b);
    case FloatCmpOp::Ge:
    case FloatCmpOp::Geu:
        return context.builder.OpCompareGreaterOrEqual(a, b);
    case FloatCmpOp::Eq:
    case FloatCmpOp::Equ:
        return context.builder.OpCompareEqual(a, b);
    case FloatCmpOp::Ne:
    case FloatCmpOp::Neu:
        return context.builder.OpCompareNotEqual(a, b);
    case FloatCmpOp::Num:
    case FloatCmpOp::Nan: {
        const auto res = context.builder.OpBitwiseOr(a, b);
        if (op == FloatCmpOp::Num)
            return context.builder.OpNot(res);
        else
            return res;
    }
    }
}

// TODO: write_cc, ftz
void EmitFloatSet(DecoderContext& context, pred_t pred, bool pred_inv,
                  FloatCmpOp op, BoolOp b_op, reg_t dst, reg_t src_a,
                  bool abs_a, bool neg_a, ir::Value src_b, bool abs_b,
                  bool neg_b, pred_t src_pred, bool src_pred_inv,
                  bool b_float) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v =
        AbsNegIf(context.builder, ir::Value::Register(src_a, DataType::F32),
                 abs_a, neg_a);
    auto src_b_v = AbsNegIf(context.builder, src_b, abs_b, neg_b);
    auto res = GetFloatCmp(context, op, src_a_v, src_b_v);

    const auto pred_v =
        NotIf(context.builder, ir::Value::Predicate(src_pred), src_pred_inv);
    res = GetLogical(context, b_op, res, pred_v);

    if (b_float) {
        res = context.builder.OpSelect(
            res, ir::Value::Immediate(std::bit_cast<u32>(1.0f), DataType::F32),
            ir::Value::Immediate(std::bit_cast<u32>(0.0f), DataType::F32));
        context.builder.OpCopy(ir::Value::Register(dst, DataType::F32), res);
    } else {
        context.builder.OpCopy(ir::Value::Register(dst), res);
    }

    if (conditional)
        context.builder.OpEndIf();
}

// TODO: ftz
void EmitFloatSetPredicate(DecoderContext& context, pred_t pred, bool pred_inv,
                           FloatCmpOp op, BoolOp b_op, pred_t dst_pred,
                           pred_t dst_inv_pred, reg_t src_a, bool abs_a,
                           bool neg_a, ir::Value src_b, bool abs_b, bool neg_b,
                           pred_t src_pred, bool src_pred_inv) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v =
        AbsNegIf(context.builder, ir::Value::Register(src_a, DataType::F32),
                 abs_a, neg_a);
    auto src_b_v = AbsNegIf(context.builder, src_b, abs_b, neg_b);
    auto res0 = GetFloatCmp(context, op, src_a_v, src_b_v);
    auto res1 = context.builder.OpNot(res0);

    const auto pred_v =
        NotIf(context.builder, ir::Value::Predicate(src_pred), src_pred_inv);
    res0 = GetLogical(context, b_op, res0, pred_v);
    res1 = GetLogical(context, b_op, res1, pred_v);

    context.builder.OpCopy(ir::Value::Predicate(dst_pred), res0);
    context.builder.OpCopy(ir::Value::Predicate(dst_inv_pred), res1);

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace

void EmitFsetR(DecoderContext& context, InstFsetR inst) {
    EmitFloatSet(context, inst.base.pred, inst.base.pred_inv, inst.base.op,
                 inst.base.b_op, inst.base.dst, inst.base.src_a,
                 inst.base.abs_a, inst.base.neg_a,
                 ir::Value::Register(inst.src_b, DataType::F32),
                 inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
                 inst.base.src_pred_inv, inst.base.b_float);
}

void EmitFsetC(DecoderContext& context, InstFsetC inst) {
    EmitFloatSet(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst, inst.base.src_a, inst.base.abs_a,
        inst.base.neg_a,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               DataType::F32),
        inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
        inst.base.src_pred_inv, inst.base.b_float);
}

void EmitFsetI(DecoderContext& context, InstFsetI inst) {
    EmitFloatSet(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst, inst.base.src_a, inst.base.abs_a,
        inst.base.neg_a,
        ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                             DataType::F32),
        inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
        inst.base.src_pred_inv, inst.base.b_float);
}

void EmitFsetpR(DecoderContext& context, InstFsetpR inst) {
    EmitFloatSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst_pred, inst.base.dst_inv_pred,
        inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
        ir::Value::Register(inst.src_b, DataType::F32), inst.base.abs_b,
        inst.base.neg_b, inst.base.src_pred, inst.base.src_pred_inv);
}

void EmitFsetpC(DecoderContext& context, InstFsetpC inst) {
    EmitFloatSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst_pred, inst.base.dst_inv_pred,
        inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               DataType::F32),
        inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
        inst.base.src_pred_inv);
}

void EmitFsetpI(DecoderContext& context, InstFsetpI inst) {
    EmitFloatSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst_pred, inst.base.dst_inv_pred,
        inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
        ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                             DataType::F32),
        inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
        inst.base.src_pred_inv);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
