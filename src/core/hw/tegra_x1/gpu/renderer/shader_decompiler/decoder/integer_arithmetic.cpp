#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/integer_arithmetic.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: extended, write_cc, sat
void EmitIadd(DecoderContext& context, pred_t pred, bool pred_inv,
              AvgMode avg_mode, reg_t dst, reg_t src_a, ir::Value src_b) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v =
        NegIf(context.builder, ir::Value::Register(src_a, ir::ScalarType::I32),
              avg_mode == AvgMode::NegA);
    auto src_b_v = NegIf(context.builder, src_b, avg_mode == AvgMode::NegB);

    auto res = context.builder.OpAdd(src_a_v, src_b_v);
    if (avg_mode == AvgMode::PlusOne)
        res = context.builder.OpAdd(res, ir::Value::ConstantI(1));
    context.builder.OpCopy(ir::Value::Register(dst, ir::ScalarType::I32), res);

    if (conditional)
        context.builder.OpEndIf();
}

// TODO: write_cc
void EmitIscadd(DecoderContext& context, pred_t pred, bool pred_inv,
                AvgMode avg_mode, reg_t dst, reg_t src_a, u32 shift,
                ir::Value src_b) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v =
        NegIf(context.builder, ir::Value::Register(src_a, ir::ScalarType::I32),
              avg_mode == AvgMode::NegA);
    src_a_v = context.builder.OpShiftLeft(src_a_v, ir::Value::ConstantU(shift));
    auto src_b_v = NegIf(context.builder, src_b, avg_mode == AvgMode::NegB);

    auto res = context.builder.OpAdd(src_a_v, src_b_v);
    if (avg_mode == AvgMode::PlusOne)
        res = context.builder.OpAdd(res, ir::Value::ConstantI(1));
    context.builder.OpCopy(ir::Value::Register(dst, ir::ScalarType::I32), res);

    if (conditional)
        context.builder.OpEndIf();
}

// TODO: check this
ir::Value Extend16To32(DecoderContext& context, ir::Value value, bool high) {
    if (high)
        return context.builder.OpShiftRight(value, ir::Value::ConstantU(16));
    else
        return context.builder.OpBitfieldExtract(value, ir::Value::ConstantU(0),
                                                 ir::Value::ConstantU(16));
}

// TODO: x, write_cc
// TODO: verify
void EmitXmad(DecoderContext& context, pred_t pred, bool pred_inv, XmadCop mode,
              bool product_shift_left, bool merge, reg_t dst, reg_t src_a,
              bool a_signed, bool high_a, ir::Value src_b, bool high_b,
              ir::Value src_c) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = ir::Value::Register(src_a, a_signed ? ir::ScalarType::I32
                                                       : ir::ScalarType::U32);
    src_a_v = Extend16To32(context, src_a_v, high_a);
    auto src_b_v = Extend16To32(context, src_b, high_b);

    auto res = context.builder.OpMultiply(
        EnsureIntegerSignedness<false>(context.builder, src_a_v),
        EnsureIntegerSignedness<false>(context.builder, src_b_v));
    if (product_shift_left)
        res = context.builder.OpShiftLeft(res, ir::Value::ConstantU(16));

    auto src_c_v = src_c;
    switch (mode) {
    case XmadCop::Cfull:
        break;
    case XmadCop::Clo:
        src_c_v = Extend16To32(context, src_c, false);
        break;
    case XmadCop::Chi:
        src_c_v = Extend16To32(context, src_c, true);
        break;
    case XmadCop::Cbcc:
        src_c_v = context.builder.OpAdd(
            src_c, context.builder.OpShiftLeft(
                       EnsureIntegerSignedness<false>(context.builder, src_b),
                       ir::Value::ConstantU(16)));
        break;
    case XmadCop::Csfu: {
        const auto src_a_adjusted = context.builder.OpShiftLeft(
            context.builder.OpShiftRight(
                EnsureIntegerSignedness<false>(context.builder, src_a_v),
                ir::Value::ConstantU(31)),
            ir::Value::ConstantU(16));
        const auto src_b_adjusted = context.builder.OpShiftLeft(
            context.builder.OpShiftRight(
                EnsureIntegerSignedness<false>(context.builder, src_b_v),
                ir::Value::ConstantU(31)),
            ir::Value::ConstantU(16));
        src_c_v = context.builder.OpAdd(
            src_c, context.builder.OpNeg(
                       context.builder.OpAdd(src_a_adjusted, src_b_adjusted)));
        break;
    }
    }

    // TODO: extend
    res = context.builder.OpAdd(res, src_c_v);

    if (merge) {
        res = context.builder.OpBitwiseAnd(res, ir::Value::ConstantU(0xffff));
        res = context.builder.OpBitwiseOr(
            res, context.builder.OpShiftLeft(src_b, ir::Value::ConstantU(16)));
    }

    context.builder.OpCopy(ir::Value::Register(dst, ir::ScalarType::U32), res);

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace

