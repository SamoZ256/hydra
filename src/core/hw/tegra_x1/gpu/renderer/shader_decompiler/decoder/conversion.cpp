#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/conversion.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

DataType ToDataType(FloatFormat format) {
    switch (format) {
    case FloatFormat::F16:
        return DataType::F16;
    case FloatFormat::F32:
        return DataType::F32;
    case FloatFormat::F64:
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "F64");
        return DataType::F32;
    default:
        unreachable();
    }
}

// TODO: sh, ftz, write_cc
void EmitFloatToFloat(DecoderContext& context, pred_t pred, bool pred_inv,
                      RoundMode2 round_mode, bool saturate, reg_t dst,
                      FloatFormat dst_format, ir::Value src, bool abs,
                      bool neg) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_v = AbsNegIf(context.builder, src, abs, neg);
    switch (round_mode) {
    case RoundMode2::Pass:
        break;
    case RoundMode2::Round:
        src_v = context.builder.OpRound(src_v);
        break;
    case RoundMode2::Floor:
        src_v = context.builder.OpFloor(src_v);
        break;
    case RoundMode2::Ceil:
        src_v = context.builder.OpCeil(src_v);
        break;
    case RoundMode2::Trunc:
        src_v = context.builder.OpTrunc(src_v);
        break;
    }

    const auto dst_type = ToDataType(dst_format);
    auto res = context.builder.OpCast(src_v, dst_type);
    res = SaturateIf(context.builder, res, saturate);
    context.builder.OpCopy(ir::Value::Register(dst, dst_type), res);

    if (conditional)
        context.builder.OpEndIf();
}

DataType ToDataType(IntegerFormat format) {
    switch (format) {
    case IntegerFormat::U16:
        return DataType::U16;
    case IntegerFormat::U32:
        return DataType::U32;
    case IntegerFormat::U64:
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "U64");
        return DataType::U32;
    case IntegerFormat::S16:
        return DataType::I16;
    case IntegerFormat::S32:
        return DataType::I32;
    case IntegerFormat::S64:
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "I64");
        return DataType::I32;
    default:
        unreachable();
    }
}

// TODO: sh, ftz, write_cc
void EmitFloatToInt(DecoderContext& context, pred_t pred, bool pred_inv,
                    RoundMode3 round_mode, reg_t dst, IntegerFormat dst_format,
                    ir::Value src, bool abs, bool neg) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_v = AbsNegIf(context.builder, src, abs, neg);
    switch (round_mode) {
    case RoundMode3::Round:
        src_v = context.builder.OpRound(src_v);
        break;
    case RoundMode3::Floor:
        src_v = context.builder.OpFloor(src_v);
        break;
    case RoundMode3::Ceil:
        src_v = context.builder.OpCeil(src_v);
        break;
    case RoundMode3::Trunc:
        src_v = context.builder.OpTrunc(src_v);
        break;
    }

    const auto dst_type = ToDataType(dst_format);
    const auto res = context.builder.OpCast(src_v, dst_type);
    context.builder.OpCopy(ir::Value::Register(dst, dst_type), res);

    if (conditional)
        context.builder.OpEndIf();
}

DataType ToDataType(IntegerFormat2 format) {
    switch (format) {
    case IntegerFormat2::U8:
        return DataType::U8;
    case IntegerFormat2::U16:
        return DataType::U16;
    case IntegerFormat2::U32:
        return DataType::U32;
    case IntegerFormat2::S8:
        return DataType::I8;
    case IntegerFormat2::S16:
        return DataType::I16;
    case IntegerFormat2::S32:
        return DataType::I32;
    default:
        unreachable();
    }
}

// TODO: write_cc
void EmitIntToInt(DecoderContext& context, pred_t pred, bool pred_inv,
                  ByteSelect byte_sel, bool saturate, reg_t dst,
                  IntegerFormat2 dst_format, ir::Value src, bool abs,
                  bool neg) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_v = AbsNegIf(context.builder, src, abs, neg);
    // TODO: byte selection
    if (byte_sel != ByteSelect::B0)
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Byte selection");

    const auto dst_type = ToDataType(dst_format);
    auto res = context.builder.OpCast(src_v, dst_type);
    // TODO: saturate
    // res = IntegerSaturateIf(context.builder, dst_type, res, saturate);
    if (saturate)
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Saturation");
    context.builder.OpCopy(ir::Value::Register(dst, dst_type), res);

    if (conditional)
        context.builder.OpEndIf();
}

// TODO: round_mode, write_cc
void EmitIntToFloat(DecoderContext& context, pred_t pred, bool pred_inv,
                    ByteSelect byte_sel, reg_t dst, FloatFormat dst_format,
                    ir::Value src, bool abs, bool neg) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_v = AbsNegIf(context.builder, src, abs, neg);
    // TODO: byte selection
    if (byte_sel != ByteSelect::B0)
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Byte selection");

    const auto dst_type = ToDataType(dst_format);
    const auto res = context.builder.OpCast(src_v, dst_type);
    context.builder.OpCopy(ir::Value::Register(dst, dst_type), res);

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace

