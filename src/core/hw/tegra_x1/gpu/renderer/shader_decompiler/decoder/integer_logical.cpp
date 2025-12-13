#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/integer_logical.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: x, write_cc
void EmitLogical(DecoderContext& context, pred_t pred, bool pred_inv,
                 LogicOp op, PredicateOp pred_op, reg_t dst, pred_t dst_pred,
                 reg_t src_a, bool inv_a, ir::Value src_b, bool inv_b) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = ir::Value::Register(src_a);
    if (inv_a)
        src_a_v = context.builder.OpBitwiseNot(src_a_v);
    auto src_b_v = src_b;
    if (inv_b)
        src_b_v = context.builder.OpBitwiseNot(src_b_v);

    auto res = ir::Value::Undefined();
    switch (op) {
    case LogicOp::And:
        res = context.builder.OpBitwiseAnd(src_a_v, src_b_v);
        break;
    case LogicOp::Or:
        res = context.builder.OpBitwiseOr(src_a_v, src_b_v);
        break;
    case LogicOp::Xor:
        res = context.builder.OpBitwiseXor(src_a_v, src_b_v);
        break;
    case LogicOp::PassB:
        res = src_b_v;
        break;
    default:
        unreachable();
    }
    context.builder.OpCopy(ir::Value::Register(dst), res);

    auto pred_res = ir::Value::Undefined();
    switch (pred_op) {
    case PredicateOp::F:
        pred_res = ir::Value::Immediate(false);
        break;
    case PredicateOp::T:
        pred_res = ir::Value::Immediate(true);
        break;
    case PredicateOp::Z:
        pred_res = context.builder.OpCompareEqual(res, ir::Value::Immediate(0));
        break;
    case PredicateOp::Nz:
        pred_res =
            context.builder.OpCompareNotEqual(res, ir::Value::Immediate(0));
        break;
    default:
        unreachable();
    }
    context.builder.OpCopy(ir::Value::Predicate(dst_pred), pred_res);

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace

void EmitLopR(DecoderContext& context, InstLopR inst) {
    EmitLogical(context, inst.base.pred, inst.base.pred_inv, inst.base.op,
                inst.base.pred_op, inst.base.dst, inst.base.dst_pred,
                inst.base.src_a, inst.base.inv_a,
                ir::Value::Register(inst.src_b), inst.base.inv_b);
}

void EmitLopC(DecoderContext& context, InstLopC inst) {
    EmitLogical(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.pred_op, inst.base.dst, inst.base.dst_pred, inst.base.src_a,
        inst.base.inv_a,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4)),
        inst.base.inv_b);
}

void EmitLopI(DecoderContext& context, InstLopI inst) {
    // TODO: really sign-extend?
    EmitLogical(context, inst.base.pred, inst.base.pred_inv, inst.base.op,
                inst.base.pred_op, inst.base.dst, inst.base.dst_pred,
                inst.base.src_a, inst.base.inv_a,
                ir::Value::Immediate(std::bit_cast<u32>(sign_extend<i32, 20>(
                    inst.imm20_0 | (inst.imm20_19 << 19)))),
                inst.base.inv_b);
}

void EmitLop32I(DecoderContext& context, InstLop32I inst) {
    EmitLogical(context, inst.pred, inst.pred_inv, inst.op, PredicateOp::F,
                inst.dst, PT, inst.src_a, inst.inv_a,
                ir::Value::Immediate(inst.imm), inst.inv_b);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
