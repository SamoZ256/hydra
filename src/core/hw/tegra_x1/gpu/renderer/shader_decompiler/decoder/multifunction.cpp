#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/multifunction.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: op
void EmitRro(DecoderContext& context, pred_t pred, bool pred_inv, reg_t dst,
             ir::Value src, bool abs, bool neg) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    const auto src_v = AbsNegIf(context.builder, src, abs, neg);

    // This should always be followed by a corresponding MUFU instruction,
    // so a simple copy should be sufficient
    context.builder.OpCopy(ir::Value::Register(dst, DataType::F32), src_v);

    if (conditional)
        context.builder.OpEndIf();
}

void EmitMultifunction(DecoderContext& context, pred_t pred, bool pred_inv,
                       MultifunctionOp op, bool saturate, reg_t dst,
                       ir::Value src, bool abs, bool neg) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    const auto src_v = AbsNegIf(context.builder, src, abs, neg);

    ir::Value res = ir::Value::Undefined();
    switch (op) {
    case MultifunctionOp::Cos:
        res = context.builder.OpCos(src_v);
        break;
    case MultifunctionOp::Sin:
        res = context.builder.OpSin(src_v);
        break;
    case MultifunctionOp::Ex2:
        res = context.builder.OpExp2(src_v);
        break;
    case MultifunctionOp::Lg2:
        res = context.builder.OpLog2(src_v);
        break;
    case MultifunctionOp::Rcp:
        res = context.builder.OpReciprocal(src_v);
        break;
    case MultifunctionOp::Rsq:
        res = context.builder.OpReciprocalSqrt(src_v);
        break;
    case MultifunctionOp::Rcp64h:
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Double reciprocal");
        break;
    case MultifunctionOp::Rsq64h:
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Double reciprocal square root");
        break;
    case MultifunctionOp::Sqrt:
        res = context.builder.OpSqrt(src_v);
        break;
    }
    res = SaturateIf(context.builder, res, saturate);
    context.builder.OpCopy(ir::Value::Register(dst, DataType::F32), res);

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace

void EmitRroR(DecoderContext& context, InstRroR inst) {
    EmitRro(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
            ir::Value::Register(inst.src, DataType::F32), inst.base.abs,
            inst.base.neg);
}

void EmitRroC(DecoderContext& context, InstRroC inst) {
    EmitRro(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
            ir::Value::ConstMemory(
                CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4), DataType::F32),
            inst.base.abs, inst.base.neg);
}

void EmitRroI(DecoderContext& context, InstRroI inst) {
    EmitRro(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
            ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                                 DataType::F32),
            inst.base.abs, inst.base.neg);
}

void EmitMufu(DecoderContext& context, InstMufu inst) {
    EmitMultifunction(context, inst.pred, inst.pred_inv, inst.op, inst.sat,
                      inst.dst, ir::Value::Register(inst.src, DataType::F32),
                      inst.abs, inst.neg);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
