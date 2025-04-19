#pragma once

#include "core/hw/tegra_x1/gpu/const.hpp"

#define MAKE_ADDR(addr) make_addr(addr.lo, addr.hi)
#define UNMAP_ADDR(addr)                                                       \
    GPU::GetInstance().GetGPUMMU().UnmapAddr(MAKE_ADDR(addr))

namespace Hydra::HW::TegraX1::GPU::Engines {

struct Iova {
    u32 hi;
    u32 lo;
};

enum class PrimitiveType {
    Points,
    Lines,
    LineLoop,
    LineStrip,
    Triangles,
    TriangleStrip,
    TriangleFan,
    Quads,
    QuadStrip,
    Polygon,
    LinesAdjacency,
    LineStripAdjacency,
    TrianglesAdjacency,
    TriangleStripAdjacency,
    Patches,
};

enum class IndexType {
    UInt8,
    UInt16,
    UInt32,
};

inline usize get_index_type_size(IndexType type) {
    switch (type) {
    case IndexType::UInt8:
        return sizeof(u8);
    case IndexType::UInt16:
        return sizeof(u16);
    case IndexType::UInt32:
        return sizeof(u32);
    default:
        return 0;
    }
}

enum class VertexAttribType : u32 {
    None,
    Snorm,
    Unorm,
    Sint,
    Uint,
    Uscaled,
    Sscaled,
    Float,
};

enum class VertexAttribSize : u32 {
    // One to four 32-bit components
    _1x32 = 0x12,
    _2x32 = 0x04,
    _3x32 = 0x02,
    _4x32 = 0x01,

    // One to four 16-bit components
    _1x16 = 0x1b,
    _2x16 = 0x0f,
    _3x16 = 0x05,
    _4x16 = 0x03,

    // One to four 8-bit components
    _1x8 = 0x1d,
    _2x8 = 0x18,
    _3x8 = 0x13,
    _4x8 = 0x0a,

    // Misc arrangements
    _10_10_10_2 = 0x30,
    _11_11_10 = 0x31,
};

struct VertexAttribState {
    u32 buffer_id : 5;
    u32 padding : 1;
    bool is_fixed : 1;
    u32 offset : 14;
    VertexAttribSize size : 6;
    VertexAttribType type : 3;
    bool bgra : 1;
};

enum class ShaderStage {
    VertexA,
    VertexB,
    TessCtrl,
    TessEval,
    Geometry,
    Fragment,

    Count,
};

enum class DepthTestFunc : u32 {
    Invalid,

    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,
};

enum class BlendOperation : u32 {
    Add = 1,
    Sub = 2,
    RevSub = 3,
    Min = 4,
    Max = 5,
};

enum class BlendFactor : u32 {
    Zero = 1,
    One = 2,
    SrcColor = 3,
    InvSrcColor = 4,
    SrcAlpha = 5,
    InvSrcAlpha = 6,
    DstAlpha = 7,
    InvDstAlpha = 8,
    DstColor = 9,
    InvDstColor = 10,
    SrcAlphaSaturate = 11,
    Src1Color = 16,
    InvSrc1Color = 17,
    Src1Alpha = 18,
    InvSrc1Alpha = 19,
    ConstColor = 1 | 0x20,
    InvConstColor = 2 | 0x20,
    ConstAlpha = 3 | 0x20,
    InvConstAlpha = 4 | 0x20,
};

// Pitch - buffer, block linear - texture
enum class MemoryLayout : u32 {
    BlockLinear,
    Pitch,
};

enum class BlockDim : u32 {
    OneGob,
    TwoGobs,
    FourGobs,
    EightGobs,
    SixteenGobs,
    ThirtyTwoGobs,
    QuarterGob = 14,
};

inline i32 get_block_size_log2(const BlockDim dim) {
    return sign_extend<i32, 4>(static_cast<i32>(dim));
}

} // namespace Hydra::HW::TegraX1::GPU::Engines

ENABLE_ENUM_FORMATTING(Hydra::HW::TegraX1::GPU::Engines::PrimitiveType, Points,
                       "points", Lines, "lines", LineLoop, "line loop",
                       LineStrip, "line strip", Triangles, "triangles",
                       TriangleStrip, "triangle strip", TriangleFan,
                       "triangle fan", Quads, "quads", QuadStrip, "quad strip",
                       Polygon, "polygon", LinesAdjacency, "lines adjacency",
                       LineStripAdjacency, "line strip adjacency",
                       TrianglesAdjacency, "triangles adjacency",
                       TriangleStripAdjacency, "triangle strip adjacency",
                       Patches, "patches")

ENABLE_ENUM_FORMATTING(Hydra::HW::TegraX1::GPU::Engines::IndexType, UInt8, "u8",
                       UInt16, "u16", UInt32, "u32")

ENABLE_ENUM_FORMATTING(Hydra::HW::TegraX1::GPU::Engines::VertexAttribType, None,
                       "none", Snorm, "snorm", Unorm, "unorm", Sint, "sint",
                       Uint, "uint", Uscaled, "uscaled", Sscaled, "sscaled",
                       Float, "float")

ENABLE_ENUM_FORMATTING(Hydra::HW::TegraX1::GPU::Engines::VertexAttribSize,
                       _1x32, "1x32", _2x32, "2x32", _3x32, "3x32", _4x32,
                       "4x32", _1x16, "1x16", _2x16, "2x16", _3x16, "3x16",
                       _4x16, "4x16", _1x8, "1x8", _2x8, "2x8", _3x8, "3x8",
                       _4x8, "4x8", _10_10_10_2, "10_10_10_2", _11_11_10,
                       "11_11_10")

ENABLE_ENUM_FORMATTING(Hydra::HW::TegraX1::GPU::Engines::ShaderStage, VertexA,
                       "vertex A", VertexB, "vertex B", TessCtrl,
                       "tessellation control", TessEval,
                       "tessellation evaluation", Geometry, "geometry",
                       Fragment, "fragment")

ENABLE_ENUM_FORMATTING(Hydra::HW::TegraX1::GPU::Engines::DepthTestFunc, Invalid,
                       "invalid", Never, "never", Less, "less", Equal, "equal",
                       LessEqual, "less equal", Greater, "greater", NotEqual,
                       "not equal", GreaterEqual, "greater equal", Always,
                       "always")
