#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/memory.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: p, todo
void EmitLoadAttribute(DecoderContext& context, pred_t pred, bool pred_inv,
                       bool is_input, LsSize size, reg_t dst, reg_t src,
                       u32 src_offset) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    u32 count;
    switch (size) {
    case LsSize::_32:
        count = 1;
        break;
    case LsSize::_64:
        count = 2;
        break;
    case LsSize::_96:
        count = 3;
        break;
    case LsSize::_128:
        count = 4;
        break;
    default:
        unreachable();
    }

    for (u32 i = 0; i < count; i++) {
        context.builder.OpCopy(
            ir::Value::Register(dst + i),
            ir::Value::AttrMemory(
                AMem(src, src_offset + i * sizeof(u32), is_input)));
    }

    if (conditional)
        context.builder.OpEndIf();
}

// TODO: p, todo
void EmitStoreAttribute(DecoderContext& context, pred_t pred, bool pred_inv,
                        LsSize size, reg_t dst, u32 dst_offset, reg_t src) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    u32 count;
    switch (size) {
    case LsSize::_32:
        count = 1;
        break;
    case LsSize::_64:
        count = 2;
        break;
    case LsSize::_96:
        count = 3;
        break;
    case LsSize::_128:
        count = 4;
        break;
    default:
        unreachable();
    }

    for (u32 i = 0; i < count; i++) {
        context.builder.OpCopy(ir::Value::AttrMemory(AMem(
                                   dst, dst_offset + i * sizeof(u32), false)),
                               ir::Value::Register(src + i));
    }

    if (conditional)
        context.builder.OpEndIf();
}

void EmitLoadConstant(DecoderContext& context, pred_t pred, bool pred_inv,
                      LsSize2 size, AddressMode address_mode, reg_t dst,
                      u32 cbuf_slot, reg_t src, u32 cbuf_offset) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    u32 count;
    switch (size) {
    case LsSize2::U8:
    case LsSize2::S8:
    case LsSize2::U16:
    case LsSize2::S16:
    case LsSize2::B32:
        count = 1;
        break;
    case LsSize2::B64:
        count = 2;
        break;
    case LsSize2::B128:
        count = 4;
        break;
    default:
        unreachable();
    }

    // TODO: slot addressing
    if (address_mode != AddressMode::Il)
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Slot addressing");

    // TODO: handle small ints
    if (size == LsSize2::U8 || size == LsSize2::S8 || size == LsSize2::U16 ||
        size == LsSize2::S16)
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Small integer loading");

    for (u32 i = 0; i < count; i++) {
        context.builder.OpCopy(
            ir::Value::Register(dst + i),
            ir::Value::ConstMemory(
                CMem(cbuf_slot, src, cbuf_offset + i * sizeof(u32))));
    }

    if (conditional)
        context.builder.OpEndIf();
}

// TODO: cache_op, extended
void EmitLoadGlobal(DecoderContext& context, pred_t pred, bool pred_inv,
                    LsSize3 size, reg_t dst, reg_t src, i32 offset) {
    const auto conditional = HandlePredCond(context.builder, pred, pred_inv);

    u32 count;
    switch (size) {
    case LsSize3::U8:
    case LsSize3::S8:
    case LsSize3::U16:
    case LsSize3::S16:
    case LsSize3::B32:
        count = 1;
        break;
    case LsSize3::B64:
        count = 2;
        break;
    case LsSize3::B128:
    case LsSize3::UB128:
        count = 4;
        break;
    default:
        unreachable();
    }

    // TODO: handle small ints
    if (size == LsSize3::U8 || size == LsSize3::S8 || size == LsSize3::U16 ||
        size == LsSize3::S16)
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Small integer loading");

    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Global memory");

    for (u32 i = 0; i < count; i++) {
        // TODO: global memory
        context.builder.OpCopy(ir::Value::Register(dst + i),
                               ir::Value::Immediate(std::bit_cast<u32>(0.0f)));
    }

    if (conditional)
        context.builder.OpEndIf();
}

} // namespace

void EmitLda(DecoderContext& context, InstLda inst) {
    EmitLoadAttribute(context, inst.pred, inst.pred_inv, !inst.o, inst.size,
                      inst.dst, inst.src, inst.src_offset);
}

void EmitSta(DecoderContext& context, InstSta inst) {
    EmitStoreAttribute(context, inst.pred, inst.pred_inv, inst.size, inst.dst,
                       inst.dst_offset, inst.src);
}

void EmitLdc(DecoderContext& context, InstLdc inst) {
    // TODO: sign extend cbuf_offset?
    EmitLoadConstant(context, inst.pred, inst.pred_inv, inst.size,
                     inst.address_mode, inst.dst, inst.cbuf_slot, inst.src,
                     inst.cbuf_offset);
}

void EmitLdg(DecoderContext& context, InstLdg inst) {
    EmitLoadGlobal(context, inst.pred, inst.pred_inv, inst.size, inst.dst,
                   inst.src, sign_extend<i32, 24>(inst.imm24));
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
