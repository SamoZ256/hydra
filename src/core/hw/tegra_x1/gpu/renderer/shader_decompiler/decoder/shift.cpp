#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/shift.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: m, x
void EmitShiftLeft(DecoderContext& context, pred_t pred, bool pred_inv,
                   reg_t dst, reg_t src_a, ir::Value src_b) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto res = context.builder.OpShiftLeft(ir::Value::Register(src_a), src_b);
    context.builder.OpCopy(ir::Value::Register(dst), res);

    if (conditional)
        context.builder.OpEndIf();
}

// TODO: m, brev, x_mode, is_signed
void EmitShiftRight(DecoderContext& context, pred_t pred, bool pred_inv,
                    reg_t dst, reg_t src_a, ir::Value src_b) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto res = context.builder.OpShiftRight(ir::Value::Register(src_a), src_b);
    context.builder.OpCopy(ir::Value::Register(dst), res);

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace

void EmitShlR(DecoderContext& context, InstShlR inst) {
    EmitShiftLeft(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
                  inst.base.src_a, ir::Value::Register(inst.src_b));
}

void EmitShlC(DecoderContext& context, InstShlC inst) {
    EmitShiftLeft(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        inst.base.src_a,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4)));
}

void EmitShlI(DecoderContext& context, InstShlI inst) {
    EmitShiftLeft(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        inst.base.src_a,
        ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12));
}

void EmitShrR(DecoderContext& context, InstShrR inst) {
    EmitShiftRight(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
                   inst.base.src_a, ir::Value::Register(inst.src_b));
}

void EmitShrC(DecoderContext& context, InstShrC inst) {
    EmitShiftRight(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        inst.base.src_a,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4)));
}

void EmitShrI(DecoderContext& context, InstShrI inst) {
    EmitShiftRight(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        inst.base.src_a,
        ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12));
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
