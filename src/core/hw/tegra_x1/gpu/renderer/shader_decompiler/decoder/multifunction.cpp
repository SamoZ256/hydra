#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/multifunction.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: op
void emitRro(DecoderContext& context, pred_t pred, bool pred_inv, reg_t dst,
             ir::Value src, bool abs, bool neg) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    const auto src_v = absNegIf(context.builder, src, abs, neg);

    // This should always be followed by a corresponding MUFU instruction,
    // so a simple copy should be sufficient
    context.builder.opCopy(ir::Value::createRegister(dst, ir::ScalarType::F32),
                           src_v);

    if (conditional)
        context.builder.opEndIf();
}

void emitMultifunction(DecoderContext& context, pred_t pred, bool pred_inv,
                       MultifunctionOp op, bool saturate, reg_t dst,
                       ir::Value src, bool abs, bool neg) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    const auto src_v = absNegIf(context.builder, src, abs, neg);

    ir::Value res = ir::Value::createUndefined();
    switch (op) {
    case MultifunctionOp::Cos:
        res = context.builder.opCos(src_v);
        break;
    case MultifunctionOp::Sin:
        res = context.builder.opSin(src_v);
        break;
    case MultifunctionOp::Ex2:
        res = context.builder.opExp2(src_v);
        break;
    case MultifunctionOp::Lg2:
        res = context.builder.opLog2(src_v);
        break;
    case MultifunctionOp::Rcp:
        res = context.builder.opReciprocal(src_v);
        break;
    case MultifunctionOp::Rsq:
        res = context.builder.opReciprocalSqrt(src_v);
        break;
    case MultifunctionOp::Rcp64h:
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Double reciprocal");
        break;
    case MultifunctionOp::Rsq64h:
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Double reciprocal square root");
        break;
    case MultifunctionOp::Sqrt:
        res = context.builder.opSqrt(src_v);
        break;
    }
    res = saturateIf(context.builder, res, saturate);
    context.builder.opCopy(ir::Value::createRegister(dst, ir::ScalarType::F32),
                           res);

    if (conditional)
        context.builder.opEndIf();
}

} // namespace

void emitRroR(DecoderContext& context, InstRroR inst) {
    emitRro(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
            ir::Value::createRegister(inst.src, ir::ScalarType::F32),
            inst.base.abs, inst.base.neg);
}

void emitRroC(DecoderContext& context, InstRroC inst) {
    emitRro(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4)),
            ir::ScalarType::F32),
        inst.base.abs, inst.base.neg);
}

void emitRroI(DecoderContext& context, InstRroI inst) {
    emitRro(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        ir::Value::createConstantF(getFloatImm20(inst.imm20_0, inst.imm20_19)),
        inst.base.abs, inst.base.neg);
}

void emitMufu(DecoderContext& context, InstMufu inst) {
    emitMultifunction(context, inst.pred, inst.pred_inv, inst.op, inst.sat,
                      inst.dst,
                      ir::Value::createRegister(inst.src, ir::ScalarType::F32),
                      inst.abs, inst.neg);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