void EmitIaddR(DecoderContext& context, InstIaddR inst) {
    EmitIadd(context, inst.base.pred, inst.base.pred_inv, inst.base.avg_mode,
             inst.base.dst, inst.base.src_a,
             ir::Value::Register(inst.src_b, ir::ScalarType::I32));
}

void EmitIaddC(DecoderContext& context, InstIaddC inst) {
    EmitIadd(
        context, inst.base.pred, inst.base.pred_inv, inst.base.avg_mode,
        inst.base.dst, inst.base.src_a,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               ir::ScalarType::I32));
}

void EmitIaddI(DecoderContext& context, InstIaddI inst) {
    EmitIadd(context, inst.base.pred, inst.base.pred_inv, inst.base.avg_mode,
             inst.base.dst, inst.base.src_a,
             ir::Value::Constant(GetIntImm20(inst.imm20_0, inst.imm20_19, true),
                                 ir::ScalarType::I32));
}

void EmitIadd32I(DecoderContext& context, InstIadd32I inst) {
    EmitIadd(context, inst.pred, inst.pred_inv, inst.avg_mode, inst.dst,
             inst.src_a, ir::Value::Constant(inst.imm, ir::ScalarType::I32));
}

void EmitIscaddR(DecoderContext& context, InstIscaddR inst) {
    EmitIscadd(context, inst.base.pred, inst.base.pred_inv, inst.base.avg_mode,
               inst.base.dst, inst.base.src_a, inst.base.shift,
               ir::Value::Register(inst.src_b, ir::ScalarType::I32));
}

void EmitIscaddC(DecoderContext& context, InstIscaddC inst) {
    EmitIscadd(
        context, inst.base.pred, inst.base.pred_inv, inst.base.avg_mode,
        inst.base.dst, inst.base.src_a, inst.base.shift,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               ir::ScalarType::I32));
}

void EmitIscaddI(DecoderContext& context, InstIscaddI inst) {
    EmitIscadd(
        context, inst.base.pred, inst.base.pred_inv, inst.base.avg_mode,
        inst.base.dst, inst.base.src_a, inst.base.shift,
        ir::Value::Constant(GetIntImm20(inst.imm20_0, inst.imm20_19, true),
                            ir::ScalarType::I32));
}

void EmitIscadd32I(DecoderContext& context, InstIscadd32I inst) {
    EmitIscadd(context, inst.pred, inst.pred_inv, AvgMode::NoNeg, inst.dst,
               inst.src_a, inst.shift,
               ir::Value::Constant(inst.imm, ir::ScalarType::I32));
}

void EmitXmadR(DecoderContext& context, InstXmadR inst) {
    EmitXmad(context, inst.base.pred, inst.base.pred_inv, inst.cop, inst.psl,
             inst.mrg, inst.base.dst, inst.base.src_a, inst.base.a_signed,
             inst.base.hilo_a,
             ir::Value::Register(inst.src_b, inst.base.b_signed
                                                 ? ir::ScalarType::I32
                                                 : ir::ScalarType::U32),
             inst.hilo_b, ir::Value::Register(inst.src_c, ir::ScalarType::U32));
}

void EmitXmadRC(DecoderContext& context, InstXmadRC inst) {
    EmitXmad(
        context, inst.base.pred, inst.base.pred_inv, inst.cop, false, false,
        inst.base.dst, inst.base.src_a, inst.base.a_signed, inst.base.hilo_a,
        ir::Value::Register(inst.src_b, inst.base.b_signed
                                            ? ir::ScalarType::I32
                                            : ir::ScalarType::U32),
        inst.hilo_b,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               ir::ScalarType::U32));
}

void EmitXmadC(DecoderContext& context, InstXmadC inst) {
    EmitXmad(
        context, inst.base.pred, inst.base.pred_inv, inst.cop, false, false,
        inst.base.dst, inst.base.src_a, inst.base.a_signed, inst.base.hilo_a,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               inst.base.b_signed ? ir::ScalarType::I32
                                                  : ir::ScalarType::U32),
        inst.hilo_b, ir::Value::Register(inst.src_c, ir::ScalarType::U32));
}

void EmitXmadI(DecoderContext& context, InstXmadI inst) {
    EmitXmad(context, inst.base.pred, inst.base.pred_inv, inst.cop, inst.psl,
             inst.mrg, inst.base.dst, inst.base.src_a, inst.base.a_signed,
             inst.base.hilo_a,
             ir::Value::Constant(inst.imm16, inst.base.b_signed
                                                 ? ir::ScalarType::I32
                                                 : ir::ScalarType::U32),
             false, ir::Value::Register(inst.src_c, ir::ScalarType::U32));
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
