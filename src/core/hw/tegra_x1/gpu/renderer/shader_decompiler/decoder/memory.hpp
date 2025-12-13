#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

enum class LsSize {
    _32 = 0,
    _64 = 1,
    _96 = 2,
    _128 = 3,
};

union InstLda {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 11> src_offset;
    BitField64<bool, 31, 1> p;
    BitField64<bool, 32, 1> o;
    BitField64<reg_t, 39, 8> todo; // TODO
    BitField64<LsSize, 47, 2> size;
};

void EmitLda(DecoderContext& context, InstLda inst);

union InstSta {
    BitField64<reg_t, 0, 8> src;
    BitField64<reg_t, 8, 8> dst;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 11> dst_offset;
    BitField64<bool, 31, 1> p;
    BitField64<reg_t, 39, 8> todo; // TODO
    BitField64<LsSize, 47, 2> size;
};

void EmitSta(DecoderContext& context, InstSta inst);

enum class LsSize2 {
    U8 = 0,
    S8 = 1,
    U16 = 2,
    S16 = 3,
    B32 = 4,
    B64 = 5,
    B128 = 6,
};

enum class AddressMode {
    Il = 1,
    Is = 2,
    Isl = 3,
};

union InstLdc {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 16> cbuf_offset;
    BitField64<u32, 36, 5> cbuf_slot;
    BitField64<AddressMode, 44, 2> address_mode;
    BitField64<LsSize2, 48, 3> size;
};

void EmitLdc(DecoderContext& context, InstLdc inst);

enum class LsSize3 {
    U8 = 0,
    S8 = 1,
    U16 = 2,
    S16 = 3,
    B32 = 4,
    B64 = 5,
    B128 = 6,
    UB128 = 7,
};

enum class CacheOp {
    Ca = 0,
    Cg = 1,
    Ci = 2,
    Cv = 3,
};

union InstLdg {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 20, 24> imm24;
    BitField64<bool, 45, 1> e;
    BitField64<CacheOp, 46, 2> cache_op;
    BitField64<LsSize3, 48, 3> size;
};

void EmitLdg(DecoderContext& context, InstLdg inst);

// TODO: Stg

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
