#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/integer_comparison.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

DataType GetDataType(bool is_signed) {
    return is_signed ? DataType::I32 : DataType::U32;
}

ir::Value GetIntCmp(DecoderContext& context, IntCmpOp op, ir::Value a,
                    ir::Value b) {
    switch (op) {
    case IntCmpOp::F:
        return ir::Value::Immediate(false);
    case IntCmpOp::T:
        return ir::Value::Immediate(true);
    case IntCmpOp::Lt:
        return context.builder.OpCompareLess(a, b);
    case IntCmpOp::Le:
        return context.builder.OpCompareLessOrEqual(a, b);
    case IntCmpOp::Gt:
        return context.builder.OpCompareGreater(a, b);
    case IntCmpOp::Ge:
        return context.builder.OpCompareGreaterOrEqual(a, b);
    case IntCmpOp::Eq:
        return context.builder.OpCompareEqual(a, b);
    case IntCmpOp::Ne:
        return context.builder.OpCompareNotEqual(a, b);
    }
}

// TODO: x, write_cc
void EmitIntSet(DecoderContext& context, pred_t pred, bool pred_inv,
                IntCmpOp op, BoolOp b_op, bool is_signed, reg_t dst,
                reg_t src_a, ir::Value src_b, pred_t src_pred,
                bool src_pred_inv, bool b_float) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = ir::Value::Register(src_a, GetDataType(is_signed));
    auto res = GetIntCmp(context, op, src_a_v, src_b);

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

// TODO: x, write_cc
void EmitIntSetPredicate(DecoderContext& context, pred_t pred, bool pred_inv,
                         IntCmpOp op, BoolOp b_op, bool is_signed,
                         pred_t dst_pred, pred_t dst_inv_pred, reg_t src_a,
                         ir::Value src_b, pred_t src_pred, bool src_pred_inv) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = ir::Value::Register(src_a, GetDataType(is_signed));
    auto res0 = GetIntCmp(context, op, src_a_v, src_b);
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

void EmitIsetR(DecoderContext& context, InstIsetR inst) {
    EmitIntSet(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.is_signed, inst.base.dst, inst.base.src_a,
        ir::Value::Register(inst.src_b, GetDataType(inst.base.is_signed)),
        inst.base.src_pred, inst.base.src_pred_inv, inst.base.b_float);
}

void EmitIsetC(DecoderContext& context, InstIsetC inst) {
    EmitIntSet(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.is_signed, inst.base.dst, inst.base.src_a,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               GetDataType(inst.base.is_signed)),
        inst.base.src_pred, inst.base.src_pred_inv, inst.base.b_float);
}

void EmitIsetI(DecoderContext& context, InstIsetI inst) {
    EmitIntSet(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.is_signed, inst.base.dst, inst.base.src_a,
        ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                             GetDataType(inst.base.is_signed)),
        inst.base.src_pred, inst.base.src_pred_inv, inst.base.b_float);
}

void EmitIsetpR(DecoderContext& context, InstIsetpR inst) {
    EmitIntSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.is_signed, inst.base.dst_pred,
        inst.base.dst_inv_pred, inst.base.src_a,
        ir::Value::Register(inst.src_b, GetDataType(inst.base.is_signed)),
        inst.base.src_pred, inst.base.src_pred_inv);
}

void EmitIsetpC(DecoderContext& context, InstIsetpC inst) {
    EmitIntSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.is_signed, inst.base.dst_pred,
        inst.base.dst_inv_pred, inst.base.src_a,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               GetDataType(inst.base.is_signed)),
        inst.base.src_pred, inst.base.src_pred_inv);
}

void EmitIsetpI(DecoderContext& context, InstIsetpI inst) {
    EmitIntSetPredicate(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.b_op, inst.base.is_signed, inst.base.dst_pred,
        inst.base.dst_inv_pred, inst.base.src_a,
        ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                             GetDataType(inst.base.is_signed)),
        inst.base.src_pred, inst.base.src_pred_inv);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
