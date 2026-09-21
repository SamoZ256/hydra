#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/integer_arithmetic.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: extended, write_cc, sat
void emitIadd(DecoderContext& context, pred_t pred, bool pred_inv,
              AvgMode avg_mode, reg_t dst, reg_t src_a, ir::Value src_b) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = negIf(context.builder,
                         ir::Value::createRegister(src_a, ir::ScalarType::I32),
                         avg_mode == AvgMode::NegA);
    auto src_b_v = negIf(context.builder, src_b, avg_mode == AvgMode::NegB);

    auto res = context.builder.opAdd(src_a_v, src_b_v);
    if (avg_mode == AvgMode::PlusOne)
        res = context.builder.opAdd(res, ir::Value::createConstantI(1));
    context.builder.opCopy(ir::Value::createRegister(dst, ir::ScalarType::I32),
                           res);

    if (conditional)
        context.builder.opEndIf();
}

// TODO: write_cc
void emitIscadd(DecoderContext& context, pred_t pred, bool pred_inv,
                AvgMode avg_mode, reg_t dst, reg_t src_a, u32 shift,
                ir::Value src_b) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = negIf(context.builder,
                         ir::Value::createRegister(src_a, ir::ScalarType::I32),
                         avg_mode == AvgMode::NegA);
    src_a_v =
        context.builder.opShiftLeft(src_a_v, ir::Value::createConstantU(shift));
    auto src_b_v = negIf(context.builder, src_b, avg_mode == AvgMode::NegB);

    auto res = context.builder.opAdd(src_a_v, src_b_v);
    if (avg_mode == AvgMode::PlusOne)
        res = context.builder.opAdd(res, ir::Value::createConstantI(1));
    context.builder.opCopy(ir::Value::createRegister(dst, ir::ScalarType::I32),
                           res);

    if (conditional)
        context.builder.opEndIf();
}

// TODO: check this
ir::Value extend16To32(DecoderContext& context, ir::Value value, bool high) {
    if (high)
        return context.builder.opShiftRight(value,
                                            ir::Value::createConstantU(16));
    else
        return context.builder.opBitfieldExtract(
            value, ir::Value::createConstantU(0),
            ir::Value::createConstantU(16));
}

// TODO: x, write_cc
// TODO: verify
void emitXmad(DecoderContext& context, pred_t pred, bool pred_inv, XmadCop mode,
              bool product_shift_left, bool merge, reg_t dst, reg_t src_a,
              bool a_signed, bool high_a, ir::Value src_b, bool high_b,
              ir::Value src_c) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = ir::Value::createRegister(
        src_a, a_signed ? ir::ScalarType::I32 : ir::ScalarType::U32);
    src_a_v = extend16To32(context, src_a_v, high_a);
    auto src_b_v = extend16To32(context, src_b, high_b);

    auto res = context.builder.opMultiply(
        ensureIntegerSignedness<false>(context.builder, src_a_v),
        ensureIntegerSignedness<false>(context.builder, src_b_v));
    if (product_shift_left)
        res = context.builder.opShiftLeft(res, ir::Value::createConstantU(16));

    auto src_c_v = src_c;
    switch (mode) {
    case XmadCop::Cfull:
        break;
    case XmadCop::Clo:
        src_c_v = extend16To32(context, src_c, false);
        break;
    case XmadCop::Chi:
        src_c_v = extend16To32(context, src_c, true);
        break;
    case XmadCop::Cbcc:
        src_c_v = context.builder.opAdd(
            src_c, context.builder.opShiftLeft(
                       ensureIntegerSignedness<false>(context.builder, src_b),
                       ir::Value::createConstantU(16)));
        break;
    case XmadCop::Csfu: {
        const auto src_a_adjusted = context.builder.opShiftLeft(
            context.builder.opShiftRight(
                ensureIntegerSignedness<false>(context.builder, src_a_v),
                ir::Value::createConstantU(31)),
            ir::Value::createConstantU(16));
        const auto src_b_adjusted = context.builder.opShiftLeft(
            context.builder.opShiftRight(
                ensureIntegerSignedness<false>(context.builder, src_b_v),
                ir::Value::createConstantU(31)),
            ir::Value::createConstantU(16));
        src_c_v = context.builder.opAdd(
            src_c, context.builder.opNeg(
                       context.builder.opAdd(src_a_adjusted, src_b_adjusted)));
        break;
    }
    }

    // TODO: extend
    res = context.builder.opAdd(res, src_c_v);

    if (merge) {
        res = context.builder.opBitwiseAnd(res,
                                           ir::Value::createConstantU(0xffff));
        res = context.builder.opBitwiseOr(
            res,
            context.builder.opShiftLeft(src_b, ir::Value::createConstantU(16)));
    }

    context.builder.opCopy(ir::Value::createRegister(dst, ir::ScalarType::U32),
                           res);

    if (conditional)
        context.builder.opEndIf();
}

} // namespace

