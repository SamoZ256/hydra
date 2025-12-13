#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/float_arithmetic.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: round_mode, ftz, write_cc
void EmitFadd(DecoderContext& context, pred_t pred, bool pred_inv,
              bool saturate, reg_t dst, reg_t src_a, bool abs_a, bool neg_a,
              ir::Value src_b, bool abs_b, bool neg_b) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v =
        AbsNegIf(context.builder, ir::Value::Register(src_a, DataType::F32),
                 abs_a, neg_a);
    auto src_b_v = AbsNegIf(context.builder, src_b, abs_b, neg_b);

    auto res = context.builder.OpAdd(src_a_v, src_b_v);
    res = SaturateIf(context.builder, res, saturate);
    context.builder.OpCopy(ir::Value::Register(dst, DataType::F32), res);

    if (conditional)
        context.builder.OpEndIf();
}

// TODO: round_mode, fmz, write_cc
void EmitFmul(DecoderContext& context, pred_t pred, bool pred_inv,
              MultiplyScale scale, bool saturate, reg_t dst, reg_t src_a,
              ir::Value src_b, bool neg_b) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = ir::Value::Register(src_a, DataType::F32);
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
        src_a_v = context.builder.OpMultiply(
            src_a_v,
            ir::Value::Immediate(std::bit_cast<u32>(scale_f), DataType::F32));
    }

    auto res = context.builder.OpMultiply(src_a_v,
                                          NegIf(context.builder, src_b, neg_b));
    res = SaturateIf(context.builder, res, saturate);
    context.builder.OpCopy(ir::Value::Register(dst, DataType::F32), res);

    if (conditional)
        context.builder.OpEndIf();
}

// TODO: round_mode, ftz, write_cc
void EmitFfma(DecoderContext& context, pred_t pred, bool pred_inv,
              bool saturate, reg_t dst, reg_t src_a, bool neg_a,
              ir::Value src_b, bool neg_b, ir::Value src_c, bool neg_c) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = NegIf(context.builder,
                         ir::Value::Register(src_a, DataType::F32), neg_a);
    auto src_b_v = NegIf(context.builder, src_b, neg_b);
    auto src_c_v = NegIf(context.builder, src_c, neg_c);

    auto res = context.builder.OpFma(src_a_v, src_b_v, src_c_v);
    res = SaturateIf(context.builder, res, saturate);
    context.builder.OpCopy(ir::Value::Register(dst, DataType::F32), res);

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace

void EmitFaddR(DecoderContext& context, InstFaddR inst) {
    EmitFadd(context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
             inst.base.dst, inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
             ir::Value::Register(inst.src_b, DataType::F32), inst.base.abs_b,
             inst.base.neg_b);
}

void EmitFaddC(DecoderContext& context, InstFaddC inst) {
    EmitFadd(context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
             inst.base.dst, inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
             ir::Value::ConstMemory(
                 CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4), DataType::F32),
             inst.base.abs_b, inst.base.neg_b);
}

void EmitFaddI(DecoderContext& context, InstFaddI inst) {
    EmitFadd(context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
             inst.base.dst, inst.base.src_a, inst.base.abs_a, inst.base.neg_a,
             ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                                  DataType::F32),
             inst.base.abs_b, inst.base.neg_b);
}

void EmitFadd32I(DecoderContext& context, InstFadd32I inst) {
    EmitFadd(context, inst.pred, inst.pred_inv, false, inst.dst, inst.src_a,
             inst.abs_a, inst.neg_a,
             ir::Value::Immediate(inst.imm, DataType::F32), inst.abs_b,
             inst.neg_b);
}

void EmitFmulR(DecoderContext& context, InstFmulR inst) {
    EmitFmul(context, inst.base.pred, inst.base.pred_inv, inst.base.scale,
             inst.base.sat, inst.base.dst, inst.base.src_a,
             ir::Value::Register(inst.src_b, DataType::F32), inst.base.neg_b);
}

void EmitFmulC(DecoderContext& context, InstFmulC inst) {
    EmitFmul(context, inst.base.pred, inst.base.pred_inv, inst.base.scale,
             inst.base.sat, inst.base.dst, inst.base.src_a,
             ir::Value::ConstMemory(
                 CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4), DataType::F32),
             inst.base.neg_b);
}

void EmitFmulI(DecoderContext& context, InstFmulI inst) {
    EmitFmul(context, inst.base.pred, inst.base.pred_inv, inst.base.scale,
             inst.base.sat, inst.base.dst, inst.base.src_a,
             ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                                  DataType::F32),
             inst.base.neg_b);
}

void EmitFmul32I(DecoderContext& context, InstFmul32I inst) {
    EmitFmul(context, inst.pred, inst.pred_inv, MultiplyScale::None, inst.sat,
             inst.dst, inst.src_a,
             ir::Value::Immediate(inst.imm, DataType::F32), false);
}

void EmitFfmaR(DecoderContext& context, InstFfmaR inst) {
    EmitFfma(context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
             inst.base.dst, inst.base.src_a, false,
             ir::Value::Register(inst.src_b, DataType::F32), inst.base.neg_b,
             ir::Value::Register(inst.src_c, DataType::F32), inst.base.neg_c);
}

void EmitFfmaRC(DecoderContext& context, InstFfmaRC inst) {
    EmitFfma(context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
             inst.base.dst, inst.base.src_a, false,
             ir::Value::Register(inst.src_b, DataType::F32), inst.base.neg_b,
             ir::Value::ConstMemory(
                 CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4), DataType::F32),
             inst.base.neg_c);
}

void EmitFfmaC(DecoderContext& context, InstFfmaC inst) {
    EmitFfma(context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
             inst.base.dst, inst.base.src_a, false,
             ir::Value::ConstMemory(
                 CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4), DataType::F32),
             inst.base.neg_b, ir::Value::Register(inst.src_c, DataType::F32),
             inst.base.neg_c);
}

void EmitFfmaI(DecoderContext& context, InstFfmaI inst) {
    EmitFfma(context, inst.base.pred, inst.base.pred_inv, inst.base.sat,
             inst.base.dst, inst.base.src_a, false,
             ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                                  DataType::F32),
             inst.base.neg_b, ir::Value::Register(inst.src_c, DataType::F32),
             inst.base.neg_c);
}

void EmitFfma32I(DecoderContext& context, InstFfma32I inst) {
    EmitFfma(context, inst.pred, inst.pred_inv, inst.sat, inst.dst, inst.src_a,
             inst.neg_a, ir::Value::Immediate(inst.imm, DataType::F32), false,
             ir::Value::Register(inst.dst, DataType::F32), inst.neg_c);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
