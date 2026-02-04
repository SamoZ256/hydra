#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

enum class AvgMode {
    NoNeg = 0,
    NegB = 1,
    NegA = 2,
    PlusOne = 3,
};

union InstIaddBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<bool, 43, 1> x;
    BitField64<bool, 47, 1> write_cc;
    BitField64<AvgMode, 48, 2> avg_mode;
    BitField64<bool, 50, 1> sat;
};

DEFINE_INST_SRC2_VARIANTS(Iadd);

union InstIadd32I {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 32> imm;
    BitField64<bool, 52, 1> write_cc;
    BitField64<bool, 53, 1> x;
    BitField64<bool, 54, 1> sat;
    BitField64<AvgMode, 55, 2> avg_mode;
};

void EmitIadd32I(DecoderContext& context, InstIadd32I inst);

union InstIscaddBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 39, 5> shift;
    BitField64<bool, 47, 1> write_cc;
    BitField64<AvgMode, 48, 2> avg_mode;
};

DEFINE_INST_SRC2_VARIANTS(Iscadd);

union InstIscadd32I {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 32> imm;
    BitField64<bool, 52, 1> write_cc;
    BitField64<u32, 53, 5> shift;
};

void EmitIscadd32I(DecoderContext& context, InstIscadd32I inst);

enum class XmadCop {
    Cfull = 0,
    Clo = 1,
    Chi = 2,
    Csfu = 3,
    Cbcc = 4,
};

union InstXmadBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<bool, 38, 1> x;
    BitField64<bool, 47, 1> write_cc;
    BitField64<bool, 48, 1> a_signed;
    BitField64<bool, 49, 1> b_signed;
    BitField64<bool, 53, 1> hilo_a;
};

union InstXmadR {
    InstXmadBase base;
    BitField64<reg_t, 20, 8> src_b;
    BitField64<bool, 35, 1> hilo_b;
    BitField64<bool, 36, 1> psl;
    BitField64<bool, 37, 1> mrg;
    BitField64<reg_t, 39, 8> src_c;
    BitField64<XmadCop, 50, 3> cop;
};

void EmitXmadR(DecoderContext& context, InstXmadR inst);

union InstXmadRC {
    InstXmadBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
    BitField64<reg_t, 39, 8> src_b;
    BitField64<XmadCop, 50, 2> cop;
    BitField64<bool, 52, 1> hilo_b;
};

void EmitXmadRC(DecoderContext& context, InstXmadRC inst);

union InstXmadC {
    InstXmadBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
    BitField64<reg_t, 39, 8> src_c;
    BitField64<XmadCop, 50, 2> cop;
    BitField64<bool, 52, 1> hilo_b;
};

void EmitXmadC(DecoderContext& context, InstXmadC inst);

union InstXmadI {
    InstXmadBase base;
    BitField64<u32, 20, 16> imm16;
    BitField64<bool, 36, 1> psl;
    BitField64<bool, 37, 1> mrg;
    BitField64<reg_t, 39, 8> src_c;
    BitField64<XmadCop, 50, 3> cop;
};

void EmitXmadI(DecoderContext& context, InstXmadI inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
