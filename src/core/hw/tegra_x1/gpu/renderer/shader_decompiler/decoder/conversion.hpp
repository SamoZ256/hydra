#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

enum class FloatFormat {
    F16 = 1,
    F32 = 2,
    F64 = 3,
};

enum class RoundMode2 {
    Pass = 1,
    Round = 4,
    Floor = 5,
    Ceil = 6,
    Trunc = 7,
};

union InstF2fBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<FloatFormat, 8, 2> dst_fmt;
    BitField64<FloatFormat, 10, 2> src_fmt;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u32, 39, 2> round_mode_0;
    BitField64<bool, 41, 1> sh;
    BitField64<u32, 42, 1> round_mode_2;
    BitField64<bool, 44, 1> ftz;
    BitField64<bool, 45, 1> neg;
    BitField64<bool, 47, 1> write_cc;
    BitField64<bool, 49, 1> abs;
    BitField64<bool, 50, 1> sat;

    RoundMode2 GetRoundMode() const {
        return static_cast<RoundMode2>(round_mode_0.Get() |
                                       (round_mode_2.Get() << 2));
    }
};

union InstF2fR {
    InstF2fBase base;
    BitField64<reg_t, 20, 8> src;
};

void EmitF2fR(DecoderContext& context, InstF2fR inst);

union InstF2fC {
    InstF2fBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
};

void EmitF2fC(DecoderContext& context, InstF2fC inst);

union InstF2fI {
    InstF2fBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
};

void EmitF2fI(DecoderContext& context, InstF2fI inst);

enum class IntegerFormat {
    U16 = 1,
    U32 = 2,
    U64 = 3,
    S16 = 5,
    S32 = 6,
    S64 = 7,
};

enum class RoundMode3 {
    Round = 0,
    Floor = 1,
    Ceil = 2,
    Trunc = 3,
};

union InstF2iBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<u32, 8, 2> dst_fmt_0;
    BitField64<FloatFormat, 10, 2> src_fmt;
    BitField64<u32, 12, 1> dst_fmt_2;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<RoundMode3, 39, 2> round_mode;
    BitField64<bool, 41, 1> sh;
    BitField64<bool, 44, 1> ftz;
    BitField64<bool, 45, 1> neg;
    BitField64<bool, 47, 1> write_cc;
    BitField64<bool, 49, 1> abs;

    IntegerFormat GetDstFmt() const {
        return static_cast<IntegerFormat>(dst_fmt_0.Get() |
                                          (dst_fmt_2.Get() << 2));
    }
};

union InstF2iR {
    InstF2iBase base;
    BitField64<reg_t, 20, 8> src;
};

void EmitF2iR(DecoderContext& context, InstF2iR inst);

union InstF2iC {
    InstF2iBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
};

void EmitF2iC(DecoderContext& context, InstF2iC inst);

union InstF2iI {
    InstF2iBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
};

void EmitF2iI(DecoderContext& context, InstF2iI inst);

enum class ByteSelect {
    B0 = 0,
    B1 = 1,
    B2 = 2,
    B3 = 3,
};

enum class IntegerFormat2 {
    U8 = 0,
    U16 = 1,
    U32 = 2,
    S8 = 4,
    S16 = 5,
    S32 = 6,
};

union InstI2iBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<u32, 8, 2> dst_fmt_0;
    BitField64<u32, 10, 2> src_fmt_0;
    BitField64<u32, 12, 1> dst_fmt_2;
    BitField64<u32, 13, 2> src_fmt_2;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<ByteSelect, 41, 2> byte_sel;
    BitField64<bool, 45, 1> neg;
    BitField64<bool, 47, 1> write_cc;
    BitField64<bool, 49, 1> abs;
    BitField64<bool, 51, 1> sat;

    IntegerFormat2 GetDstFmt() const {
        return static_cast<IntegerFormat2>(dst_fmt_0.Get() |
                                           (dst_fmt_2.Get() << 2));
    }

    IntegerFormat2 GetSrcFmt() const {
        return static_cast<IntegerFormat2>(src_fmt_0.Get() |
                                           (src_fmt_2.Get() << 2));
    }
};

union InstI2iR {
    InstI2iBase base;
    BitField64<reg_t, 20, 8> src;
};

void EmitI2iR(DecoderContext& context, InstI2iR inst);

union InstI2iC {
    InstI2iBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
};

void EmitI2iC(DecoderContext& context, InstI2iC inst);

union InstI2iI {
    InstI2iBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
};

void EmitI2iI(DecoderContext& context, InstI2iI inst);

union InstI2fBase {
    BitField64<reg_t, 0, 8> dst;
    BitField64<FloatFormat, 8, 2> dst_fmt;
    BitField64<u32, 10, 2> src_fmt_0;
    BitField64<u32, 13, 2> src_fmt_2;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<RoundMode, 39, 2> round_mode;
    BitField64<ByteSelect, 41, 2> byte_sel;
    BitField64<bool, 45, 1> neg;
    BitField64<bool, 47, 1> write_cc;
    BitField64<bool, 49, 1> abs;

    IntegerFormat2 GetSrcFmt() const {
        return static_cast<IntegerFormat2>(src_fmt_0.Get() |
                                           (src_fmt_2.Get() << 2));
    }
};

union InstI2fR {
    InstI2fBase base;
    BitField64<reg_t, 20, 8> src;
};

void EmitI2fR(DecoderContext& context, InstI2fR inst);

union InstI2fC {
    InstI2fBase base;
    BitField64<u32, 20, 14> cbuf_offset;
    BitField64<u32, 34, 5> cbuf_slot;
};

void EmitI2fC(DecoderContext& context, InstI2fC inst);

union InstI2fI {
    InstI2fBase base;
    BitField64<u32, 20, 19> imm20_0;
    BitField64<u32, 56, 1> imm20_19;
};

void EmitI2fI(DecoderContext& context, InstI2fI inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
