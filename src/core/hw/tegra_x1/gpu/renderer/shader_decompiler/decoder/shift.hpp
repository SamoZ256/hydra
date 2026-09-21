#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

union InstShlBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<bool, 39, 1> m;
    BitField64<bool, 43, 1> x;
    BitField64<bool, 47, 1> write_cc;
};

DEFINE_INST_SRC2_VARIANTS(Shl);

enum class XMode {
    Xlo = 1,
    Xmed = 2,
    Xhi = 3,
};

union InstShrBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<bool, 39, 1> m;
    BitField64<bool, 40, 1> brev;
    BitField64<XMode, 43, 1> x_mode;
    BitField64<bool, 47, 1> write_cc;
    BitField64<bool, 48, 1> is_signed;
};

DEFINE_INST_SRC2_VARIANTS(Shr);

// TODO
/*
enum class XMode2 {
    Hi = 1,
    X = 2,
    Xhi = 3,
};

enum class MaxShift {
    U64 = 2,
    S64 = 3,
};

union InstShflrBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<MaxShift, 37, 2> max_shift;
    BitField64<reg_t, 39, 8> src_c;
    BitField64<bool, 47, 1> write_cc;
    BitField64<XMode2, 48, 2> x_mode;
    BitField64<bool, 50, 1> m;
};

union InstShflrR {
    InstShflrBase base;
    BitField64<reg_t, 20, 8> src_b;
};

void emitShflR(DecoderContext& context, InstShflrR inst);
void emitShfrR(DecoderContext& context, InstShflrR inst);

union InstShflrI {
    InstShflrBase base;
    BitField64<u32, 20, 6> imm6;
};

void emitShflI(DecoderContext& context, InstShflrI inst);
void emitShfrI(DecoderContext& context, InstShflrI inst);
*/

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
