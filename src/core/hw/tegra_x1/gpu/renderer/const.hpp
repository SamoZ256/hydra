#pragma once

#include "core/hw/tegra_x1/gpu/engines/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class TextureBase;
class ShaderBase;

enum class TextureFormat {
    Invalid,

    R8Unorm,
    R8Snorm,
    R8Uint,
    R8Sint,
    R16Float,
    R16Unorm,
    R16Snorm,
    R16Uint,
    R16Sint,
    R32Float,
    R32Uint,
    R32Sint,
    RG8Unorm,
    RG8Snorm,
    RG8Uint,
    RG8Sint,
    RG16Float,
    RG16Unorm,
    RG16Snorm,
    RG16Uint,
    RG16Sint,
    RG32Float,
    RG32Uint,
    RG32Sint,
    RGB32Float,
    RGB32Uint,
    RGB32Sint,
    RGBA8Unorm,
    RGBA8Snorm,
    RGBA8Uint,
    RGBA8Sint,
    RGBA16Float,
    RGBA16Unorm,
    RGBA16Snorm,
    RGBA16Uint,
    RGBA16Sint,
    RGBA32Float,
    RGBA32Uint,
    RGBA32Sint,
    S8Uint,
    Z16Unorm,
    Z24Unorm_X8Uint,
    Z32Float,
    Z24Unorm_S8Uint,
    Z32Float_X24S8Uint,
    RGBX8Unorm_sRGB,
    RGBA8Unorm_sRGB,
    RGBA4Unorm,
    RGB5Unorm,
    RGB5A1Unorm,
    R5G6B5Unorm,
    RGB10A2Unorm,
    RGB10A2Uint,
    RG11B10Float,
    E5BGR9Float,
    BC1_RGB,
    BC1_RGBA,
    BC2_RGBA,
    BC3_RGBA,
    BC1_RGB_sRGB,
    BC1_RGBA_sRGB,
    BC2_RGBA_sRGB,
    BC3_RGBA_sRGB,
    BC4_RUnorm,
    BC4_RSnorm,
    BC5_RGUnorm,
    BC5_RGSnorm,
    BC7_RGBAUnorm,
    BC7_RGBAUnorm_sRGB,
    BC6H_RGBA_SF16_Float,
    BC6H_RGBA_UF16_Float,
    RGBX8Unorm,
    RGBX8Snorm,
    RGBX8Uint,
    RGBX8Sint,
    RGBX16Float,
    RGBX16Unorm,
    RGBX16Snorm,
    RGBX16Uint,
    RGBX16Sint,
    RGBX32Float,
    RGBX32Uint,
    RGBX32Sint,
    ASTC_RGBA_4x4,
    ASTC_RGBA_5x4,
    ASTC_RGBA_5x5,
    ASTC_RGBA_6x5,
    ASTC_RGBA_6x6,
    ASTC_RGBA_8x5,
    ASTC_RGBA_8x6,
    ASTC_RGBA_8x8,
    ASTC_RGBA_10x5,
    ASTC_RGBA_10x6,
    ASTC_RGBA_10x8,
    ASTC_RGBA_10x10,
    ASTC_RGBA_12x10,
    ASTC_RGBA_12x12,
    ASTC_RGBA_4x4_sRGB,
    ASTC_RGBA_5x4_sRGB,
    ASTC_RGBA_5x5_sRGB,
    ASTC_RGBA_6x5_sRGB,
    ASTC_RGBA_6x6_sRGB,
    ASTC_RGBA_8x5_sRGB,
    ASTC_RGBA_8x6_sRGB,
    ASTC_RGBA_8x8_sRGB,
    ASTC_RGBA_10x5_sRGB,
    ASTC_RGBA_10x6_sRGB,
    ASTC_RGBA_10x8_sRGB,
    ASTC_RGBA_10x10_sRGB,
    ASTC_RGBA_12x10_sRGB,
    ASTC_RGBA_12x12_sRGB,
    B5G6R5Unorm,
    BGR5Unorm,
    BGR5A1Unorm,
    A1BGR5Unorm,
    BGRX8Unorm,
    BGRA8Unorm,
    BGRX8Unorm_sRGB,
    BGRA8Unorm_sRGB,
    ETC2_R_Unorm,
    ETC2_R_Snorm,
    ETC2_RG_Unorm,
    ETC2_RG_Snorm,
    ETC2_RGB, // compatible with ETC1
    PTA_ETC2_RGB,
    ETC2_RGBA,
    ETC2_RGB_sRGB,
    PTA_ETC2_RGB_sRGB,
    ETC2_RGBA_sRGB,
};

