#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/bitfield.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: brev, write_cc
void EmitBitfieldExtract(DecoderContext& context, pred_t pred, bool pred_inv,
                         bool is_signed, reg_t dst, reg_t src_a,
                         ir::Value src_b) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    const auto src_a_v = ir::Value::Register(
        src_a, (is_signed ? ir::ScalarType::I32 : ir::ScalarType::U32));
    const auto position =
        context.builder.OpBitwiseAnd(src_b, ir::Value::ConstantU(0xff));
    const auto size = context.builder.OpBitfieldExtract(
        src_b, ir::Value::ConstantU(8), ir::Value::ConstantU(8));

    auto res = context.builder.OpBitfieldExtract(src_a_v, position, size);
    context.builder.OpCopy(ir::Value::Register(dst), res);

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace

void EmitBfeR(DecoderContext& context, InstBfeR inst) {
    EmitBitfieldExtract(context, inst.base.pred, inst.base.pred_inv,
                        inst.base.is_signed, inst.base.dst, inst.base.src_a,
                        ir::Value::Register(inst.src_b));
}

void EmitBfeC(DecoderContext& context, InstBfeC inst) {
    EmitBitfieldExtract(
        context, inst.base.pred, inst.base.pred_inv, inst.base.is_signed,
        inst.base.dst, inst.base.src_a,
        ir::Value::ConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4))));
}

void EmitBfeI(DecoderContext& context, InstBfeI inst) {
    EmitBitfieldExtract(
        context, inst.base.pred, inst.base.pred_inv, inst.base.is_signed,
        inst.base.dst, inst.base.src_a,
        ir::Value::ConstantU(inst.imm20_0 | (inst.imm20_19 << 19)));
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
