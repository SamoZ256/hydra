#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

namespace {

#define FORMAT(bytes_per_block, block_width, block_height, is_depth_stencil)   \
    TextureFormatInfo {                                                        \
        bytes_per_block, block_width, block_height, is_depth_stencil           \
    }
#define COLOR_FORMAT(bytes_per_block, block_width, block_height)               \
    FORMAT(bytes_per_block, block_width, block_height, false)
#define DEPTH_STENCIL_FORMAT(bytes_per_block, block_width, block_height)       \
    FORMAT(bytes_per_block, block_width, block_height, true)

TextureFormatInfo texture_format_infos[] = {
    FORMAT(0, 0, 0, false),        // Invalid
    COLOR_FORMAT(1, 1, 1),         // R8Unorm
    COLOR_FORMAT(1, 1, 1),         // R8Snorm
    COLOR_FORMAT(1, 1, 1),         // R8Uint
    COLOR_FORMAT(1, 1, 1),         // R8Sint
    COLOR_FORMAT(2, 1, 1),         // R16Float
    COLOR_FORMAT(2, 1, 1),         // R16Unorm
    COLOR_FORMAT(2, 1, 1),         // R16Snorm
    COLOR_FORMAT(2, 1, 1),         // R16Uint
    COLOR_FORMAT(2, 1, 1),         // R16Sint
    COLOR_FORMAT(4, 1, 1),         // R32Float
    COLOR_FORMAT(4, 1, 1),         // R32Uint
    COLOR_FORMAT(4, 1, 1),         // R32Sint
    COLOR_FORMAT(2, 1, 1),         // RG8Unorm
    COLOR_FORMAT(2, 1, 1),         // RG8Snorm
    COLOR_FORMAT(2, 1, 1),         // RG8Uint
    COLOR_FORMAT(2, 1, 1),         // RG8Sint
    COLOR_FORMAT(4, 1, 1),         // RG16Float
    COLOR_FORMAT(4, 1, 1),         // RG16Unorm
    COLOR_FORMAT(4, 1, 1),         // RG16Snorm
    COLOR_FORMAT(4, 1, 1),         // RG16Uint
    COLOR_FORMAT(4, 1, 1),         // RG16Sint
    COLOR_FORMAT(8, 1, 1),         // RG32Float
    COLOR_FORMAT(8, 1, 1),         // RG32Uint
    COLOR_FORMAT(8, 1, 1),         // RG32Sint
    COLOR_FORMAT(12, 1, 1),        // RGB32Float
    COLOR_FORMAT(12, 1, 1),        // RGB32Uint
    COLOR_FORMAT(12, 1, 1),        // RGB32Sint
    COLOR_FORMAT(4, 1, 1),         // RGBA8Unorm
    COLOR_FORMAT(4, 1, 1),         // RGBA8Snorm
    COLOR_FORMAT(4, 1, 1),         // RGBA8Uint
    COLOR_FORMAT(4, 1, 1),         // RGBA8Sint
    COLOR_FORMAT(8, 1, 1),         // RGBA16Float
    COLOR_FORMAT(8, 1, 1),         // RGBA16Unorm
    COLOR_FORMAT(8, 1, 1),         // RGBA16Snorm
    COLOR_FORMAT(8, 1, 1),         // RGBA16Uint
    COLOR_FORMAT(8, 1, 1),         // RGBA16Sint
    COLOR_FORMAT(16, 1, 1),        // RGBA32Float
    COLOR_FORMAT(16, 1, 1),        // RGBA32Uint
    COLOR_FORMAT(16, 1, 1),        // RGBA32Sint
    DEPTH_STENCIL_FORMAT(1, 1, 1), // S8Uint
    DEPTH_STENCIL_FORMAT(2, 1, 1), // Z16Unorm
    DEPTH_STENCIL_FORMAT(4, 1, 1), // Z24Unorm_X8Uint
    DEPTH_STENCIL_FORMAT(4, 1, 1), // Z32Float
    DEPTH_STENCIL_FORMAT(4, 1, 1), // Z24Unorm_S8Uint
    DEPTH_STENCIL_FORMAT(8, 1, 1), // Z32Float_X24S8Uint
    COLOR_FORMAT(4, 1, 1),         // RGBX8Unorm_sRGB
    COLOR_FORMAT(4, 1, 1),         // RGBA8Unorm_sRGB
    COLOR_FORMAT(2, 1, 1),         // RGBA4Unorm
    COLOR_FORMAT(2, 1, 1),         // RGB5Unorm
    COLOR_FORMAT(2, 1, 1),         // RGB5A1Unorm
    COLOR_FORMAT(2, 1, 1),         // R5G6B5Unorm
    COLOR_FORMAT(4, 1, 1),         // RGB10A2Unorm
    COLOR_FORMAT(4, 1, 1),         // RGB10A2Uint
    COLOR_FORMAT(4, 1, 1),         // RG11B10Float
    COLOR_FORMAT(4, 1, 1),         // E5BGR9Float
    COLOR_FORMAT(8, 4, 4),         // BC1_RGB
    COLOR_FORMAT(8, 4, 4),         // BC1_RGBA
    COLOR_FORMAT(16, 4, 4),        // BC2_RGBA
    COLOR_FORMAT(16, 4, 4),        // BC3_RGBA
    COLOR_FORMAT(8, 4, 4),         // BC1_RGB_sRGB
    COLOR_FORMAT(8, 4, 4),         // BC1_RGBA_sRGB
    COLOR_FORMAT(16, 4, 4),        // BC2_RGBA_sRGB
    COLOR_FORMAT(16, 4, 4),        // BC3_RGBA_sRGB
    COLOR_FORMAT(8, 4, 4),         // BC4_RUnorm
    COLOR_FORMAT(8, 4, 4),         // BC4_RSnorm
    COLOR_FORMAT(16, 4, 4),        // BC5_RGUnorm
    COLOR_FORMAT(16, 4, 4),        // BC5_RGSnorm
    COLOR_FORMAT(16, 4, 4),        // BC7_RGBAUnorm
    COLOR_FORMAT(16, 4, 4),        // BC7_RGBAUnorm_sRGB
    COLOR_FORMAT(16, 4, 4),        // BC6H_RGBA_SF16_Float
    COLOR_FORMAT(16, 4, 4),        // BC6H_RGBA_UF16_Float
    COLOR_FORMAT(4, 1, 1),         // RGBX8Unorm
    COLOR_FORMAT(4, 1, 1),         // RGBX8Snorm
    COLOR_FORMAT(4, 1, 1),         // RGBX8Uint
    COLOR_FORMAT(4, 1, 1),         // RGBX8Sint
    COLOR_FORMAT(8, 1, 1),         // RGBX16Float
    COLOR_FORMAT(8, 1, 1),         // RGBX16Unorm
    COLOR_FORMAT(8, 1, 1),         // RGBX16Snorm
    COLOR_FORMAT(8, 1, 1),         // RGBX16Uint
    COLOR_FORMAT(8, 1, 1),         // RGBX16Sint
    COLOR_FORMAT(16, 1, 1),        // RGBX32Float
    COLOR_FORMAT(16, 1, 1),        // RGBX32Uint
    COLOR_FORMAT(16, 1, 1),        // RGBX32Sint
    COLOR_FORMAT(16, 4, 4),        // ASTC_RGBA_4x4
    COLOR_FORMAT(16, 5, 4),        // ASTC_RGBA_5x4
    COLOR_FORMAT(16, 5, 5),        // ASTC_RGBA_5x5
    COLOR_FORMAT(16, 6, 5),        // ASTC_RGBA_6x5
    COLOR_FORMAT(16, 6, 6),        // ASTC_RGBA_6x6
    COLOR_FORMAT(16, 8, 5),        // ASTC_RGBA_8x5
    COLOR_FORMAT(16, 8, 6),        // ASTC_RGBA_8x6
    COLOR_FORMAT(16, 8, 8),        // ASTC_RGBA_8x8
    COLOR_FORMAT(16, 10, 5),       // ASTC_RGBA_10x5
    COLOR_FORMAT(16, 10, 6),       // ASTC_RGBA_10x6
    COLOR_FORMAT(16, 10, 8),       // ASTC_RGBA_10x8
    COLOR_FORMAT(16, 10, 10),      // ASTC_RGBA_10x10
    COLOR_FORMAT(16, 12, 10),      // ASTC_RGBA_12x10
    COLOR_FORMAT(16, 12, 12),      // ASTC_RGBA_12x12
    COLOR_FORMAT(16, 4, 4),        // ASTC_RGBA_4x4_sRGB
    COLOR_FORMAT(16, 5, 4),        // ASTC_RGBA_5x4_sRGB
    COLOR_FORMAT(16, 5, 5),        // ASTC_RGBA_5x5_sRGB
    COLOR_FORMAT(16, 6, 5),        // ASTC_RGBA_6x5_sRGB
    COLOR_FORMAT(16, 6, 6),        // ASTC_RGBA_6x6_sRGB
    COLOR_FORMAT(16, 8, 5),        // ASTC_RGBA_8x5_sRGB
    COLOR_FORMAT(16, 8, 6),        // ASTC_RGBA_8x6_sRGB
    COLOR_FORMAT(16, 8, 8),        // ASTC_RGBA_8x8_sRGB
    COLOR_FORMAT(16, 10, 5),       // ASTC_RGBA_10x5_sRGB
    COLOR_FORMAT(16, 10, 6),       // ASTC_RGBA_10x6_sRGB
    COLOR_FORMAT(16, 10, 8),       // ASTC_RGBA_10x8_sRGB
    COLOR_FORMAT(16, 10, 10),      // ASTC_RGBA_10x10_sRGB
    COLOR_FORMAT(16, 12, 10),      // ASTC_RGBA_12x10_sRGB
    COLOR_FORMAT(16, 12, 12),      // ASTC_RGBA_12x12_sRGB
    COLOR_FORMAT(2, 1, 1),         // B5G6R5Unorm
    COLOR_FORMAT(2, 1, 1),         // BGR5Unorm
    COLOR_FORMAT(2, 1, 1),         // BGR5A1Unorm
    COLOR_FORMAT(2, 1, 1),         // A1BGR5Unorm
    COLOR_FORMAT(4, 1, 1),         // BGRX8Unorm
    COLOR_FORMAT(4, 1, 1),         // BGRA8Unorm
    COLOR_FORMAT(4, 1, 1),         // BGRX8Unorm_sRGB
    COLOR_FORMAT(4, 1, 1),         // BGRA8Unorm_sRGB
    COLOR_FORMAT(8, 4, 4),         // ETC2_R_Unorm
    COLOR_FORMAT(8, 4, 4),         // ETC2_R_Snorm
    COLOR_FORMAT(16, 4, 4),        // ETC2_RG_Unorm
    COLOR_FORMAT(16, 4, 4),        // ETC2_RG_Snorm
    COLOR_FORMAT(8, 4, 4),         // ETC2_RGB
    COLOR_FORMAT(8, 4, 4),         // PTA_ETC2_RGB
    COLOR_FORMAT(16, 4, 4),        // ETC2_RGBA
    COLOR_FORMAT(8, 4, 4),         // ETC2_RGB_sRGB
    COLOR_FORMAT(8, 4, 4),         // PTA_ETC2_RGB_sRGB
    COLOR_FORMAT(16, 4, 4),        // ETC2_RGBA_sRGB
};

#undef DEPTH_STENCIL_FORMAT
#undef COLOR_FORMAT
#undef FORMAT

enum class TextureTypeCompatibility {
    _1D,
    _1DBuffer,
    _2D,
    _3D,
    Cube,
};

static TextureTypeCompatibility ToTextureTypeCompatibility(TextureType type) {
    switch (type) {
    case TextureType::_1D:
    case TextureType::_1DArray:
        return TextureTypeCompatibility::_1D;
    case TextureType::_1DBuffer:
        return TextureTypeCompatibility::_1DBuffer;
    case TextureType::_2D:
    case TextureType::_2DArray:
    case TextureType::_3D: // TODO: 2D arrays aren't compatible with 3D
        return TextureTypeCompatibility::_2D;
    case TextureType::Cube:
    case TextureType::CubeArray:
        return TextureTypeCompatibility::Cube;
    }
}

} // namespace

