#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

enum class RroOp {
    SinCos = 0,
    Ex2 = 1,
};

union InstRroBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<RroOp, 39, 1> op;
    BitField64<bool, 45, 1> neg;
    BitField64<bool, 49, 1> abs;
};

union InstRroR {
    InstRroBase base;
    BitField64<reg_t, 20, 8> src;
};

void EmitRroR(DecoderContext& context, InstRroR inst);

union InstRroC {
    InstRroBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
};

void EmitRroC(DecoderContext& context, InstRroC inst);

union InstRroI {
    InstRroBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
};

void EmitRroI(DecoderContext& context, InstRroI inst);

enum class MultifunctionOp {
    Cos = 0,
    Sin = 1,
    Ex2 = 2,
    Lg2 = 3,
    Rcp = 4,
    Rsq = 5,
    Rcp64h = 6,
    Rsq64h = 7,
    Sqrt = 8,
};

union InstMufu {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<MultifunctionOp, 20, 4> op;
    BitField64<bool, 46, 1> abs;
    BitField64<bool, 48, 1> neg;
    BitField64<bool, 50, 1> sat;
};

void EmitMufu(DecoderContext& context, InstMufu inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