TextureFormat to_texture_format(NvColorFormat color_format);
TextureFormat to_texture_format(const ImageFormatWord image_format_word);
TextureFormat to_texture_format(ColorSurfaceFormat color_surface_format);
TextureFormat to_texture_format(DepthSurfaceFormat depth_surface_format);

usize get_texture_format_stride(const TextureFormat format, usize width);
bool is_texture_format_compressed(const TextureFormat format);

struct SwizzleChannels {
    ImageSwizzle r : 3;
    ImageSwizzle g : 3;
    ImageSwizzle b : 3;
    ImageSwizzle a : 3;

    SwizzleChannels(const ImageSwizzle r_, const ImageSwizzle g_,
                    const ImageSwizzle b_, const ImageSwizzle a_)
        : r{r_}, g{g_}, b{b_}, a{a_} {}

    SwizzleChannels(const TextureFormat format, const ImageSwizzle x,
                    const ImageSwizzle y, const ImageSwizzle z,
                    const ImageSwizzle w);

    bool operator==(const SwizzleChannels& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
};

SwizzleChannels
get_texture_format_default_swizzle_channels(const TextureFormat format);

enum class BlendOperation {
    Add = 1,
    Sub = 2,
    RevSub = 3,
    Min = 4,
    Max = 5,
};

enum class BlendFactor {
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
    ConstColor = 20,
    InvConstColor = 21,
    ConstAlpha = 22,
    InvConstAlpha = 23,
};

struct BufferDescriptor {
    uptr ptr;
    usize size;
};

struct TextureDescriptor {
    uptr ptr;
    TextureFormat format;
    NvKind kind;
    usize width;
    usize height;
    usize block_height_log2;
    usize stride;
    SwizzleChannels swizzle_channels;
    // TODO: more

    TextureDescriptor(const uptr ptr_, const TextureFormat format_,
                      const NvKind kind_, const usize width_,
                      const usize height_, const usize block_height_log2_,
                      const usize stride_,
                      const SwizzleChannels& swizzle_channels_)
        : ptr{ptr_}, format{format_}, kind{kind_}, width{width_},
          height{height_}, block_height_log2{block_height_log2_},
          stride{stride_}, swizzle_channels{swizzle_channels_} {}

    TextureDescriptor(const uptr ptr_, const TextureFormat format_,
                      const NvKind kind_, const usize width_,
                      const usize height_, const usize block_height_log2_,
                      const usize stride_)
        : TextureDescriptor(
              ptr_, format_, kind_, width_, height_, block_height_log2_,
              stride_, get_texture_format_default_swizzle_channels(format_)) {}
};

struct TextureViewDescriptor {
    TextureFormat format;
    SwizzleChannels swizzle_channels;

    u32 GetHash() const {
        return (u32)format | ((u32)swizzle_channels.r << 8) |
               ((u32)swizzle_channels.g << 11) |
               ((u32)swizzle_channels.b << 14) |
               ((u32)swizzle_channels.a << 17);
    }
};

enum class SamplerFilter {
    Nearest = 1,
    Linear = 2,
};

enum class SamplerMipFilter {
    NotMipmapped = 1,
    Nearest = 2,
    Linear = 3,
};

enum class SamplerAddressMode {
    Repeat = 0,
    MirroredRepeat = 1,
    ClampToEdge = 2,
    ClampToBorder = 3,
    Clamp = 4,
    MirrorClampToEdge = 5,
    MirrorClampToBorder = 6,
    MirrorClamp = 7,
};

struct SamplerDescriptor {
    SamplerFilter min_filter;
    SamplerFilter mag_filter;
    SamplerMipFilter mip_filter;
    SamplerAddressMode address_mode_r;
    SamplerAddressMode address_mode_s;
    SamplerAddressMode address_mode_t;
    // TODO: more
};

struct RenderTargetDescriptor {
    TextureBase* texture;
    bool load_action_clear = false;
    union {
        float color[4];
        struct {
            bool clear_depth;
            bool clear_stencil;
            float depth;
            u32 stencil;
        };
    } clear_data;
};

struct RenderPassDescriptor {
    RenderTargetDescriptor color_targets[COLOR_TARGET_COUNT];
    RenderTargetDescriptor depth_stencil_target;
};

enum class ShaderType {
    Vertex,
    Fragment,