const TextureFormatInfo& GetTextureFormatInfo(TextureFormat format) {
    return texture_format_infos[static_cast<u32>(format)];
}

TextureFormat to_texture_format(NvColorFormat color_format) {
#define NV_COLOR_FORMAT_CASE(color_format, texture_format)                     \
    case NvColorFormat::color_format:                                          \
        return TextureFormat::texture_format;

    // TODO: check
    switch (color_format) {
        NV_COLOR_FORMAT_CASE(A8B8G8R8, RGBA8Unorm)
        NV_COLOR_FORMAT_CASE(X8B8G8R8, RGBX8Unorm)
        NV_COLOR_FORMAT_CASE(R8G8B8, Invalid) // Doesn't work
        NV_COLOR_FORMAT_CASE(R5G6B5, B5G6R5Unorm)
        // NV_COLOR_FORMAT_CASE(B5G6R5, R5G6B5Unorm)
        NV_COLOR_FORMAT_CASE(A8R8G8B8, BGRA8Unorm)
        NV_COLOR_FORMAT_CASE(R5G5B5A1, A1BGR5Unorm) // Doesn't work
        NV_COLOR_FORMAT_CASE(A4B4G4R4, RGBA4Unorm)
    default:
        LOG_NOT_IMPLEMENTED(Gpu, "NV color format {}", color_format);
        return TextureFormat::Invalid;
    }

#undef NV_COLOR_FORMAT_CASE
}

