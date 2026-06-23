#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/half_comparison.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: ftz
void EmitHalf2SetPredicate(DecoderContext& context, pred_t pred, bool pred_inv,
                           FloatCmpOp op, BoolOp b_op, bool h_and,
                           pred_t dst_pred, pred_t dst_inv_pred, reg_t src_a,
                           HalfSwizzle swizzle_a, bool abs_a, bool neg_a,
                           ir::Value src_b, bool abs_b, bool neg_b,
                           pred_t src_pred, bool src_pred_inv) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = AbsNegIf(context.builder,
                            GetSwizzledHalf(context.builder, swizzle_a, src_a),
                            abs_a, neg_a);
    auto src_b_v = AbsNegIf(context.builder, src_b, abs_b, neg_b);
    auto res0 =
        GetFloatCmp(context, op, context.builder.OpVectorExtract(src_a_v, 0),
                    context.builder.OpVectorExtract(src_b_v, 0));
    auto res1 =
        GetFloatCmp(context, op, context.builder.OpVectorExtract(src_a_v, 1),
                    context.builder.OpVectorExtract(src_b_v, 1));

    if (h_and) {
        res0 = context.builder.OpBitwiseAnd(res0, res1);
        res0 = context.builder.OpBitwiseNot(res0);
    }

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

void EmitHsetp2R(DecoderContext& context, InstHsetp2R inst) {
    EmitHalf2SetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.op, inst.base.b_op,
        inst.h_and, inst.base.dst_pred, inst.base.dst_inv_pred, inst.base.src_a,
        inst.base.swizzle_a, inst.base.abs_a, inst.base.neg_a,
        GetSwizzledHalf(context.builder, inst.swizzle_b, inst.src_b),
        inst.abs_b, inst.neg_b, inst.base.src_pred, inst.base.src_pred_inv);
}

void EmitHsetp2C(DecoderContext& context, InstHsetp2C inst) {
    EmitHalf2SetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.op, inst.base.b_op,
        inst.h_and, inst.base.dst_pred, inst.base.dst_inv_pred, inst.base.src_a,
        inst.base.swizzle_a, inst.base.abs_a, inst.base.neg_a,
        ir::Value::ConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::VectorType(ir::ScalarType::F16, 2)),
        inst.abs_b, inst.neg_b, inst.base.src_pred, inst.base.src_pred_inv);
}

void EmitHsetp2I(DecoderContext& context, InstHsetp2I inst) {
    EmitHalf2SetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.op, inst.base.b_op,
        inst.h_and, inst.base.dst_pred, inst.base.dst_inv_pred, inst.base.src_a,
        inst.base.swizzle_a, inst.base.abs_a, inst.base.neg_a,
        GetHalf2Const20(context.builder, inst.h0_imm10, inst.GetH1Imm10()),
        false, false, inst.base.src_pred, inst.base.src_pred_inv);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
