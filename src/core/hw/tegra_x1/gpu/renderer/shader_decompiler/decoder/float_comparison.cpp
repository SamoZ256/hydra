#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/float_comparison.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: write_cc, ftz
void EmitFloatSet(DecoderContext& context, pred_t pred, bool pred_inv,
                  FloatCmpOp op, BoolOp b_op, reg_t dst, reg_t src_a,
                  bool abs_a, bool neg_a, ir::Value src_b, bool abs_b,
                  bool neg_b, pred_t src_pred, bool src_pred_inv,
                  bool b_float) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v =
        AbsNegIf(context.builder,
                 ir::Value::Register(src_a, ir::ScalarType::F32), abs_a, neg_a);
    auto src_b_v = AbsNegIf(context.builder, src_b, abs_b, neg_b);
    auto res = GetFloatCmp(context, op, src_a_v, src_b_v);

    const auto pred_v =
        NotIf(context.builder, ir::Value::Predicate(src_pred), src_pred_inv);
    res = GetLogical(context, b_op, res, pred_v);

    if (b_float) {
        res = context.builder.OpSelect(res, ir::Value::ConstantF(1.0f),
                                       ir::Value::ConstantF(0.0f));
        context.builder.OpCopy(ir::Value::Register(dst, ir::ScalarType::F32),
                               res);
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
        AbsNegIf(context.builder,
                 ir::Value::Register(src_a, ir::ScalarType::F32), abs_a, neg_a);
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
                 ir::Value::Register(inst.src_b, ir::ScalarType::F32),
                 inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
                 inst.base.src_pred_inv, inst.base.b_float);
}

void EmitFsetC(DecoderContext& context, InstFsetC inst) {
    EmitFloatSet(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst, inst.base.src_a, inst.base.abs_a,
        inst.base.neg_a,
        ir::Value::ConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::ScalarType::F32),
        inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
        inst.base.src_pred_inv, inst.base.b_float);
}

void EmitFsetI(DecoderContext& context, InstFsetI inst) {
    EmitFloatSet(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst, inst.base.src_a, inst.base.abs_a,
        inst.base.neg_a,
        ir::Value::ConstantF(GetFloatImm20(inst.imm20_0, inst.imm20_19)),
        inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
        inst.base.src_pred_inv, inst.base.b_float);
}

void EmitFsetpR(DecoderContext& context, InstFsetpR inst) {
    EmitFloatSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst_pred, inst.base.dst_inv_pred,
        inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
        ir::Value::Register(inst.src_b, ir::ScalarType::F32), inst.base.abs_b,
        inst.base.neg_b, inst.base.src_pred, inst.base.src_pred_inv);
}

void EmitFsetpC(DecoderContext& context, InstFsetpC inst) {
    EmitFloatSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst_pred, inst.base.dst_inv_pred,
        inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
        ir::Value::ConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::ScalarType::F32),
        inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
        inst.base.src_pred_inv);
}

void EmitFsetpI(DecoderContext& context, InstFsetpI inst) {
    EmitFloatSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.dst_pred, inst.base.dst_inv_pred,
        inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
        ir::Value::ConstantF(GetFloatImm20(inst.imm20_0, inst.imm20_19)),
        inst.base.abs_b, inst.base.neg_b, inst.base.src_pred,
        inst.base.src_pred_inv);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