TextureFormat to_texture_format(const ImageFormatWord image_format_word,
                                bool is_srgb) {
#define IMAGE_FORMAT_CASE_IMPL(img_format, c_r, c_g, c_b, c_a, texture_format, \
                               is_srgb_)                                       \
    else if (image_format_word.image_format == ImageFormat::img_format &&      \
             image_format_word.component_r == ImageComponent::c_r &&           \
             image_format_word.component_g == ImageComponent::c_g &&           \
             image_format_word.component_b == ImageComponent::c_b &&           \
             image_format_word.component_a == ImageComponent::c_a &&           \
             is_srgb == is_srgb_) return TextureFormat::texture_format;

#define IMAGE_FORMAT_CASE(img_format, c_r, c_g, c_b, c_a, texture_format)      \
    IMAGE_FORMAT_CASE_IMPL(img_format, c_r, c_g, c_b, c_a, texture_format,     \
                           false)
#define IMAGE_FORMAT_CASE_SRGB(img_format, c_r, c_g, c_b, c_a, texture_format) \
    IMAGE_FORMAT_CASE_IMPL(img_format, c_r, c_g, c_b, c_a, texture_format, true)

    // TODO: more formats
    // TODO: check
    if (image_format_word.image_format == ImageFormat::Invalid)
        return TextureFormat::Invalid;
    IMAGE_FORMAT_CASE(R16, Float, Float, Float, Float, R16Float)
    IMAGE_FORMAT_CASE(R32, Float, Float, Float, Float, R32Float)
    IMAGE_FORMAT_CASE(ARGB8, Unorm, Unorm, Unorm, Unorm, RGBA8Unorm)
    IMAGE_FORMAT_CASE_SRGB(ARGB8, Unorm, Unorm, Unorm, Unorm, RGBA8Unorm_sRGB)
    IMAGE_FORMAT_CASE(R8, Unorm, Unorm, Unorm, Unorm, R8Unorm)
    IMAGE_FORMAT_CASE(R16, Unorm, Unorm, Unorm, Unorm, R16Unorm)
    IMAGE_FORMAT_CASE(GR8, Unorm, Unorm, Unorm, Unorm, RG8Unorm)
    IMAGE_FORMAT_CASE(GR8, Snorm, Snorm, Snorm, Snorm, RG8Snorm)
    IMAGE_FORMAT_CASE(RG16, Unorm, Unorm, Unorm, Unorm, RG16Unorm)
    IMAGE_FORMAT_CASE(RG16, Snorm, Snorm, Snorm, Snorm, RG16Snorm)
    IMAGE_FORMAT_CASE(RG16, Uint, Uint, Uint, Uint, RG16Uint)
    IMAGE_FORMAT_CASE(RG16, Sint, Sint, Sint, Sint, RG16Sint)
    IMAGE_FORMAT_CASE(RG16, Float, Float, Float, Float, RG16Float)
    IMAGE_FORMAT_CASE(DXT1, Unorm, Unorm, Unorm, Unorm, BC1_RGB)
    IMAGE_FORMAT_CASE_SRGB(DXT1, Unorm, Unorm, Unorm, Unorm, BC1_RGB_sRGB)
    IMAGE_FORMAT_CASE(DXT23, Unorm, Unorm, Unorm, Unorm, BC2_RGBA)
    IMAGE_FORMAT_CASE_SRGB(DXT23, Unorm, Unorm, Unorm, Unorm, BC2_RGBA_sRGB)
    IMAGE_FORMAT_CASE(DXT45, Unorm, Unorm, Unorm, Unorm, BC3_RGBA)
    IMAGE_FORMAT_CASE_SRGB(DXT45, Unorm, Unorm, Unorm, Unorm, BC3_RGBA_sRGB)
    IMAGE_FORMAT_CASE(DXN2, Unorm, Unorm, Unorm, Unorm, BC5_RGUnorm)
    IMAGE_FORMAT_CASE(DXN2, Snorm, Snorm, Snorm, Snorm, BC5_RGSnorm)
    IMAGE_FORMAT_CASE(B5G6R5, Unorm, Unorm, Unorm, Unorm, B5G6R5Unorm)
    IMAGE_FORMAT_CASE(ABGR4, Unorm, Unorm, Unorm, Unorm, RGBA4Unorm)
    IMAGE_FORMAT_CASE(A1BGR5, Unorm, Unorm, Unorm, Unorm, A1BGR5Unorm)
    IMAGE_FORMAT_CASE(B10GR11Float, Float, Float, Float, Float, RG11B10Float)
    IMAGE_FORMAT_CASE(A2BGR10, Unorm, Unorm, Unorm, Unorm, RGB10A2Unorm)
    IMAGE_FORMAT_CASE(ASTC_2D_4X4, Unorm, Unorm, Unorm, Unorm, ASTC_RGBA_4x4)
    IMAGE_FORMAT_CASE_SRGB(ASTC_2D_4X4, Unorm, Unorm, Unorm, Unorm,
                           ASTC_RGBA_4x4_sRGB)
    IMAGE_FORMAT_CASE(DXN1, Unorm, Unorm, Unorm, Unorm, BC4_RUnorm)
    IMAGE_FORMAT_CASE(Z24S8, Uint, Unorm, Unorm, Unorm, Z24Unorm_S8Uint)
    IMAGE_FORMAT_CASE(Z16, Unorm, Unorm, Unorm, Unorm, Z16Unorm)
    IMAGE_FORMAT_CASE(ZF32, Float, Float, Float, Float, Z32Float)
    IMAGE_FORMAT_CASE(ZF32_X24S8, Float, Uint, Unorm, Unorm, Z32Float_X24S8Uint)
    IMAGE_FORMAT_CASE(RGBA16, Float, Float, Float, Float, RGBA16Float)
    IMAGE_FORMAT_CASE(RGBA32, Float, Float, Float, Float, RGBA32Float)
    IMAGE_FORMAT_CASE(BC7U, Unorm, Unorm, Unorm, Unorm, BC7_RGBAUnorm)
    else {
        LOG_FATAL(Gpu, "Image format {}, components: {}, {}, {}, {}, sRGB: {}",
                  image_format_word.image_format, image_format_word.component_r,
                  image_format_word.component_g, image_format_word.component_b,
                  image_format_word.component_a, is_srgb);
    }