    Count,
};

struct ResourceMapping {
    u32 uniform_buffers[UNIFORM_BUFFER_BINDING_COUNT];
    // TODO: storage buffers
    std::map<u32, u32> textures;
    // TODO: images

    ResourceMapping() {
        for (u32 i = 0; i < UNIFORM_BUFFER_BINDING_COUNT; i++)
            uniform_buffers[i] = invalid<u32>();
        // TODO: storage buffers
        // TODO: images
    }
};

struct ShaderDescriptor {
    ShaderType type;
    ShaderBackend backend;
    std::vector<u8> code;
    ResourceMapping resource_mapping;
};

struct VertexArray {
    bool enable;
    u32 stride;
    bool is_per_instance;
    u32 divisor;
};

struct VertexState {
    engines::VertexAttribState vertex_attrib_states[VERTEX_ATTRIB_COUNT];
    VertexArray vertex_arrays[VERTEX_ARRAY_COUNT];
};

struct ColorTargetState {
    TextureFormat format;
    bool blend_enabled;
    BlendOperation rgb_op;
    BlendFactor src_rgb_factor;
    BlendFactor dst_rgb_factor;
    BlendOperation alpha_op;
    BlendFactor src_alpha_factor;
    BlendFactor dst_alpha_factor;
};

struct PipelineDescriptor {
    ShaderBase* shaders[usize(ShaderType::Count)];
    VertexState vertex_state;
    ColorTargetState color_target_states[COLOR_TARGET_COUNT];
};

usize get_vertex_format_size(engines::VertexAttribSize size);

} // namespace hydra::hw::tegra_x1::gpu::renderer

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::TextureFormat, Invalid, "invalid",
    R8Unorm, "r8unorm", R8Snorm, "r8snorm", R8Uint, "r8uint", R8Sint, "r8sint",
    R16Float, "r16float", R16Unorm, "r16unorm", R16Snorm, "r16snorm", R16Uint,
    "r16uint", R16Sint, "r16sint", R32Float, "r32float", R32Uint, "r32uint",
    R32Sint, "r32sint", RG8Unorm, "rg8unorm", RG8Snorm, "rg8snorm", RG8Uint,
    "rg8uint", RG8Sint, "rg8sint", RG16Float, "rg16float", RG16Unorm,
    "rg16unorm", RG16Snorm, "rg16snorm", RG16Uint, "rg16uint", RG16Sint,
    "rg16sint", RG32Float, "rg32float", RG32Uint, "rg32uint", RG32Sint,
    "rg32sint", RGB32Float, "rgb32float", RGB32Uint, "rgb32uint", RGB32Sint,
    "rgb32sint", RGBA8Unorm, "rgba8unorm", RGBA8Snorm, "rgba8snorm", RGBA8Uint,
    "rgba8uint", RGBA8Sint, "rgba8sint", RGBA16Float, "rgba16float",
    RGBA16Unorm, "rgba16unorm", RGBA16Snorm, "rgba16snorm", RGBA16Uint,
    "rgba16uint", RGBA16Sint, "rgba16sint", RGBA32Float, "rgba32float",
    RGBA32Uint, "rgba32uint", RGBA32Sint, "rgba32sint", S8Uint, "s8uint",
    Z16Unorm, "z16unorm", Z24Unorm_X8Uint, "z24unorm x8uint", Z32Float,
    "z32float", Z24Unorm_S8Uint, "z24unorm s8uint", Z32Float_X24S8Uint,
    "z32float x24s8uint", RGBX8Unorm_sRGB, "rgbx8unorm_srgb", RGBA8Unorm_sRGB,
    "rgba8unorm_srgb", RGBA4Unorm, "rgba4unorm", RGB5Unorm, "rgb5unorm",
    RGB5A1Unorm, "rgb5a1unorm", R5G6B5Unorm, "r5g6b5unorm", RGB10A2Unorm,
    "rgb10a2unorm", RGB10A2Uint, "rgb10a2uint", RG11B10Float, "rg11b10float",
    E5BGR9Float, "e5bgr9float", BC1_RGB, "bc1_rgb", BC1_RGBA, "bc1_rgba",
    BC2_RGBA, "bc2_rgba", BC3_RGBA, "bc3_rgba", BC1_RGB_sRGB, "bc1_rgb_srgb",
    BC1_RGBA_sRGB, "bc1_rgba_srgb", BC2_RGBA_sRGB, "bc2_rgba_srgb",
    BC3_RGBA_sRGB, "bc3_rgba_srgb", BC4_RUnorm, "bc4_runorm", BC4_RSnorm,
    "bc4_rsnorm", BC5_RGUnorm, "bc5_rgunorm", BC5_RGSnorm, "bc5_rgsnorm",
    BC7_RGBAUnorm, "bc7_rgbaunorm", BC7_RGBAUnorm_sRGB, "bc7_rgbaunorm_srgb",
    BC6H_RGBA_SF16_Float, "bc6h_rgba_sf16_float", BC6H_RGBA_UF16_Float,
    "bc6h_rgba_uf16_float", RGBX8Unorm, "rgbx8unorm", RGBX8Snorm, "rgbx8snorm",
    RGBX8Uint, "rgbx8uint", RGBX8Sint, "rgbx8sint", RGBX16Float, "rgbx16float",
    RGBX16Unorm, "rgbx16unorm", RGBX16Snorm, "rgbx16snorm", RGBX16Uint,
    "rgbx16uint", RGBX16Sint, "rgbx16sint", RGBX32Float, "rgbx32float",
    RGBX32Uint, "rgbx32uint", RGBX32Sint, "rgbx32sint", ASTC_RGBA_4x4,
    "astc_rgba_4x4", ASTC_RGBA_5x4, "astc_rgba_5x4", ASTC_RGBA_5x5,
    "astc_rgba_5x5", ASTC_RGBA_6x5, "astc_rgba_6x5", ASTC_RGBA_6x6,
    "astc_rgba_6x6", ASTC_RGBA_8x5, "astc_rgba_8x5", ASTC_RGBA_8x6,
    "astc_rgba_8x6", ASTC_RGBA_8x8, "astc_rgba_8x8", ASTC_RGBA_10x5,
    "astc_rgba_10x5", ASTC_RGBA_10x6, "astc_rgba_10x6", ASTC_RGBA_10x8,
    "astc_rgba_10x8", ASTC_RGBA_10x10, "astc_rgba_10x10", ASTC_RGBA_12x10,
    "astc_rgba_12x10", ASTC_RGBA_12x12, "astc_rgba_12x12", ASTC_RGBA_4x4_sRGB,
    "astc_rgba_4x4_srgb", ASTC_RGBA_5x4_sRGB, "astc_rgba_5x4_srgb",
    ASTC_RGBA_5x5_sRGB, "astc_rgba_5x5_srgb", ASTC_RGBA_6x5_sRGB,
    "astc_rgba_6x5_srgb", ASTC_RGBA_6x6_sRGB, "astc_rgba_6x6_srgb",
    ASTC_RGBA_8x5_sRGB, "astc_rgba_8x5_srgb", ASTC_RGBA_8x6_sRGB,
    "astc_rgba_8x6_srgb", ASTC_RGBA_8x8_sRGB, "astc_rgba_8x8_srgb",
    ASTC_RGBA_10x5_sRGB, "astc_rgba_10x5_srgb", ASTC_RGBA_10x6_sRGB,
    "astc_rgba_10x6_srgb", ASTC_RGBA_10x8_sRGB, "astc_rgba_10x8_srgb",
    ASTC_RGBA_10x10_sRGB, "astc_rgba_10x10_srgb", ASTC_RGBA_12x10_sRGB,
    "astc_rgba_12x10_srgb", ASTC_RGBA_12x12_sRGB, "astc_rgba_12x12_srgb",
    B5G6R5Unorm, "b5g6r5unorm", BGR5Unorm, "bgr5unorm", BGR5A1Unorm,
    "bgr5a1unorm", A1BGR5Unorm, "a1bgr5unorm", BGRX8Unorm, "bgrx8unorm",
    BGRA8Unorm, "bgra8unorm", BGRX8Unorm_sRGB, "bgrx8unorm_srgb",
    BGRA8Unorm_sRGB, "bgra8unorm_srgb", ETC2_R_Unorm, "etc2_r_unorm",
    ETC2_R_Snorm, "etc2_r_snorm", ETC2_RG_Unorm, "etc2_rg_unorm", ETC2_RG_Snorm,
    "etc2_rg_snorm", ETC2_RGB, "etc2_rgb", PTA_ETC2_RGB, "pta_etc2_rgb",
    ETC2_RGBA, "etc2_rgba", ETC2_RGB_sRGB, "etc2_rgb_srgb", PTA_ETC2_RGB_sRGB,
    "pta_etc2_rgb_srgb", ETC2_RGBA_sRGB, "etc2_rgba_srgb")

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::renderer::ShaderType, Vertex,
                       "vertex", Fragment, "fragment", Count, "invalid")