void emitIaddR(DecoderContext& context, InstIaddR inst) {
    emitIadd(context, inst.base.pred, inst.base.pred_inv, inst.base.avg_mode,
             inst.base.dst, inst.base.src_a,
             ir::Value::createRegister(inst.src_b, ir::ScalarType::I32));
}

void emitIaddC(DecoderContext& context, InstIaddC inst) {
    emitIadd(
        context, inst.base.pred, inst.base.pred_inv, inst.base.avg_mode,
        inst.base.dst, inst.base.src_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::ScalarType::I32));
}

void emitIaddI(DecoderContext& context, InstIaddI inst) {
    emitIadd(context, inst.base.pred, inst.base.pred_inv, inst.base.avg_mode,
             inst.base.dst, inst.base.src_a,
             ir::Value::createConstant(
                 getIntImm20(inst.imm20_0, inst.imm20_19, true),
                 ir::ScalarType::I32));
}

void emitIadd32I(DecoderContext& context, InstIadd32I inst) {
    emitIadd(context, inst.pred, inst.pred_inv, inst.avg_mode, inst.dst,
             inst.src_a,
             ir::Value::createConstant(inst.imm, ir::ScalarType::I32));
}

void emitIscaddR(DecoderContext& context, InstIscaddR inst) {
    emitIscadd(context, inst.base.pred, inst.base.pred_inv, inst.base.avg_mode,
               inst.base.dst, inst.base.src_a, inst.base.shift,
               ir::Value::createRegister(inst.src_b, ir::ScalarType::I32));
}

void emitIscaddC(DecoderContext& context, InstIscaddC inst) {
    emitIscadd(
        context, inst.base.pred, inst.base.pred_inv, inst.base.avg_mode,
        inst.base.dst, inst.base.src_a, inst.base.shift,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::ScalarType::I32));
}

void emitIscaddI(DecoderContext& context, InstIscaddI inst) {
    emitIscadd(context, inst.base.pred, inst.base.pred_inv, inst.base.avg_mode,
               inst.base.dst, inst.base.src_a, inst.base.shift,
               ir::Value::createConstant(
                   getIntImm20(inst.imm20_0, inst.imm20_19, true),
                   ir::ScalarType::I32));
}

void emitIscadd32I(DecoderContext& context, InstIscadd32I inst) {
    emitIscadd(context, inst.pred, inst.pred_inv, AvgMode::NoNeg, inst.dst,
               inst.src_a, inst.shift,
               ir::Value::createConstant(inst.imm, ir::ScalarType::I32));
}

void emitXmadR(DecoderContext& context, InstXmadR inst) {
    emitXmad(context, inst.base.pred, inst.base.pred_inv, inst.cop, inst.psl,
             inst.mrg, inst.base.dst, inst.base.src_a, inst.base.a_signed,
             inst.base.hilo_a,
             ir::Value::createRegister(inst.src_b, inst.base.b_signed
                                                       ? ir::ScalarType::I32
                                                       : ir::ScalarType::U32),
             inst.hilo_b,
             ir::Value::createRegister(inst.src_c, ir::ScalarType::U32));
}

void emitXmadRC(DecoderContext& context, InstXmadRC inst) {
    emitXmad(
        context, inst.base.pred, inst.base.pred_inv, inst.cop, false, false,
        inst.base.dst, inst.base.src_a, inst.base.a_signed, inst.base.hilo_a,
        ir::Value::createRegister(inst.src_b, inst.base.b_signed
                                                  ? ir::ScalarType::I32
                                                  : ir::ScalarType::U32),
        inst.hilo_b,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::ScalarType::U32));
}

void emitXmadC(DecoderContext& context, InstXmadC inst) {
    emitXmad(
        context, inst.base.pred, inst.base.pred_inv, inst.cop, false, false,
        inst.base.dst, inst.base.src_a, inst.base.a_signed, inst.base.hilo_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            inst.base.b_signed ? ir::ScalarType::I32 : ir::ScalarType::U32),
        inst.hilo_b,
        ir::Value::createRegister(inst.src_c, ir::ScalarType::U32));
}

void emitXmadI(DecoderContext& context, InstXmadI inst) {
    emitXmad(context, inst.base.pred, inst.base.pred_inv, inst.cop, inst.psl,
             inst.mrg, inst.base.dst, inst.base.src_a, inst.base.a_signed,
             inst.base.hilo_a,
             ir::Value::createConstant(inst.imm16, inst.base.b_signed
                                                       ? ir::ScalarType::I32
                                                       : ir::ScalarType::U32),
             false, ir::Value::createRegister(inst.src_c, ir::ScalarType::U32));
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