#undef IMAGE_FORMAT_CASE
}

TextureFormat to_texture_format(ColorSurfaceFormat color_surface_format) {
#define COLOR_SURFACE_FORMAT_CASE(color_surface_format, texture_format)        \
    case ColorSurfaceFormat::color_surface_format:                             \
        return TextureFormat::texture_format;

    switch (color_surface_format) {
        COLOR_SURFACE_FORMAT_CASE(Invalid, Invalid)
        COLOR_SURFACE_FORMAT_CASE(Bitmap, Invalid)
        COLOR_SURFACE_FORMAT_CASE(Unknown1D, Invalid)
        COLOR_SURFACE_FORMAT_CASE(RGBA32Float, RGBA32Float)
        COLOR_SURFACE_FORMAT_CASE(RGBA32Sint, RGBA32Sint)
        COLOR_SURFACE_FORMAT_CASE(RGBA32Uint, RGBA32Uint)
        COLOR_SURFACE_FORMAT_CASE(RGBX32Float, RGBX32Float)
        COLOR_SURFACE_FORMAT_CASE(RGBX32Sint, RGBX32Sint)
        COLOR_SURFACE_FORMAT_CASE(RGBX32Uint, RGBX32Uint)
        COLOR_SURFACE_FORMAT_CASE(RGBA16Unorm, RGBA16Unorm)
        COLOR_SURFACE_FORMAT_CASE(RGBA16Snorm, RGBA16Snorm)
        COLOR_SURFACE_FORMAT_CASE(RGBA16Sint, RGBA16Sint)
        COLOR_SURFACE_FORMAT_CASE(RGBA16Uint, RGBA16Uint)
        COLOR_SURFACE_FORMAT_CASE(RGBA16Float, RGBA16Float)
        COLOR_SURFACE_FORMAT_CASE(RG32Float, RG32Float)
        COLOR_SURFACE_FORMAT_CASE(RG32Sint, RG32Sint)
        COLOR_SURFACE_FORMAT_CASE(RG32Uint, RG32Uint)
        COLOR_SURFACE_FORMAT_CASE(RGBX16Float, RGBX16Float)
        COLOR_SURFACE_FORMAT_CASE(BGRA8Unorm, BGRA8Unorm)
        COLOR_SURFACE_FORMAT_CASE(BGRA8Unorm_sRGB, BGRA8Unorm_sRGB)
        COLOR_SURFACE_FORMAT_CASE(RGB10A2Unorm, RGB10A2Unorm)
        COLOR_SURFACE_FORMAT_CASE(RGB10A2Uint, RGB10A2Uint)
        COLOR_SURFACE_FORMAT_CASE(RGBA8Unorm, RGBA8Unorm)
        COLOR_SURFACE_FORMAT_CASE(RGBA8Unorm_sRGB, RGBA8Unorm_sRGB)
        COLOR_SURFACE_FORMAT_CASE(RGBA8Snorm, RGBA8Snorm)
        COLOR_SURFACE_FORMAT_CASE(RGBA8Sint, RGBA8Sint)
        COLOR_SURFACE_FORMAT_CASE(RGBA8Uint, RGBA8Uint)
        COLOR_SURFACE_FORMAT_CASE(RG16Unorm, RG16Unorm)
        COLOR_SURFACE_FORMAT_CASE(RG16Snorm, RG16Snorm)
        COLOR_SURFACE_FORMAT_CASE(RG16Sint, RG16Sint)
        COLOR_SURFACE_FORMAT_CASE(RG16Uint, RG16Uint)
        COLOR_SURFACE_FORMAT_CASE(RG16Float, RG16Float)
        COLOR_SURFACE_FORMAT_CASE(BGR10A2Unorm, RGB10A2Unorm) // TODO: correct?
        COLOR_SURFACE_FORMAT_CASE(RG11B10Float, RG11B10Float)
        COLOR_SURFACE_FORMAT_CASE(R32Sint, R32Sint)
        COLOR_SURFACE_FORMAT_CASE(R32Uint, R32Uint)
        COLOR_SURFACE_FORMAT_CASE(R32Float, R32Float)
        COLOR_SURFACE_FORMAT_CASE(BGRX8Unorm, BGRX8Unorm)
        COLOR_SURFACE_FORMAT_CASE(BGRX8Unorm_sRGB, BGRX8Unorm_sRGB)
        COLOR_SURFACE_FORMAT_CASE(B5G6R5Unorm, B5G6R5Unorm)
        COLOR_SURFACE_FORMAT_CASE(BGR5A1Unorm, BGR5A1Unorm)
        COLOR_SURFACE_FORMAT_CASE(RG8Unorm, RG8Unorm)
        COLOR_SURFACE_FORMAT_CASE(RG8Snorm, RG8Snorm)
        COLOR_SURFACE_FORMAT_CASE(RG8Sint, RG8Sint)
        COLOR_SURFACE_FORMAT_CASE(RG8Uint, RG8Uint)
        COLOR_SURFACE_FORMAT_CASE(R16Unorm, R16Unorm)
        COLOR_SURFACE_FORMAT_CASE(R16Snorm, R16Snorm)
        COLOR_SURFACE_FORMAT_CASE(R16Sint, R16Sint)
        COLOR_SURFACE_FORMAT_CASE(R16Uint, R16Uint)
        COLOR_SURFACE_FORMAT_CASE(R16Float, R16Float)
        COLOR_SURFACE_FORMAT_CASE(R8Unorm, R8Unorm)
        COLOR_SURFACE_FORMAT_CASE(R8Snorm, R8Snorm)
        COLOR_SURFACE_FORMAT_CASE(R8Sint, R8Sint)
        COLOR_SURFACE_FORMAT_CASE(R8Uint, R8Uint)
        COLOR_SURFACE_FORMAT_CASE(A8Unorm, Invalid)     // TODO
        COLOR_SURFACE_FORMAT_CASE(BGR5X1Unorm, Invalid) // TODO
        COLOR_SURFACE_FORMAT_CASE(RGBX8Unorm, RGBX8Unorm)
        COLOR_SURFACE_FORMAT_CASE(RGBX8Unorm_sRGB, RGBX8Unorm_sRGB)
        COLOR_SURFACE_FORMAT_CASE(BGR5X1UnormUnknownFB, Invalid)
        COLOR_SURFACE_FORMAT_CASE(BGR5X1UnormUnknownFC, Invalid)
        COLOR_SURFACE_FORMAT_CASE(BGRX8UnormUnknownFD, Invalid)
        COLOR_SURFACE_FORMAT_CASE(BGRX8UnormUnknownFE, Invalid)
        COLOR_SURFACE_FORMAT_CASE(Y32UintUnknownFF, Invalid)
    default:
        LOG_FATAL(Gpu, "Color surface format {}", color_surface_format);
    }

#undef COLOR_SURFACE_FORMAT_CASE
}

