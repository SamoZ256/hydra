#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/integer_arithmetic.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: extended, write_cc, add_one, sat
void EmitIadd(DecoderContext& context, pred_t pred, bool pred_inv, reg_t dst,
              reg_t src_a, bool neg_a, ir::Value src_b, bool neg_b) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    auto src_a_v = NegIf(context.builder,
                         ir::Value::Register(src_a, DataType::I32), neg_a);
    auto src_b_v = NegIf(context.builder, src_b, neg_b);

    auto res = context.builder.OpAdd(src_a_v, src_b_v);
    context.builder.OpCopy(ir::Value::Register(dst, DataType::I32), res);

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace

void EmitIaddR(DecoderContext& context, InstIaddR inst) {
    EmitIadd(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
             inst.base.src_a, inst.base.neg_a,
             ir::Value::Register(inst.src_b, DataType::I32), inst.base.neg_b);
}

void EmitIaddC(DecoderContext& context, InstIaddC inst) {
    EmitIadd(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
             inst.base.src_a, inst.base.neg_a,
             ir::Value::ConstMemory(
                 CMem(inst.cbuf_slot, RZ, inst.cbuf_offset * 4), DataType::I32),
             inst.base.neg_b);
}

void EmitIaddI(DecoderContext& context, InstIaddI inst) {
    EmitIadd(context, inst.base.pred, inst.base.pred_inv, inst.base.dst,
             inst.base.src_a, inst.base.neg_a,
             ir::Value::Immediate((inst.imm20_0 | (inst.imm20_19 << 19)) << 12,
                                  DataType::I32),
             inst.base.neg_b);
}

void EmitIadd32I(DecoderContext& context, InstIadd32I inst) {
    EmitIadd(context, inst.pred, inst.pred_inv, inst.dst, inst.src_a,
             inst.neg_a, ir::Value::Immediate(inst.imm, DataType::I32), false);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
