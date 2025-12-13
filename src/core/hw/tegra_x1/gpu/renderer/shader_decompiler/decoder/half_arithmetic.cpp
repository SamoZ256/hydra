#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/half_arithmetic.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

ir::Value GetSwizzledHalf(ir::Builder& builder, HalfSwizzle swizzle,
                          reg_t src) {
    // TODO: implement
    if (swizzle != HalfSwizzle::F16)
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Half swizzle");

    return ir::Value::Register(src, DataType::F16X2);
}

void CopyHalfOutput(ir::Builder& builder, HalfOutputFormat format, reg_t dst,
                    ir::Value value) {
    // TODO: implement
    if (format != HalfOutputFormat::F16)
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Half output format");

    builder.OpCopy(ir::Value::Register(dst, DataType::F16X2), value);
}

// TODO: ftz
void EmitHadd2(DecoderContext& context, pred_t pred, bool pred_inv,
               bool saturate, HalfOutputFormat out_format, reg_t dst,
               reg_t src_a, HalfSwizzle swizzle_a, bool abs_a, bool neg_a,
               ir::Value src_b, bool abs_b, bool neg_b) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = AbsNegIf(context.builder,
                            GetSwizzledHalf(context.builder, swizzle_a, src_a),
                            abs_a, neg_a);
    auto src_b_v = AbsNegIf(context.builder, src_b, abs_b, neg_b);

    auto res = context.builder.OpAdd(src_a_v, src_b_v);
    res = SaturateIf(context.builder, res, saturate);
    CopyHalfOutput(context.builder, out_format, dst, res);

    if (conditional)
        context.builder.OpEndIf();
}

// TODO: fmz
void EmitHmul2(DecoderContext& context, pred_t pred, bool pred_inv,
               bool saturate, HalfOutputFormat out_format, reg_t dst,
               reg_t src_a, HalfSwizzle swizzle_a, bool abs_a, bool neg_a,
               ir::Value src_b, bool abs_b, bool neg_b) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = AbsNegIf(context.builder,
                            GetSwizzledHalf(context.builder, swizzle_a, src_a),
                            abs_a, neg_a);
    auto src_b_v = AbsNegIf(context.builder, src_b, abs_b, neg_b);

    auto res = context.builder.OpMultiply(src_a_v, src_b_v);
    res = SaturateIf(context.builder, res, saturate);
    CopyHalfOutput(context.builder, out_format, dst, res);

    if (conditional)
        context.builder.OpEndIf();
}

// TODO: fmz
void EmitHfma2(DecoderContext& context, pred_t pred, bool pred_inv,
               bool saturate, HalfOutputFormat out_format, reg_t dst,
               reg_t src_a, HalfSwizzle swizzle_a, bool neg_a, ir::Value src_b,
               bool neg_b, ir::Value src_c, bool neg_c) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v =
        NegIf(context.builder,
              GetSwizzledHalf(context.builder, swizzle_a, src_a), neg_a);
    auto src_b_v = NegIf(context.builder, src_b, neg_b);
    auto src_c_v = NegIf(context.builder, src_c, neg_c);

    auto res = context.builder.OpFma(src_a_v, src_b_v, src_c_v);
    res = SaturateIf(context.builder, res, saturate);
    CopyHalfOutput(context.builder, out_format, dst, res);

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace

void EmitHadd2R(DecoderContext& context, InstHadd2R inst) {
    EmitHadd2(context, inst.base.pred, inst.base.pred_inv, inst.sat,
              inst.base.out_fmt, inst.base.dst, inst.base.src_a,
              inst.base.swizzle_a, inst.base.abs_a, inst.neg_a,
              GetSwizzledHalf(context.builder, inst.swizzle_b, inst.src_b),
              inst.abs_b, inst.neg_b);
}

void EmitHadd2C(DecoderContext& context, InstHadd2C inst) {
    EmitHadd2(
        context, inst.base.pred, inst.base.pred_inv, inst.sat,
        inst.base.out_fmt, inst.base.dst, inst.base.src_a, inst.base.swizzle_a,
        inst.base.abs_a, inst.neg_a,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               DataType::F16X2),
        inst.abs_b, inst.neg_b);
}

void EmitHadd2I(DecoderContext& context, InstHadd2I inst) {
    // TODO: correct?
    EmitHadd2(
        context, inst.base.pred, inst.base.pred_inv, inst.sat,
        inst.base.out_fmt, inst.base.dst, inst.base.src_a, inst.base.swizzle_a,
        inst.base.abs_a, inst.neg_a,
        context.builder.OpVectorConstruct(
            DataType::F16,
            {ir::Value::Immediate(inst.h0_imm10 << 6, DataType::F16),
             ir::Value::Immediate(inst.GetH1Imm10() << 6, DataType::F16)}),
        false, false);
}

