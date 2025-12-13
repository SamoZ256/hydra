#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/float_min_max.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: ftz, write_cc
void EmitFloatMinMax(DecoderContext& context, pred_t pred, bool pred_inv,
                     reg_t dst, pred_t src_pred, bool src_pred_inv, reg_t src_a,
                     bool abs_a, bool neg_a, ir::Value src_b, bool abs_b,
                     bool neg_b) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v =
        AbsNegIf(context.builder, ir::Value::Register(src_a, DataType::F32),
                 abs_a, neg_a);
    auto src_b_v = AbsNegIf(context.builder, src_b, abs_b, neg_b);
    auto min_v = context.builder.OpMin(src_a_v, src_b_v);
    auto max_v = context.builder.OpMax(src_a_v, src_b_v);
    auto res = context.builder.OpSelect(
        NotIf(context.builder, ir::Value::Predicate(src_pred), src_pred_inv),
        min_v, max_v);
    context.builder.OpCopy(ir::Value::Register(dst, DataType::F32), res);

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace

void EmitFmnmxR(DecoderContext& context, InstFmnmxR inst) {
    EmitFloatMinMax(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
                    inst.base.src_pred, inst.base.src_pred_inv, inst.base.src_a,
                    inst.base.abs_a, inst.base.neg_a,
                    ir::Value::Register(inst.src_b, DataType::F32),
                    inst.base.abs_b, inst.base.neg_b);
}

void EmitFmnmxC(DecoderContext& context, InstFmnmxC inst) {
    EmitFloatMinMax(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        inst.base.src_pred, inst.base.src_pred_inv, inst.base.src_a,
        inst.base.abs_a, inst.base.neg_a,
        ir::Value::ConstMemory(CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4),
                               DataType::F32),
        inst.base.abs_b, inst.base.neg_b);
}

void EmitFmnmxI(DecoderContext& context, InstFmnmxI inst) {
    EmitFloatMinMax(
        context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
        inst.base.src_pred, inst.base.src_pred_inv, inst.base.src_a,
        inst.base.abs_a, inst.base.neg_a,
        ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                             DataType::F32),
        inst.base.abs_b, inst.base.neg_b);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
