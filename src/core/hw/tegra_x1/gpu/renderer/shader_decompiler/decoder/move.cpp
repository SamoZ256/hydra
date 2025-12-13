#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/move.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: mask
void EmitMove(DecoderContext& context, pred_t pred, bool pred_inv, reg_t dst,
              ir::Value src) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    context.builder.OpCopy(ir::Value::Register(dst), src);

    if (conditional)
        context.builder.OpEndIf();
}

void EmitSelect(DecoderContext& context, pred_t pred, bool pred_inv, reg_t dst,
                pred_t src_pred, bool src_pred_inv, reg_t src_a,
                ir::Value src_b) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    const auto src_pred_v =
        NotIf(context.builder, ir::Value::Predicate(src_pred), src_pred_inv);
    const auto res =
        context.builder.OpSelect(src_pred_v, ir::Value::Register(src_a), src_b);
    context.builder.OpCopy(ir::Value::Register(dst), res);

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace

void EmitMovR(DecoderContext& context, InstMovR inst) {
    EmitMove(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
             ir::Value::Register(inst.src));
}

void EmitMovC(DecoderContext& context, InstMovC inst) {
    EmitMove(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4)));
}

void EmitMovI(DecoderContext& context, InstMovI inst) {
    EmitMove(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12));
}

void EmitMov32I(DecoderContext& context, InstMov32I inst) {
    EmitMove(context, inst.pred, inst.pred_inv, inst.dst,
             ir::Value::Immediate(inst.imm));
}

void EmitSelR(DecoderContext& context, InstSelR inst) {
    EmitSelect(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
               inst.base.src_pred, inst.base.src_pred_inv, inst.base.src_a,
               ir::Value::Register(inst.src_b));
}

void EmitSelC(DecoderContext& context, InstSelC inst) {
    EmitSelect(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        inst.base.src_pred, inst.base.src_pred_inv, inst.base.src_a,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4)));
}

void EmitSelI(DecoderContext& context, InstSelI inst) {
    EmitSelect(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
               inst.base.src_pred, inst.base.src_pred_inv, inst.base.src_a,
               ir::Value::Immediate(std::bit_cast<u32>(sign_extend<i32, 20>(
                   inst.imm20_0 | (inst.imm20_19 << 19)))));
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
