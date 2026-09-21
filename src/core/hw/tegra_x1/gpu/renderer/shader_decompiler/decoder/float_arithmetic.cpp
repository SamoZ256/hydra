#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/float_arithmetic.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: round_mode, ftz, write_cc
void emitFadd(DecoderContext& context, pred_t pred, bool pred_inv,
              bool saturate, reg_t dst, reg_t src_a, bool abs_a, bool neg_a,
              ir::Value src_b, bool abs_b, bool neg_b) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = absNegIf(
        context.builder, ir::Value::createRegister(src_a, ir::ScalarType::F32),
        abs_a, neg_a);
    auto src_b_v = absNegIf(context.builder, src_b, abs_b, neg_b);

    auto res = context.builder.opAdd(src_a_v, src_b_v);
    res = saturateIf(context.builder, res, saturate);
    context.builder.opCopy(ir::Value::createRegister(dst, ir::ScalarType::F32),
                           res);

    if (conditional)
        context.builder.opEndIf();
}

// TODO: round_mode, fmz, write_cc
void emitFmul(DecoderContext& context, pred_t pred, bool pred_inv,
              MultiplyScale scale, bool saturate, reg_t dst, reg_t src_a,
              ir::Value src_b, bool neg_b) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = ir::Value::createRegister(src_a, ir::ScalarType::F32);
    if (scale != MultiplyScale::None) {
        f32 scale_f;
        switch (scale) {
        case MultiplyScale::M2:
            scale_f = 2.0f;
            break;
        case MultiplyScale::M4:
            scale_f = 4.0f;
            break;
        case MultiplyScale::M8:
            scale_f = 8.0f;
            break;
        case MultiplyScale::D2:
            scale_f = 0.5f;
            break;
        case MultiplyScale::D4:
            scale_f = 0.25f;
            break;
        case MultiplyScale::D8:
            scale_f = 0.125f;
            break;
        default:
            unreachable();
        }
        src_a_v = context.builder.opMultiply(
            src_a_v, ir::Value::createConstantF(scale_f));
    }

    auto res = context.builder.opMultiply(src_a_v,
                                          negIf(context.builder, src_b, neg_b));
    res = saturateIf(context.builder, res, saturate);
    context.builder.opCopy(ir::Value::createRegister(dst, ir::ScalarType::F32),
                           res);

    if (conditional)
        context.builder.opEndIf();
}

// TODO: round_mode, ftz, write_cc
void emitFfma(DecoderContext& context, pred_t pred, bool pred_inv,
              bool saturate, reg_t dst, reg_t src_a, bool neg_a,
              ir::Value src_b, bool neg_b, ir::Value src_c, bool neg_c) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_a_v =
        negIf(context.builder,
              ir::Value::createRegister(src_a, ir::ScalarType::F32), neg_a);
    auto src_b_v = negIf(context.builder, src_b, neg_b);
    auto src_c_v = negIf(context.builder, src_c, neg_c);

    auto res = context.builder.opFma(src_a_v, src_b_v, src_c_v);
    res = saturateIf(context.builder, res, saturate);
    context.builder.opCopy(ir::Value::createRegister(dst, ir::ScalarType::F32),
                           res);

    if (conditional)
        context.builder.opEndIf();
}

} // namespace

void emitFaddR(DecoderContext& context, InstFaddR inst) {
    emitFadd(context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
             inst.base.dst, inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
             ir::Value::createRegister(inst.src_b, ir::ScalarType::F32),
             inst.base.abs_b, inst.base.neg_b);
}

void emitFaddC(DecoderContext& context, InstFaddC inst) {
    emitFadd(
        context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
        inst.base.dst, inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::ScalarType::F32),
        inst.base.abs_b, inst.base.neg_b);
}

void emitFaddI(DecoderContext& context, InstFaddI inst) {
    emitFadd(
        context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
        inst.base.dst, inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
        ir::Value::createConstantF(getFloatImm20(inst.imm20_0, inst.imm20_19)),
        inst.base.abs_b, inst.base.neg_b);
}

void emitFadd32I(DecoderContext& context, InstFadd32I inst) {
    emitFadd(context, inst.pred, inst.pred_inv, false, inst.dst, inst.src_a,
             inst.abs_a, inst.neg_a,
             ir::Value::createConstant(inst.imm, ir::ScalarType::F32),
             inst.abs_b, inst.neg_b);
}

void emitFmulR(DecoderContext& context, InstFmulR inst) {
    emitFmul(context, inst.base.pred, inst.base.pred_inv, inst.base.scale,
             inst.base.sat, inst.base.dst, inst.base.src_a,
             ir::Value::createRegister(inst.src_b, ir::ScalarType::F32),
             inst.base.neg_b);
}

void emitFmulC(DecoderContext& context, InstFmulC inst) {
    emitFmul(
        context, inst.base.pred, inst.base.pred_inv, inst.base.scale,
        inst.base.sat, inst.base.dst, inst.base.src_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::ScalarType::F32),
        inst.base.neg_b);
}

void emitFmulI(DecoderContext& context, InstFmulI inst) {
    emitFmul(
        context, inst.base.pred, inst.base.pred_inv, inst.base.scale,
        inst.base.sat, inst.base.dst, inst.base.src_a,
        ir::Value::createConstantF(getFloatImm20(inst.imm20_0, inst.imm20_19)),
        inst.base.neg_b);
}

void emitFmul32I(DecoderContext& context, InstFmul32I inst) {
    emitFmul(context, inst.pred, inst.pred_inv, MultiplyScale::None, inst.sat,
             inst.dst, inst.src_a,
             ir::Value::createConstant(inst.imm, ir::ScalarType::F32), false);
}

void emitFfmaR(DecoderContext& context, InstFfmaR inst) {
    emitFfma(context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
             inst.base.dst, inst.base.src_a, false,
             ir::Value::createRegister(inst.src_b, ir::ScalarType::F32),
             inst.base.neg_b,
             ir::Value::createRegister(inst.src_c, ir::ScalarType::F32),
             inst.base.neg_c);
}

void emitFfmaRC(DecoderContext& context, InstFfmaRC inst) {
    emitFfma(
        context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
        inst.base.dst, inst.base.src_a, false,
        ir::Value::createRegister(inst.src_b, ir::ScalarType::F32),
        inst.base.neg_b,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::ScalarType::F32),
        inst.base.neg_c);
}

void emitFfmaC(DecoderContext& context, InstFfmaC inst) {
    emitFfma(
        context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
        inst.base.dst, inst.base.src_a, false,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::ScalarType::F32),
        inst.base.neg_b,
        ir::Value::createRegister(inst.src_c, ir::ScalarType::F32),
        inst.base.neg_c);
}

void emitFfmaI(DecoderContext& context, InstFfmaI inst) {
    emitFfma(
        context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
        inst.base.dst, inst.base.src_a, false,
        ir::Value::createConstantF(getFloatImm20(inst.imm20_0, inst.imm20_19)),
        inst.base.neg_b,
        ir::Value::createRegister(inst.src_c, ir::ScalarType::F32),
        inst.base.neg_c);
}

void emitFfma32I(DecoderContext& context, InstFfma32I inst) {
    emitFfma(context, inst.pred, inst.pred_inv, inst.sat, inst.dst, inst.src_a,
             inst.neg_a,
             ir::Value::createConstant(inst.imm, ir::ScalarType::F32), false,
             ir::Value::createRegister(inst.dst, ir::ScalarType::F32),
             inst.neg_c);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