TextureFormat to_texture_format(DepthSurfaceFormat depth_surface_format) {
#define DEPTH_SURFACE_FORMAT_CASE(depth_surface_format, texture_format)        \
    case DepthSurfaceFormat::depth_surface_format:                             \
        return TextureFormat::texture_format;

    switch (depth_surface_format) {
        DEPTH_SURFACE_FORMAT_CASE(Invalid, Invalid)
        DEPTH_SURFACE_FORMAT_CASE(Z32Float, Z32Float)
        DEPTH_SURFACE_FORMAT_CASE(Z16Unorm, Z16Unorm)
        DEPTH_SURFACE_FORMAT_CASE(S8Z24Unorm, Z24Unorm_S8Uint) // TODO
        DEPTH_SURFACE_FORMAT_CASE(Z24S8Unorm, Z24Unorm_S8Uint)
        DEPTH_SURFACE_FORMAT_CASE(S8Uint, S8Uint)
        DEPTH_SURFACE_FORMAT_CASE(Z24C8Unorm, Invalid)
        DEPTH_SURFACE_FORMAT_CASE(Z32S8X24Float, Z32Float_X24S8Uint) // TODO
        DEPTH_SURFACE_FORMAT_CASE(Z24X8S8C8X16Unorm, Invalid)
        DEPTH_SURFACE_FORMAT_CASE(Z32X8C8X16Float, Invalid)
        DEPTH_SURFACE_FORMAT_CASE(Z32S8C8X16Float, Invalid)
    default:
        LOG_FATAL(Gpu, "Depth surface format {}", depth_surface_format);
    }

#undef DEPTH_SURFACE_FORMAT_CASE
}

