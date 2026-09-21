#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/conversion.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

ir::ScalarType toType(FloatFormat format) {
    switch (format) {
    case FloatFormat::F16:
        return ir::ScalarType::F16;
    case FloatFormat::F32:
        return ir::ScalarType::F32;
    case FloatFormat::F64:
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "F64");
        return ir::ScalarType::F32;
    default:
        unreachable();
    }
}

// TODO: sh, ftz, write_cc
void emitFloatToFloat(DecoderContext& context, pred_t pred, bool pred_inv,
                      RoundMode2 round_mode, bool saturate, reg_t dst,
                      FloatFormat dst_format, ir::Value src, bool abs,
                      bool neg) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_v = absNegIf(context.builder, src, abs, neg);
    switch (round_mode) {
    case RoundMode2::Pass:
        break;
    case RoundMode2::Round:
        src_v = context.builder.opRound(src_v);
        break;
    case RoundMode2::Floor:
        src_v = context.builder.opFloor(src_v);
        break;
    case RoundMode2::Ceil:
        src_v = context.builder.opCeil(src_v);
        break;
    case RoundMode2::Trunc:
        src_v = context.builder.opTrunc(src_v);
        break;
    }

    const auto dst_type = toType(dst_format);
    auto res = context.builder.opCast(src_v, dst_type);
    res = saturateIf(context.builder, res, saturate);
    context.builder.opCopy(ir::Value::createRegister(dst, dst_type), res);

    if (conditional)
        context.builder.opEndIf();
}

ir::ScalarType toType(IntegerFormat format) {
    switch (format) {
    case IntegerFormat::U16:
        return ir::ScalarType::U16;
    case IntegerFormat::U32:
        return ir::ScalarType::U32;
    case IntegerFormat::U64:
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "U64");
        return ir::ScalarType::U32;
    case IntegerFormat::S16:
        return ir::ScalarType::I16;
    case IntegerFormat::S32:
        return ir::ScalarType::I32;
    case IntegerFormat::S64:
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "I64");
        return ir::ScalarType::I32;
    default:
        unreachable();
    }
}

// TODO: sh, ftz, write_cc
void emitFloatToInt(DecoderContext& context, pred_t pred, bool pred_inv,
                    RoundMode3 round_mode, reg_t dst, IntegerFormat dst_format,
                    ir::Value src, bool abs, bool neg) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_v = absNegIf(context.builder, src, abs, neg);
    switch (round_mode) {
    case RoundMode3::Round:
        src_v = context.builder.opRound(src_v);
        break;
    case RoundMode3::Floor:
        src_v = context.builder.opFloor(src_v);
        break;
    case RoundMode3::Ceil:
        src_v = context.builder.opCeil(src_v);
        break;
    case RoundMode3::Trunc:
        src_v = context.builder.opTrunc(src_v);
        break;
    }

    const auto dst_type = toType(dst_format);
    const auto res = context.builder.opCast(src_v, dst_type);
    context.builder.opCopy(ir::Value::createRegister(dst, dst_type), res);

    if (conditional)
        context.builder.opEndIf();
}

ir::ScalarType toType(IntegerFormat2 format) {
    switch (format) {
    case IntegerFormat2::U8:
        return ir::ScalarType::U8;
    case IntegerFormat2::U16:
        return ir::ScalarType::U16;
    case IntegerFormat2::U32:
        return ir::ScalarType::U32;
    case IntegerFormat2::S8:
        return ir::ScalarType::I8;
    case IntegerFormat2::S16:
        return ir::ScalarType::I16;
    case IntegerFormat2::S32:
        return ir::ScalarType::I32;
    default:
        unreachable();
    }
}

// TODO: write_cc
void emitIntToInt(DecoderContext& context, pred_t pred, bool pred_inv,
                  ByteSelect byte_sel, bool saturate, reg_t dst,
                  IntegerFormat2 dst_format, ir::Value src, bool abs,
                  bool neg) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_v = absNegIf(context.builder, src, abs, neg);
    // TODO: byte selection
    if (byte_sel != ByteSelect::B0)
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Byte selection");

    const auto dst_type = toType(dst_format);
    auto res = context.builder.opCast(src_v, dst_type);
    // TODO: saturate
    // res = IntegerSaturateIf(context.builder, dst_type, res, saturate);
    if (saturate)
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Saturation");
    context.builder.opCopy(ir::Value::createRegister(dst, dst_type), res);

    if (conditional)
        context.builder.opEndIf();
}

// TODO: round_mode, write_cc
void emitIntToFloat(DecoderContext& context, pred_t pred, bool pred_inv,
                    ByteSelect byte_sel, reg_t dst, FloatFormat dst_format,
                    ir::Value src, bool abs, bool neg) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto src_v = absNegIf(context.builder, src, abs, neg);
    // TODO: byte selection
    if (byte_sel != ByteSelect::B0)
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Byte selection");

    const auto dst_type = toType(dst_format);
    const auto res = context.builder.opCast(src_v, dst_type);
    context.builder.opCopy(ir::Value::createRegister(dst, dst_type), res);

    if (conditional)
        context.builder.opEndIf();
}

} // namespace

