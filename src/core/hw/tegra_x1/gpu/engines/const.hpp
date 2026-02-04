#pragma once

#include "core/hw/tegra_x1/gpu/const.hpp"

namespace hydra::hw::tegra_x1::gpu::engines {

struct Iova {
    u32 hi;
    u32 lo;

    operator u64() const { return u64(hi) << 32 | u64(lo); }
};

enum class Winding : u32 {
    Clockwise = 0x900,
    CounterClockwise = 0x901,
};

enum class CullFaceMode : u32 {
    Front = 0x404,
    Back = 0x405,
    FrontAndBack = 0x408,
};

enum class PrimitiveType : u32 {
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

enum class IndexType : u32 {
    UInt8,
    UInt16,
    UInt32,
    None,
};

inline u32 get_index_type_size(IndexType type) {
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

enum class ShaderStage : u32 {
    VertexA,
    VertexB,
    TessCtrl,
    TessEval,
    Geometry,
    Fragment,

    Count,
};

enum class CompareOp : u32 {
    Invalid = 0,
    Never = 1,
    Less = 2,
    Equal = 3,
    LessEqual = 4,
    Greater = 5,
    NotEqual = 6,
    GreaterEqual = 7,
    Always = 8,
};

enum class TextureType : u32 {
    _1D = 0,
    _2D = 1,
    _3D = 2,
    Cubemap = 3,
    _1DArray = 4,
    _2DArray = 5,
    _1DBuffer = 6,
    _2DNoMipmap = 7,
    CubeArray = 8,
};

enum class MemoryLayout : u32 {
    BlockLinear,
    Pitch,
};

enum class BlockDim : u32 {
    OneGob = 0,
    TwoGobs = 1,
    FourGobs = 2,
    EightGobs = 3,
    SixteenGobs = 4,
    ThirtyTwoGobs = 5,
    QuarterGob = 14,
};

inline i32 get_block_size_log2(const BlockDim dim) {
    return sign_extend<i32, 4>(static_cast<i32>(dim));
}

enum class ColorWriteMask : u32 {
    None = 0,
    Red = BIT(0),
    Green = BIT(4),
    Blue = BIT(8),
    Alpha = BIT(12),
    All = Red | Green | Blue | Alpha,
};
ENABLE_ENUM_BITWISE_OPERATORS(ColorWriteMask)

} // namespace hydra::hw::tegra_x1::gpu::engines

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::engines::PrimitiveType, Points,
                       "points", Lines, "lines", LineLoop, "line loop",
                       LineStrip, "line strip", Triangles, "triangles",
                       TriangleStrip, "triangle strip", TriangleFan,
                       "triangle fan", Quads, "quads", QuadStrip, "quad strip",
                       Polygon, "polygon", LinesAdjacency, "lines adjacency",
                       LineStripAdjacency, "line strip adjacency",
                       TrianglesAdjacency, "triangles adjacency",
                       TriangleStripAdjacency, "triangle strip adjacency",
                       Patches, "patches")

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::engines::IndexType, None,
                       "none", UInt8, "u8", UInt16, "u16", UInt32, "u32")

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::engines::VertexAttribType,
                       None, "none", Snorm, "snorm", Unorm, "unorm", Sint,
                       "sint", Uint, "uint", Uscaled, "uscaled", Sscaled,
                       "sscaled", Float, "float")

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::engines::VertexAttribSize,
                       _1x32, "1x32", _2x32, "2x32", _3x32, "3x32", _4x32,
                       "4x32", _1x16, "1x16", _2x16, "2x16", _3x16, "3x16",
                       _4x16, "4x16", _1x8, "1x8", _2x8, "2x8", _3x8, "3x8",
                       _4x8, "4x8", _10_10_10_2, "10_10_10_2", _11_11_10,
                       "11_11_10")

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::engines::ShaderStage, VertexA,
                       "vertex A", VertexB, "vertex B", TessCtrl,
                       "tessellation control", TessEval,
                       "tessellation evaluation", Geometry, "geometry",
                       Fragment, "fragment")

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::engines::CompareOp, Invalid,
                       "invalid", Never, "never", Less, "less", Equal, "equal",
                       LessEqual, "less equal", Greater, "greater", NotEqual,
                       "not equal", GreaterEqual, "greater equal", Always,
                       "always")
