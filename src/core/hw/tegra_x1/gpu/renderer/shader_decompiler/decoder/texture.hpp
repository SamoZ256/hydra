#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

// TODO: bindless

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

enum class TextureQuery {
    Dimensions = 1,
    TextureType = 2,
    SamplePos = 5,
    SamplerFilter = 16,
    SamplerLod = 18,
    SamplerWrap = 20,
    SamplerBorderColor = 22,
};

union InstTxq {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<u8, 31, 4> write_mask;
    BitField64<u32, 36, 13> cbuf_index;
    BitField64<bool, 49, 1> nodep;
    BitField64<TextureQuery, 22, 7> query;
};

void EmitTxq(DecoderContext& context, InstTxq inst);

enum class TextureComponent {
    R = 0,
    G = 1,
    B = 2,
    A = 3,
};

enum class TextureOffset {
    None = 0,
    Aoffi = 1,
    Ptp = 2,
};

enum class TextureType {
    _1DLodZero = 0,
    _2D = 1,
    _2DLodZero = 2,
    _2DLodLevel = 3,
    _2DDepthCompare = 4,
    _2DLodLevelDepthCompare = 5,
    _2DLodZeroDepthCompare = 6,
    _2DArray = 7,
    _2DArrayLodZero = 8,
    _2DArrayLodZeroDepthCompare = 9,
    _3D = 10,
    _3DLodZero = 11,
    Cube = 12,
    CubeLodLevel = 13,
};

union InstTexsTlds {
    BitField64<reg_t, 0, 8> dst0;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<reg_t, 20, 8> src_b;
    BitField64<reg_t, 28, 8> dst1;
    BitField64<u32, 36, 13> cbuf_index;
    BitField64<bool, 49, 1> nodep;
    BitField64<u8, 50, 3> write_mask;
    BitField64<TextureType, 53, 4> type;
};

using InstTexs = InstTexsTlds;
using InstTlds = InstTexsTlds;

void EmitTexs(DecoderContext& context, InstTexs inst);
void EmitTlds(DecoderContext& context, InstTlds inst);

enum class TextureType2 {
    _1D = 0,
    _1DArray = 1,
    _2D = 2,
    _2DArray = 3,
    _3D = 4,
    _3DArray = 5,
    Cube = 6,
    ArrayCube = 7,
};

union InstTld4 {
    BitField64<reg_t, 0, 8> dst;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<reg_t, 20, 8> src_b;
    BitField64<TextureType2, 28, 3> type;
    BitField64<u8, 31, 4> write_mask;
    BitField64<bool, 35, 1> ndv;
    BitField64<u32, 36, 13> cbuf_index;
    BitField64<bool, 49, 1> nodep;
    BitField64<bool, 50, 1> dc;
    BitField64<pred_t, 51, 3> dst_pred;
    BitField64<TextureOffset, 54, 2> offset;
    BitField64<TextureComponent, 56, 2> component;
    BitField64<bool, 58, 1> lc;
};

void EmitTld4(DecoderContext& context, InstTld4 inst);

union InstTld4s {
    BitField64<reg_t, 0, 8> dst0;
    BitField64<reg_t, 8, 8> src_a;
    BitField64<pred_t, 16, 3> pred;
    BitField64<bool, 19, 1> pred_inv;
    BitField64<reg_t, 20, 8> src_b;
    BitField64<reg_t, 28, 8> dst1;
    BitField64<u32, 36, 13> cbuf_index;
    BitField64<TextureComponent, 52, 2> component;
    BitField64<bool, 49, 1> nodep;
    BitField64<bool, 50, 1> dc;
    BitField64<bool, 51, 1> aoffi;
};

void EmitTld4s(DecoderContext& context, InstTld4s inst);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder::TextureQuery,
    Dimensions, "dimensions", TextureType, "texture type", SamplePos,
    "sample pos", SamplerFilter, "sampler filter", SamplerLod, "sampler LOD",
    SamplerWrap, "sampler wrap", SamplerBorderColor, "sampler border color")