void emitF2fR(DecoderContext& context, InstF2fR inst) {
    emitFloatToFloat(
        context, inst.base.pred, inst.base.pred_inv, inst.base.getRoundMode(),
        inst.base.sat, inst.base.dst, inst.base.dst_fmt,
        ir::Value::createRegister(inst.src, toType(inst.base.src_fmt)),
        inst.base.abs, inst.base.neg);
}

void emitF2fC(DecoderContext& context, InstF2fC inst) {
    emitFloatToFloat(
        context, inst.base.pred, inst.base.pred_inv, inst.base.getRoundMode(),
        inst.base.sat, inst.base.dst, inst.base.dst_fmt,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            toType(inst.base.src_fmt)),
        inst.base.abs, inst.base.neg);
}

void emitF2fI(DecoderContext& context, InstF2fI inst) {
    emitFloatToFloat(
        context, inst.base.pred, inst.base.pred_inv, inst.base.getRoundMode(),
        inst.base.sat, inst.base.dst, inst.base.dst_fmt,
        ir::Value::createConstant((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                                  toType(inst.base.src_fmt)),
        inst.base.abs, inst.base.neg);
}

void emitF2iR(DecoderContext& context, InstF2iR inst) {
    emitFloatToInt(
        context, inst.base.pred, inst.base.pred_inv, inst.base.round_mode,
        inst.base.dst, inst.base.getDstFmt(),
        ir::Value::createRegister(inst.src, toType(inst.base.src_fmt)),
        inst.base.abs, inst.base.neg);
}

void emitF2iC(DecoderContext& context, InstF2iC inst) {
    emitFloatToInt(
        context, inst.base.pred, inst.base.pred_inv, inst.base.round_mode,
        inst.base.dst, inst.base.getDstFmt(),
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            toType(inst.base.src_fmt)),
        inst.base.abs, inst.base.neg);
}

void emitF2iI(DecoderContext& context, InstF2iI inst) {
    emitFloatToInt(
        context, inst.base.pred, inst.base.pred_inv, inst.base.round_mode,
        inst.base.dst, inst.base.getDstFmt(),
        ir::Value::createConstant((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                                  toType(inst.base.src_fmt)),
        inst.base.abs, inst.base.neg);
}

void emitI2iR(DecoderContext& context, InstI2iR inst) {
    emitIntToInt(
        context, inst.base.pred, inst.base.pred_inv, inst.base.byte_sel,
        inst.base.sat, inst.base.dst, inst.base.getDstFmt(),
        ir::Value::createRegister(inst.src, toType(inst.base.getSrcFmt())),
        inst.base.abs, inst.base.neg);
}

void emitI2iC(DecoderContext& context, InstI2iC inst) {
    emitIntToInt(
        context, inst.base.pred, inst.base.pred_inv, inst.base.byte_sel,
        inst.base.sat, inst.base.dst, inst.base.getDstFmt(),
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            toType(inst.base.getSrcFmt())),
        inst.base.abs, inst.base.neg);
}

void emitI2iI(DecoderContext& context, InstI2iI inst) {
    emitIntToInt(context, inst.base.pred, inst.base.pred_inv,
                 inst.base.byte_sel, inst.base.sat, inst.base.dst,
                 inst.base.getDstFmt(),
                 ir::Value::createConstant(
                     getIntImm20(inst.imm20_0, inst.imm20_19, true),
                     toType(inst.base.getSrcFmt())),
                 inst.base.abs, inst.base.neg);
}

void emitI2fR(DecoderContext& context, InstI2fR inst) {
    emitIntToFloat(
        context, inst.base.pred, inst.base.pred_inv, inst.base.byte_sel,
        inst.base.dst, inst.base.dst_fmt,
        ir::Value::createRegister(inst.src, toType(inst.base.getSrcFmt())),
        inst.base.abs, inst.base.neg);
}

void emitI2fC(DecoderContext& context, InstI2fC inst) {
    emitIntToFloat(
        context, inst.base.pred, inst.base.pred_inv, inst.base.byte_sel,
        inst.base.dst, inst.base.dst_fmt,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            toType(inst.base.getSrcFmt())),
        inst.base.abs, inst.base.neg);
}

void emitI2fI(DecoderContext& context, InstI2fI inst) {
    emitIntToFloat(context, inst.base.pred, inst.base.pred_inv,
                   inst.base.byte_sel, inst.base.dst, inst.base.dst_fmt,
                   ir::Value::createConstant(
                       getIntImm20(inst.imm20_0, inst.imm20_19, true),
                       toType(inst.base.getSrcFmt())),
                   inst.base.abs, inst.base.neg);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