u32 get_texture_format_bpp(const TextureFormat format) {
    const auto& info = GetTextureFormatInfo(format);
    if (info.block_width != 1 || info.block_height != 1)
        throw GetTextureFormatBppError::UnsupportedFormatForBpp;

    return info.bytes_per_block;
}

u32 get_texture_format_stride(const TextureFormat format, u32 width) {
    const auto& info = GetTextureFormatInfo(format);
    return ceil_divide(width, info.block_width) * info.bytes_per_block;
}

bool is_texture_format_compressed(const TextureFormat format) {
    const auto& info = GetTextureFormatInfo(format);
    return info.block_width != 1 || info.block_height != 1;
}

bool is_texture_format_depth_or_stencil(const TextureFormat format) {
    const auto& info = GetTextureFormatInfo(format);
    return info.is_depth_stencil;
}

ColorDataType to_color_data_type(ColorSurfaceFormat format) {
    switch (format) {
    case ColorSurfaceFormat::RGBA32Float:
    case ColorSurfaceFormat::RGBX32Float:
    case ColorSurfaceFormat::RGBA16Unorm:
    case ColorSurfaceFormat::RGBA16Snorm:
    case ColorSurfaceFormat::RGBA16Float:
    case ColorSurfaceFormat::RG32Float:
    case ColorSurfaceFormat::RGBX16Float:
    case ColorSurfaceFormat::BGRA8Unorm:
    case ColorSurfaceFormat::BGRA8Unorm_sRGB:
    case ColorSurfaceFormat::RGB10A2Unorm:
    case ColorSurfaceFormat::RGBA8Unorm:
    case ColorSurfaceFormat::RGBA8Unorm_sRGB:
    case ColorSurfaceFormat::RGBA8Snorm:
    case ColorSurfaceFormat::RG16Unorm:
    case ColorSurfaceFormat::RG16Snorm:
    case ColorSurfaceFormat::RG16Float:
    case ColorSurfaceFormat::BGR10A2Unorm:
    case ColorSurfaceFormat::RG11B10Float:
    case ColorSurfaceFormat::R32Float:
    case ColorSurfaceFormat::BGRX8Unorm:
    case ColorSurfaceFormat::BGRX8Unorm_sRGB:
    case ColorSurfaceFormat::B5G6R5Unorm:
    case ColorSurfaceFormat::BGR5A1Unorm:
    case ColorSurfaceFormat::RG8Unorm:
    case ColorSurfaceFormat::RG8Snorm:
    case ColorSurfaceFormat::R16Unorm:
    case ColorSurfaceFormat::R16Snorm:
    case ColorSurfaceFormat::R16Float:
    case ColorSurfaceFormat::R8Unorm:
    case ColorSurfaceFormat::R8Snorm:
    case ColorSurfaceFormat::A8Unorm:
    case ColorSurfaceFormat::BGR5X1Unorm:
    case ColorSurfaceFormat::RGBX8Unorm:
    case ColorSurfaceFormat::RGBX8Unorm_sRGB:
        return ColorDataType::Float;
    case ColorSurfaceFormat::RGBA32Sint:
    case ColorSurfaceFormat::RGBX32Sint:
    case ColorSurfaceFormat::RGBA16Sint:
    case ColorSurfaceFormat::RG32Sint:
    case ColorSurfaceFormat::RGBA8Sint:
    case ColorSurfaceFormat::RG16Sint:
    case ColorSurfaceFormat::R32Sint:
    case ColorSurfaceFormat::RG8Sint:
    case ColorSurfaceFormat::R16Sint:
    case ColorSurfaceFormat::R8Sint:
        return ColorDataType::Int;
    case ColorSurfaceFormat::RGBA32Uint:
    case ColorSurfaceFormat::RGBX32Uint:
    case ColorSurfaceFormat::RGBA16Uint:
    case ColorSurfaceFormat::RG32Uint:
    case ColorSurfaceFormat::RGB10A2Uint:
    case ColorSurfaceFormat::RGBA8Uint:
    case ColorSurfaceFormat::RG16Uint:
    case ColorSurfaceFormat::R32Uint:
    case ColorSurfaceFormat::RG8Uint:
    case ColorSurfaceFormat::R16Uint:
    case ColorSurfaceFormat::R8Uint:
        return ColorDataType::UInt;
    default:
        return ColorDataType::Invalid;
    }
}