void EmitHadd2_32I(DecoderContext& context, InstHadd2_32I inst) {
    EmitHadd2(context, inst.pred, inst.pred_inv, inst.sat,
              HalfOutputFormat::F16, inst.dst, inst.src_a, inst.swizzle_a,
              false, inst.neg_a,
              ir::Value::Immediate(inst.imm, DataType::F16X2), false, false);
}

void EmitHmul2R(DecoderContext& context, InstHmul2R inst) {
    EmitHmul2(context, inst.base.pred, inst.base.pred_inv, inst.sat,
              inst.base.out_fmt, inst.base.dst, inst.base.src_a,
              inst.base.swizzle_a, inst.base.abs_a, false,
              GetSwizzledHalf(context.builder, inst.swizzle_b, inst.src_b),
              inst.abs_b, inst.neg_b);
}

void EmitHmul2C(DecoderContext& context, InstHmul2C inst) {
    EmitHmul2(
        context, inst.base.pred, inst.base.pred_inv, inst.sat,
        inst.base.out_fmt, inst.base.dst, inst.base.src_a, inst.base.swizzle_a,
        inst.base.abs_a, inst.neg_a,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               DataType::F16X2),
        inst.abs_b, false);
}

void EmitHmul2I(DecoderContext& context, InstHmul2I inst) {
    // TODO: correct?
    EmitHmul2(
        context, inst.base.pred, inst.base.pred_inv, inst.sat,
        inst.base.out_fmt, inst.base.dst, inst.base.src_a, inst.base.swizzle_a,
        inst.base.abs_a, inst.neg_a,
        context.builder.OpVectorConstruct(
            DataType::F16,
            {ir::Value::Immediate(inst.h0_imm10 << 6, DataType::F16),
             ir::Value::Immediate(inst.GetH1Imm10() << 6, DataType::F16)}),
        false, false);
}

void EmitHmul2_32I(DecoderContext& context, InstHmul2_32I inst) {
    EmitHmul2(context, inst.pred, inst.pred_inv, inst.sat,
              HalfOutputFormat::F16, inst.dst, inst.src_a, inst.swizzle_a,
              false, false, ir::Value::Immediate(inst.imm, DataType::F16X2),
              false, false);
}

void EmitHfma2R(DecoderContext& context, InstHfma2R inst) {
    EmitHfma2(context, inst.base.pred, inst.base.pred_inv, inst.sat,
              inst.base.out_fmt, inst.base.dst, inst.base.src_a,
              inst.base.swizzle_a, false,
              GetSwizzledHalf(context.builder, inst.swizzle_b, inst.src_b),
              inst.neg_b,
              GetSwizzledHalf(context.builder, inst.swizzle_c, inst.src_c),
              inst.neg_c);
}

void EmitHfma2RC(DecoderContext& context, InstHfma2RC inst) {
    EmitHfma2(
        context, inst.base.pred, inst.base.pred_inv, inst.sat,
        inst.base.out_fmt, inst.base.dst, inst.base.src_a, inst.base.swizzle_a,
        false, GetSwizzledHalf(context.builder, inst.swizzle_b, inst.src_b),
        inst.neg_b,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               DataType::F16X2),
        inst.neg_c);
}

void EmitHfma2C(DecoderContext& context, InstHfma2C inst) {
    EmitHfma2(
        context, inst.base.pred, inst.base.pred_inv, inst.sat,
        inst.base.out_fmt, inst.base.dst, inst.base.src_a, inst.base.swizzle_a,
        false,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               DataType::F16X2),
        inst.neg_b,
        GetSwizzledHalf(context.builder, inst.swizzle_c, inst.src_c),
        inst.neg_c);
}

void EmitHfma2I(DecoderContext& context, InstHfma2I inst) {
    // TODO: correct?
    EmitHfma2(
        context, inst.base.pred, inst.base.pred_inv, inst.sat,
        inst.base.out_fmt, inst.base.dst, inst.base.src_a, inst.base.swizzle_a,
        false,
        context.builder.OpVectorConstruct(
            DataType::F16,
            {ir::Value::Immediate(inst.h0_imm10 << 6, DataType::F16),
             ir::Value::Immediate(inst.GetH1Imm10() << 6, DataType::F16)}),
        false, GetSwizzledHalf(context.builder, inst.swizzle_c, inst.src_c),
        inst.neg_c);
}

void EmitHfma2_32I(DecoderContext& context, InstHfma2_32I inst) {
    EmitHfma2(context, inst.pred, inst.pred_inv, false, HalfOutputFormat::F16,
              inst.dst, inst.src_a, inst.swizzle_a, false,
              ir::Value::Immediate(inst.imm, DataType::F16X2), false,
              ir::Value::Register(inst.dst, DataType::F16X2), inst.neg_c);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