void EmitF2fR(DecoderContext& context, InstF2fR inst) {
    EmitFloatToFloat(
        context, inst.base.pred, inst.base.pred_inv, inst.base.GetRoundMode(),
        inst.base.sat, inst.base.dst, inst.base.dst_fmt,
        ir::Value::Register(inst.src, ToDataType(inst.base.src_fmt)),
        inst.base.abs, inst.base.neg);
}

void EmitF2fC(DecoderContext& context, InstF2fC inst) {
    EmitFloatToFloat(
        context, inst.base.pred, inst.base.pred_inv, inst.base.GetRoundMode(),
        inst.base.sat, inst.base.dst, inst.base.dst_fmt,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               ToDataType(inst.base.src_fmt)),
        inst.base.abs, inst.base.neg);
}

void EmitF2fI(DecoderContext& context, InstF2fI inst) {
    EmitFloatToFloat(
        context, inst.base.pred, inst.base.pred_inv, inst.base.GetRoundMode(),
        inst.base.sat, inst.base.dst, inst.base.dst_fmt,
        ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                             ToDataType(inst.base.src_fmt)),
        inst.base.abs, inst.base.neg);
}

void EmitF2iR(DecoderContext& context, InstF2iR inst) {
    EmitFloatToInt(context, inst.base.pred, inst.base.pred_inv,
                   inst.base.round_mode, inst.base.dst, inst.base.GetDstFmt(),
                   ir::Value::Register(inst.src, ToDataType(inst.base.src_fmt)),
                   inst.base.abs, inst.base.neg);
}

void EmitF2iC(DecoderContext& context, InstF2iC inst) {
    EmitFloatToInt(
        context, inst.base.pred, inst.base.pred_inv, inst.base.round_mode,
        inst.base.dst, inst.base.GetDstFmt(),
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               ToDataType(inst.base.src_fmt)),
        inst.base.abs, inst.base.neg);
}

void EmitF2iI(DecoderContext& context, InstF2iI inst) {
    EmitFloatToInt(
        context, inst.base.pred, inst.base.pred_inv, inst.base.round_mode,
        inst.base.dst, inst.base.GetDstFmt(),
        ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                             ToDataType(inst.base.src_fmt)),
        inst.base.abs, inst.base.neg);
}

void EmitI2iR(DecoderContext& context, InstI2iR inst) {
    EmitIntToInt(
        context, inst.base.pred, inst.base.pred_inv, inst.base.byte_sel,
        inst.base.sat, inst.base.dst, inst.base.GetDstFmt(),
        ir::Value::Register(inst.src, ToDataType(inst.base.GetSrcFmt())),
        inst.base.abs, inst.base.neg);
}

void EmitI2iC(DecoderContext& context, InstI2iC inst) {
    EmitIntToInt(
        context, inst.base.pred, inst.base.pred_inv, inst.base.byte_sel,
        inst.base.sat, inst.base.dst, inst.base.GetDstFmt(),
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               ToDataType(inst.base.GetSrcFmt())),
        inst.base.abs, inst.base.neg);
}

void EmitI2iI(DecoderContext& context, InstI2iI inst) {
    EmitIntToInt(
        context, inst.base.pred, inst.base.pred_inv, inst.base.byte_sel,
        inst.base.sat, inst.base.dst, inst.base.GetDstFmt(),
        ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                             ToDataType(inst.base.GetSrcFmt())),
        inst.base.abs, inst.base.neg);
}

void EmitI2fR(DecoderContext& context, InstI2fR inst) {
    EmitIntToFloat(
        context, inst.base.pred, inst.base.pred_inv, inst.base.byte_sel,
        inst.base.dst, inst.base.dst_fmt,
        ir::Value::Register(inst.src, ToDataType(inst.base.GetSrcFmt())),
        inst.base.abs, inst.base.neg);
}

void EmitI2fC(DecoderContext& context, InstI2fC inst) {
    EmitIntToFloat(
        context, inst.base.pred, inst.base.pred_inv, inst.base.byte_sel,
        inst.base.dst, inst.base.dst_fmt,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               ToDataType(inst.base.GetSrcFmt())),
        inst.base.abs, inst.base.neg);
}

void EmitI2fI(DecoderContext& context, InstI2fI inst) {
    EmitIntToFloat(
        context, inst.base.pred, inst.base.pred_inv, inst.base.byte_sel,
        inst.base.dst, inst.base.dst_fmt,
        ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                             ToDataType(inst.base.GetSrcFmt())),
        inst.base.abs, inst.base.neg);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