// TODO: check the logic of this
SwizzleChannels::SwizzleChannels(const TextureFormat format,
                                 const ImageSwizzle x, const ImageSwizzle y,
                                 const ImageSwizzle z, const ImageSwizzle w) {
#define SWIZZLE(r_, g_, b_, a_)                                                \
    {                                                                          \
        r = r_;                                                                \
        g = g_;                                                                \
        b = b_;                                                                \
        a = a_;                                                                \
        break;                                                                 \
    }

    switch (format) {
    // TODO: fix these
    case TextureFormat::R8Unorm:
    case TextureFormat::R8Snorm:
    case TextureFormat::R8Uint:
    case TextureFormat::R8Sint:
    case TextureFormat::R16Float:
    case TextureFormat::R16Unorm:
    case TextureFormat::R16Snorm:
    case TextureFormat::R16Uint:
    case TextureFormat::R16Sint:
    case TextureFormat::R32Float:
    case TextureFormat::R32Uint:
    case TextureFormat::R32Sint:
    case TextureFormat::RG8Unorm:
    case TextureFormat::RG8Snorm:
    case TextureFormat::RG8Uint:
    case TextureFormat::RG8Sint:
    case TextureFormat::RG16Float:
    case TextureFormat::RG16Unorm:
    case TextureFormat::RG16Snorm:
    case TextureFormat::RG16Uint:
    case TextureFormat::RG16Sint:
    case TextureFormat::RG32Float:
    case TextureFormat::RG32Uint:
    case TextureFormat::RG32Sint:
    case TextureFormat::RGB32Float:
    case TextureFormat::RGB32Uint:
    case TextureFormat::RGB32Sint:
    case TextureFormat::RGBA8Unorm:
    case TextureFormat::RGBA8Snorm:
    case TextureFormat::RGBA8Uint:
    case TextureFormat::RGBA8Sint:
    case TextureFormat::RGBA8Unorm_sRGB:
    case TextureFormat::RGBX8Unorm:
    case TextureFormat::RGBX8Snorm:
    case TextureFormat::RGBX8Uint:
    case TextureFormat::RGBX8Sint:
    case TextureFormat::RGBX8Unorm_sRGB:
    case TextureFormat::RGBA16Float:
    case TextureFormat::RGBA16Unorm:
    case TextureFormat::RGBA16Snorm:
    case TextureFormat::RGBA16Uint:
    case TextureFormat::RGBA16Sint:
    case TextureFormat::RGBX16Float:
    case TextureFormat::RGBX16Unorm:
    case TextureFormat::RGBX16Snorm:
    case TextureFormat::RGBX16Uint:
    case TextureFormat::RGBX16Sint:
    case TextureFormat::RGBA32Float:
    case TextureFormat::RGBA32Uint:
    case TextureFormat::RGBA32Sint:
    case TextureFormat::RGBX32Float:
    case TextureFormat::RGBX32Uint:
    case TextureFormat::RGBX32Sint:
    case TextureFormat::RGBA4Unorm:
    case TextureFormat::RGB5Unorm:
    case TextureFormat::RGB5A1Unorm:
    case TextureFormat::R5G6B5Unorm:
    case TextureFormat::RGB10A2Unorm:
    case TextureFormat::RGB10A2Uint:
    case TextureFormat::RG11B10Float:
    case TextureFormat::E5BGR9Float:
    case TextureFormat::BC1_RGB:
    case TextureFormat::BC1_RGBA:
    case TextureFormat::BC1_RGB_sRGB:
    case TextureFormat::BC1_RGBA_sRGB:
    case TextureFormat::BC2_RGBA:
    case TextureFormat::BC3_RGBA:
    case TextureFormat::BC2_RGBA_sRGB:
    case TextureFormat::BC3_RGBA_sRGB:
    case TextureFormat::BC4_RUnorm:
    case TextureFormat::BC4_RSnorm:
    case TextureFormat::BC5_RGUnorm:
    case TextureFormat::BC5_RGSnorm:
    case TextureFormat::BC7_RGBAUnorm:
    case TextureFormat::BC7_RGBAUnorm_sRGB:
    case TextureFormat::BC6H_RGBA_SF16_Float:
    case TextureFormat::BC6H_RGBA_UF16_Float:
    case TextureFormat::ASTC_RGBA_4x4:
    case TextureFormat::ASTC_RGBA_4x4_sRGB:
    case TextureFormat::ASTC_RGBA_5x4:
    case TextureFormat::ASTC_RGBA_5x4_sRGB:
    case TextureFormat::ASTC_RGBA_5x5:
    case TextureFormat::ASTC_RGBA_5x5_sRGB:
    case TextureFormat::ASTC_RGBA_6x5:
    case TextureFormat::ASTC_RGBA_6x5_sRGB:
    case TextureFormat::ASTC_RGBA_6x6:
    case TextureFormat::ASTC_RGBA_6x6_sRGB:
    case TextureFormat::ASTC_RGBA_8x5:
    case TextureFormat::ASTC_RGBA_8x5_sRGB:
    case TextureFormat::ASTC_RGBA_8x6:
    case TextureFormat::ASTC_RGBA_8x6_sRGB:
    case TextureFormat::ASTC_RGBA_8x8:
    case TextureFormat::ASTC_RGBA_8x8_sRGB:
    case TextureFormat::ASTC_RGBA_10x5:
    case TextureFormat::ASTC_RGBA_10x5_sRGB:
    case TextureFormat::ASTC_RGBA_10x6:
    case TextureFormat::ASTC_RGBA_10x6_sRGB:
    case TextureFormat::ASTC_RGBA_10x8:
    case TextureFormat::ASTC_RGBA_10x8_sRGB:
    case TextureFormat::ASTC_RGBA_10x10:
    case TextureFormat::ASTC_RGBA_10x10_sRGB:
    case TextureFormat::ASTC_RGBA_12x10:
    case TextureFormat::ASTC_RGBA_12x10_sRGB:
    case TextureFormat::ASTC_RGBA_12x12:
    case TextureFormat::ASTC_RGBA_12x12_sRGB:
    case TextureFormat::A1BGR5Unorm:
    case TextureFormat::ETC2_R_Unorm:
    case TextureFormat::ETC2_R_Snorm:
    case TextureFormat::ETC2_RG_Unorm:
    case TextureFormat::ETC2_RG_Snorm:
    case TextureFormat::ETC2_RGB:
    case TextureFormat::PTA_ETC2_RGB:
    case TextureFormat::ETC2_RGB_sRGB:
    case TextureFormat::PTA_ETC2_RGB_sRGB:
    case TextureFormat::ETC2_RGBA:
    case TextureFormat::ETC2_RGBA_sRGB:
    // TODO: depth stencil correct?
    case TextureFormat::S8Uint:
    case TextureFormat::Z16Unorm:
    case TextureFormat::Z24Unorm_X8Uint:
    case TextureFormat::Z32Float:
    case TextureFormat::Z24Unorm_S8Uint:
    case TextureFormat::Z32Float_X24S8Uint:
        SWIZZLE(x, y, z, w);
    case TextureFormat::BGR5A1Unorm:
    case TextureFormat::BGRA8Unorm:
    case TextureFormat::BGRA8Unorm_sRGB:
        SWIZZLE(z, y, x, w);
    case TextureFormat::B5G6R5Unorm:
    case TextureFormat::BGR5Unorm:
    case TextureFormat::BGRX8Unorm:
    case TextureFormat::BGRX8Unorm_sRGB:
        SWIZZLE(z, y, x, ImageSwizzle::OneFloat);
    default:
        LOG_NOT_IMPLEMENTED(Gpu, "Swizzle for format {}", format);
        break;
    }

#undef SWIZZLE
}

