#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/bitfield.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: brev, write_cc
void emitBitfieldExtract(DecoderContext& context, pred_t pred, bool pred_inv,
                         bool is_signed, reg_t dst, reg_t src_a,
                         ir::Value src_b) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    const auto src_a_v = ir::Value::createRegister(
        src_a, (is_signed ? ir::ScalarType::I32 : ir::ScalarType::U32));
    const auto position =
        context.builder.opBitwiseAnd(src_b, ir::Value::createConstantU(0xff));
    const auto size = context.builder.opBitfieldExtract(
        src_b, ir::Value::createConstantU(8), ir::Value::createConstantU(8));

    auto res = context.builder.opBitfieldExtract(src_a_v, position, size);
    context.builder.opCopy(ir::Value::createRegister(dst), res);

    if (conditional)
        context.builder.opEndIf();
}

} // namespace

void emitBfeR(DecoderContext& context, InstBfeR inst) {
    emitBitfieldExtract(context, inst.base.pred, inst.base.pred_inv,
                        inst.base.is_signed, inst.base.dst, inst.base.src_a,
                        ir::Value::createRegister(inst.src_b));
}

void emitBfeC(DecoderContext& context, InstBfeC inst) {
    emitBitfieldExtract(
        context, inst.base.pred, inst.base.pred_inv, inst.base.is_signed,
        inst.base.dst, inst.base.src_a,
        ir::Value::createConstMemory(
            CMem(inst.cbuf_slot, RZ, static_cast<u64>(inst.cbuf_offset * 4))));
}

void emitBfeI(DecoderContext& context, InstBfeI inst) {
    emitBitfieldExtract(
        context, inst.base.pred, inst.base.pred_inv, inst.base.is_signed,
        inst.base.dst, inst.base.src_a,
        ir::Value::createConstantU(inst.imm20_0 | (inst.imm20_19 << 19)));
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
