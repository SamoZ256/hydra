#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/integer_logical.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: x, write_cc
void emitLogical(DecoderContext& context, pred_t pred, bool pred_inv,
                 LogicOp op, PredicateOp pred_op, reg_t dst, pred_t dst_pred,
                 reg_t src_a, bool inv_a, ir::Value src_b, bool inv_b) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = ir::Value::createRegister(src_a);
    if (inv_a)
        src_a_v = context.builder.opBitwiseNot(src_a_v);
    auto src_b_v = src_b;
    if (inv_b)
        src_b_v = context.builder.opBitwiseNot(src_b_v);

    auto res = ir::Value::createUndefined();
    switch (op) {
    case LogicOp::And:
        res = context.builder.opBitwiseAnd(src_a_v, src_b_v);
        break;
    case LogicOp::Or:
        res = context.builder.opBitwiseOr(src_a_v, src_b_v);
        break;
    case LogicOp::Xor:
        res = context.builder.opBitwiseXor(src_a_v, src_b_v);
        break;
    case LogicOp::PassB:
        res = src_b_v;
        break;
    default:
        unreachable();
    }
    context.builder.opCopy(ir::Value::createRegister(dst), res);

    auto pred_res = ir::Value::createUndefined();
    switch (pred_op) {
    case PredicateOp::F:
        pred_res = ir::Value::createConstantB(false);
        break;
    case PredicateOp::T:
        pred_res = ir::Value::createConstantB(true);
        break;
    case PredicateOp::Z:
        pred_res =
            context.builder.opCompareEqual(res, ir::Value::createConstantU(0));
        break;
    case PredicateOp::Nz:
        pred_res = context.builder.opCompareNotEqual(
            res, ir::Value::createConstantU(0));
        break;
    default:
        unreachable();
    }
    context.builder.opCopy(ir::Value::createPredicate(dst_pred), pred_res);

    if (conditional)
        context.builder.opEndIf();
}

} // namespace

void emitLopR(DecoderContext& context, InstLopR inst) {
    emitLogical(context, inst.base.pred, inst.base.pred_inv, inst.base.op,
                inst.base.pred_op, inst.base.dst, inst.base.dst_pred,
                inst.base.src_a, inst.base.inv_a,
                ir::Value::createRegister(inst.src_b), inst.base.inv_b);
}

void emitLopC(DecoderContext& context, InstLopC inst) {
    emitLogical(
        context, inst.base.pred, inst.base.pred_inv, inst.base.op,
        inst.base.pred_op, inst.base.dst, inst.base.dst_pred, inst.base.src_a,
        inst.base.inv_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4))),
        inst.base.inv_b);
}

void emitLopI(DecoderContext& context, InstLopI inst) {
    emitLogical(context, inst.base.pred, inst.base.pred_inv, inst.base.op,
                inst.base.pred_op, inst.base.dst, inst.base.dst_pred,
                inst.base.src_a, inst.base.inv_a,
                ir::Value::createConstantU(
                    getIntImm20(inst.imm20_0, inst.imm20_19, true)),
                inst.base.inv_b);
}

void emitLop32I(DecoderContext& context, InstLop32I inst) {
    emitLogical(context, inst.pred, inst.pred_inv, inst.op, PredicateOp::F,
                inst.dst, PT, inst.src_a, inst.inv_a,
                ir::Value::createConstantU(inst.imm), inst.inv_b);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