SwizzleChannels
get_texture_format_default_swizzle_channels(const TextureFormat format) {
    if (is_texture_format_depth_or_stencil(format))
        return {ImageSwizzle::Zero, ImageSwizzle::Zero, ImageSwizzle::Zero,
                ImageSwizzle::Zero};

#define SWIZZLE(r, g, b, a)                                                    \
    SwizzleChannels(ImageSwizzle::r, ImageSwizzle::g, ImageSwizzle::b,         \
                    ImageSwizzle::a)

    // TODO: implement all formats
    switch (format) {
    case TextureFormat::R8Unorm:
        return SWIZZLE(R, Zero, Zero, OneFloat);
    case TextureFormat::B5G6R5Unorm:
    case TextureFormat::BC1_RGB:
        return SWIZZLE(R, G, B, OneFloat);
    case TextureFormat::RG8Unorm:
        return SWIZZLE(R, G, Zero, OneFloat);
    case TextureFormat::RGBA8Unorm:
    case TextureFormat::BGRA8Unorm:
    case TextureFormat::RGBA8Unorm_sRGB:
    case TextureFormat::RGB10A2Unorm:
    case TextureFormat::BC2_RGBA:
    case TextureFormat::BC3_RGBA:
        return SWIZZLE(R, G, B, A);
    default:
        ONCE(LOG_NOT_IMPLEMENTED(Gpu, "{} default swizzle", format));
        return SWIZZLE(R, G, B, A);
    }

#undef SWIZZLE
}

u32 TextureDescriptor::GetHash() const {
    HashCode hash;
    hash.Add(ptr);
    hash.Add(width);
    hash.Add(height);
    hash.Add(depth);
    // TODO
    // hash.Add(level_count);
    hash.Add(layer_count);
    hash.Add(stride);

    hash.Add(ToTextureTypeCompatibility(type));

    // TODO: get format info from the renderer instead
    hash.Add(is_texture_format_compressed(format));
    hash.Add(is_texture_format_depth_or_stencil(format));
    hash.Add(get_texture_format_stride(format, 16));

    return hash.ToHashCode();
}

u32 TextureViewDescriptor::GetHash() const {
    HashCode hash;
    hash.Add(format);
    hash.Add(swizzle_channels.r);
    hash.Add(swizzle_channels.g);
    hash.Add(swizzle_channels.b);
    hash.Add(swizzle_channels.a);
    hash.Add(levels.GetBegin());
    hash.Add(levels.GetEnd());
    hash.Add(layers.GetBegin());
    hash.Add(layers.GetEnd());

    return hash.ToHashCode();
}

usize get_vertex_format_size(engines::VertexAttribSize size) {
    switch (size) {
    case engines::VertexAttribSize::_1x32:
        return 4;
    case engines::VertexAttribSize::_2x32:
        return 8;
    case engines::VertexAttribSize::_3x32:
        return 12;
    case engines::VertexAttribSize::_4x32:
        return 16;

    case engines::VertexAttribSize::_1x16:
        return 2;
    case engines::VertexAttribSize::_2x16:
        return 4;
    case engines::VertexAttribSize::_3x16:
        return 6;
    case engines::VertexAttribSize::_4x16:
        return 8;

    case engines::VertexAttribSize::_1x8:
        return 1;
    case engines::VertexAttribSize::_2x8:
        return 2;
    case engines::VertexAttribSize::_3x8:
        return 3;
    case engines::VertexAttribSize::_4x8:
        return 4;

    case engines::VertexAttribSize::_10_10_10_2:
    case engines::VertexAttribSize::_11_11_10:
        return 4;
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
