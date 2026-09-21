#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/shift.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: m, x
void emitShiftLeft(DecoderContext& context, pred_t pred, bool pred_inv,
                   reg_t dst, reg_t src_a, ir::Value src_b) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto res =
        context.builder.opShiftLeft(ir::Value::createRegister(src_a), src_b);
    context.builder.opCopy(ir::Value::createRegister(dst), res);

    if (conditional)
        context.builder.opEndIf();
}

// TODO: m, brev, x_mode, is_signed
void emitShiftRight(DecoderContext& context, pred_t pred, bool pred_inv,
                    reg_t dst, reg_t src_a, ir::Value src_b) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    auto res =
        context.builder.opShiftRight(ir::Value::createRegister(src_a), src_b);
    context.builder.opCopy(ir::Value::createRegister(dst), res);

    if (conditional)
        context.builder.opEndIf();
}

} // namespace

void emitShlR(DecoderContext& context, InstShlR inst) {
    emitShiftLeft(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
                  inst.base.src_a, ir::Value::createRegister(inst.src_b));
}

void emitShlC(DecoderContext& context, InstShlC inst) {
    emitShiftLeft(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        inst.base.src_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4))));
}

void emitShlI(DecoderContext& context, InstShlI inst) {
    emitShiftLeft(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
                  inst.base.src_a,
                  ir::Value::createConstantU(
                      getIntImm20(inst.imm20_0, inst.imm20_19, true)));
}

void emitShrR(DecoderContext& context, InstShrR inst) {
    emitShiftRight(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
                   inst.base.src_a, ir::Value::createRegister(inst.src_b));
}

void emitShrC(DecoderContext& context, InstShrC inst) {
    emitShiftRight(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        inst.base.src_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4))));
}

void emitShrI(DecoderContext& context, InstShrI inst) {
    emitShiftRight(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
                   inst.base.src_a,
                   ir::Value::createConstantU(
                       getIntImm20(inst.imm20_0, inst.imm20_19, true)));
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
