#pragma once

#include "core/hw/tegra_x1/gpu/const.hpp"

#define MAKE_ADDR(addr) make_addr(addr.lo, addr.hi)
#define UNMAP_ADDR(addr)                                                       \
    GPU::GetInstance().GetGPUMMU().UnmapAddr(MAKE_ADDR(addr))

namespace hydra::hw::tegra_x1::gpu::engines {

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

enum class IndexType : u32 {
    UInt8,
    UInt16,
    UInt32,
    None,
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

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::engines::DepthTestFunc,
                       Invalid, "invalid", Never, "never", Less, "less", Equal,
                       "equal", LessEqual, "less equal", Greater, "greater",
                       NotEqual, "not equal", GreaterEqual, "greater equal",
                       Always, "always")
