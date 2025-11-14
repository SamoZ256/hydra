#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

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

TextureFormat to_texture_format(const ImageFormatWord image_format_word) {
#define IMAGE_FORMAT_CASE(img_format, c_r, c_g, c_b, c_a, texture_format)      \
    else if (image_format_word.image_format == ImageFormat::img_format &&      \
             image_format_word.component_r == ImageComponent::c_r &&           \
             image_format_word.component_g == ImageComponent::c_g &&           \
             image_format_word.component_b == ImageComponent::c_b &&           \
             image_format_word.component_a ==                                  \
                 ImageComponent::c_a) return TextureFormat::texture_format;

    // TODO: more formats
    if (image_format_word.image_format == ImageFormat::Invalid)
        return TextureFormat::Invalid;
    IMAGE_FORMAT_CASE(R16, Float, Float, Float, Float, R16Float)
    IMAGE_FORMAT_CASE(R32, Float, Float, Float, Float, R32Float)
    IMAGE_FORMAT_CASE(ARGB8, Unorm, Unorm, Unorm, Unorm,
                      RGBA8Unorm) // TODO: why argb?
    IMAGE_FORMAT_CASE(R8, Unorm, Unorm, Unorm, Unorm, R8Unorm)
    IMAGE_FORMAT_CASE(R16, Unorm, Unorm, Unorm, Unorm, R16Unorm)
    IMAGE_FORMAT_CASE(GR8, Unorm, Unorm, Unorm, Unorm,
                      RG8Unorm) // TODO: correct?
    IMAGE_FORMAT_CASE(RG16, Unorm, Unorm, Unorm, Unorm, RG16Unorm)
    IMAGE_FORMAT_CASE(RG16, Snorm, Snorm, Snorm, Snorm, RG16Snorm)
    IMAGE_FORMAT_CASE(RG16, Uint, Uint, Uint, Uint, RG16Uint)
    IMAGE_FORMAT_CASE(RG16, Sint, Sint, Sint, Sint, RG16Sint)
    IMAGE_FORMAT_CASE(RG16, Float, Float, Float, Float, RG16Float)
    IMAGE_FORMAT_CASE(DXT1, Unorm, Unorm, Unorm, Unorm, BC1_RGB)
    IMAGE_FORMAT_CASE(DXT23, Unorm, Unorm, Unorm, Unorm, BC2_RGBA)
    IMAGE_FORMAT_CASE(DXT45, Unorm, Unorm, Unorm, Unorm, BC3_RGBA)
    IMAGE_FORMAT_CASE(DXN2, Unorm, Unorm, Unorm, Unorm, BC5_RGUnorm)
    IMAGE_FORMAT_CASE(DXN2, Snorm, Snorm, Snorm, Snorm, BC5_RGSnorm)
    IMAGE_FORMAT_CASE(B5G6R5, Unorm, Unorm, Unorm, Unorm, B5G6R5Unorm)
    IMAGE_FORMAT_CASE(ABGR4, Unorm, Unorm, Unorm, Unorm,
                      RGBA4Unorm) // TODO: correct?
    IMAGE_FORMAT_CASE(A1BGR5, Unorm, Unorm, Unorm, Unorm, A1BGR5Unorm)
    IMAGE_FORMAT_CASE(B10GR11Float, Float, Float, Float, Float, RG11B10Float)
    IMAGE_FORMAT_CASE(A2BGR10, Unorm, Unorm, Unorm, Unorm, RGB10A2Unorm)
    IMAGE_FORMAT_CASE(ASTC_2D_4X4, Unorm, Unorm, Unorm, Unorm, ASTC_RGBA_4x4)
    IMAGE_FORMAT_CASE(DXN1, Unorm, Unorm, Unorm, Unorm, BC4_RUnorm)
    IMAGE_FORMAT_CASE(Z24S8, Uint, Unorm, Unorm, Unorm, Z24Unorm_S8Uint)
    IMAGE_FORMAT_CASE(Z16, Unorm, Unorm, Unorm, Unorm, Z16Unorm)
    IMAGE_FORMAT_CASE(ZF32, Float, Float, Float, Float, Z32Float)
    IMAGE_FORMAT_CASE(ZF32_X24S8, Float, Uint, Unorm, Unorm, Z32Float_X24S8Uint)
    IMAGE_FORMAT_CASE(RGBA16, Float, Float, Float, Float, RGBA16Float)
    IMAGE_FORMAT_CASE(RGBA32, Float, Float, Float, Float, RGBA32Float)
    IMAGE_FORMAT_CASE(BC7U, Unorm, Unorm, Unorm, Unorm, BC7_RGBAUnorm)
    else {
        LOG_NOT_IMPLEMENTED(
            Gpu, "Image format {}, components: {}, {}, {}, {}",
            image_format_word.image_format, image_format_word.component_r,
            image_format_word.component_g, image_format_word.component_b,
            image_format_word.component_a);
        throw;
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
        LOG_NOT_IMPLEMENTED(Gpu, "Color surface format {}",
                            color_surface_format);
        return TextureFormat::Invalid;
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
        LOG_NOT_IMPLEMENTED(Gpu, "Depth surface format {}",
                            depth_surface_format);
        // TODO: don't throw
        throw;
        return TextureFormat::Invalid;
    }

#undef DEPTH_SURFACE_FORMAT_CASE
}

usize get_texture_format_stride(const TextureFormat format, usize width) {
    // TODO: check this
    switch (format) {
    case TextureFormat::Invalid:
        return 0;
    case TextureFormat::R8Unorm:
    case TextureFormat::R8Snorm:
    case TextureFormat::R8Uint:
    case TextureFormat::R8Sint:
        return width;
    case TextureFormat::R16Float:
    case TextureFormat::R16Unorm:
    case TextureFormat::R16Snorm:
    case TextureFormat::R16Uint:
    case TextureFormat::R16Sint:
        return width * 2;
    case TextureFormat::R32Float:
    case TextureFormat::R32Uint:
    case TextureFormat::R32Sint:
        return width * 4;
    case TextureFormat::RG8Unorm:
    case TextureFormat::RG8Snorm:
    case TextureFormat::RG8Uint:
    case TextureFormat::RG8Sint:
        return width * 2;
    case TextureFormat::RG16Float:
    case TextureFormat::RG16Unorm:
    case TextureFormat::RG16Snorm:
    case TextureFormat::RG16Uint:
    case TextureFormat::RG16Sint:
        return width * 4;
    case TextureFormat::RG32Float:
    case TextureFormat::RG32Uint:
    case TextureFormat::RG32Sint:
        return width * 8;
    case TextureFormat::RGB32Float:
    case TextureFormat::RGB32Uint:
    case TextureFormat::RGB32Sint:
        return width * 12;
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
        return width * 4;
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
        return width * 8;
    case TextureFormat::RGBA32Float:
    case TextureFormat::RGBA32Uint:
    case TextureFormat::RGBA32Sint:
    case TextureFormat::RGBX32Float:
    case TextureFormat::RGBX32Uint:
    case TextureFormat::RGBX32Sint:
        return width * 16;
    case TextureFormat::S8Uint:
        return width;
    case TextureFormat::Z16Unorm:
        return width * 2;
    case TextureFormat::Z24Unorm_X8Uint:
    case TextureFormat::Z24Unorm_S8Uint:
        return width * 4;
    case TextureFormat::Z32Float:
        return width * 4;
    case TextureFormat::Z32Float_X24S8Uint:
        return width * 8;
        return width * 4;
    case TextureFormat::RGBA4Unorm:
        return width * 2;
    case TextureFormat::RGB5Unorm:
    case TextureFormat::RGB5A1Unorm:
    case TextureFormat::R5G6B5Unorm:
        return width * 2;
    case TextureFormat::RGB10A2Unorm:
    case TextureFormat::RGB10A2Uint:
        return width * 4;
    case TextureFormat::RG11B10Float:
        return width * 4;
    case TextureFormat::E5BGR9Float:
        return width * 4;
    case TextureFormat::BC1_RGB:
    case TextureFormat::BC1_RGBA:
    case TextureFormat::BC1_RGB_sRGB:
    case TextureFormat::BC1_RGBA_sRGB:
        return ((width + 3) / 4) * 8;
    case TextureFormat::BC2_RGBA:
    case TextureFormat::BC3_RGBA:
    case TextureFormat::BC2_RGBA_sRGB:
    case TextureFormat::BC3_RGBA_sRGB:
        return ((width + 3) / 4) * 16;
    case TextureFormat::BC4_RUnorm:
    case TextureFormat::BC4_RSnorm:
        return ((width + 3) / 4) * 8;
    case TextureFormat::BC5_RGUnorm:
    case TextureFormat::BC5_RGSnorm:
        return ((width + 3) / 4) * 16;
    case TextureFormat::BC7_RGBAUnorm:
    case TextureFormat::BC7_RGBAUnorm_sRGB:
        return ((width + 3) / 4) * 16;
    case TextureFormat::BC6H_RGBA_SF16_Float:
    case TextureFormat::BC6H_RGBA_UF16_Float:
        return ((width + 3) / 4) * 16;
    case TextureFormat::ASTC_RGBA_4x4:
    case TextureFormat::ASTC_RGBA_4x4_sRGB:
        return ((width + 3) / 4) * 16;
    case TextureFormat::ASTC_RGBA_5x4:
    case TextureFormat::ASTC_RGBA_5x4_sRGB:
        return ((width + 4) / 5) * 16;
    case TextureFormat::ASTC_RGBA_5x5:
    case TextureFormat::ASTC_RGBA_5x5_sRGB:
        return ((width + 4) / 5) * 16;
    case TextureFormat::ASTC_RGBA_6x5:
    case TextureFormat::ASTC_RGBA_6x5_sRGB:
        return ((width + 5) / 6) * 16;
    case TextureFormat::ASTC_RGBA_6x6:
    case TextureFormat::ASTC_RGBA_6x6_sRGB:
        return ((width + 5) / 6) * 16;
    case TextureFormat::ASTC_RGBA_8x5:
    case TextureFormat::ASTC_RGBA_8x5_sRGB:
        return ((width + 7) / 8) * 16;
    case TextureFormat::ASTC_RGBA_8x6:
    case TextureFormat::ASTC_RGBA_8x6_sRGB:
        return ((width + 7) / 8) * 16;
    case TextureFormat::ASTC_RGBA_8x8:
    case TextureFormat::ASTC_RGBA_8x8_sRGB:
        return ((width + 7) / 8) * 16;
    case TextureFormat::ASTC_RGBA_10x5:
    case TextureFormat::ASTC_RGBA_10x5_sRGB:
        return ((width + 9) / 10) * 16;
    case TextureFormat::ASTC_RGBA_10x6:
    case TextureFormat::ASTC_RGBA_10x6_sRGB:
        return ((width + 9) / 10) * 16;
    case TextureFormat::ASTC_RGBA_10x8:
    case TextureFormat::ASTC_RGBA_10x8_sRGB:
        return ((width + 9) / 10) * 16;
    case TextureFormat::ASTC_RGBA_10x10:
    case TextureFormat::ASTC_RGBA_10x10_sRGB:
        return ((width + 9) / 10) * 16;
    case TextureFormat::ASTC_RGBA_12x10:
    case TextureFormat::ASTC_RGBA_12x10_sRGB:
        return ((width + 11) / 12) * 16;
    case TextureFormat::ASTC_RGBA_12x12:
    case TextureFormat::ASTC_RGBA_12x12_sRGB:
        return ((width + 11) / 12) * 16;
    case TextureFormat::B5G6R5Unorm:
    case TextureFormat::BGR5Unorm:
    case TextureFormat::BGR5A1Unorm:
    case TextureFormat::A1BGR5Unorm:
        return width * 2;
    case TextureFormat::BGRX8Unorm:
    case TextureFormat::BGRA8Unorm:
    case TextureFormat::BGRX8Unorm_sRGB:
    case TextureFormat::BGRA8Unorm_sRGB:
        return width * 4;
    case TextureFormat::ETC2_R_Unorm:
    case TextureFormat::ETC2_R_Snorm:
        return ((width + 3) / 4) * 8;
    case TextureFormat::ETC2_RG_Unorm:
    case TextureFormat::ETC2_RG_Snorm:
        return ((width + 3) / 4) * 16;
    case TextureFormat::ETC2_RGB:
    case TextureFormat::PTA_ETC2_RGB:
    case TextureFormat::ETC2_RGB_sRGB:
    case TextureFormat::PTA_ETC2_RGB_sRGB:
        return ((width + 3) / 4) * 8;
    case TextureFormat::ETC2_RGBA:
    case TextureFormat::ETC2_RGBA_sRGB:
        return ((width + 3) / 4) * 16;
    }
}

bool is_texture_format_compressed(const TextureFormat format) {
    switch (format) {
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
        return true;
    default:
        return false;
    }
}

bool is_texture_format_depth_or_stencil(const TextureFormat format) {
    switch (format) {
    case TextureFormat::S8Uint:
    case TextureFormat::Z16Unorm:
    case TextureFormat::Z24Unorm_X8Uint:
    case TextureFormat::Z32Float:
    case TextureFormat::Z24Unorm_S8Uint:
    case TextureFormat::Z32Float_X24S8Uint:
        return true;
    default:
        return false;
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
        SWIZZLE(x, y, z, w);
    case TextureFormat::Z32Float:
        SWIZZLE(x, ImageSwizzle::Zero, ImageSwizzle::Zero,
                ImageSwizzle::OneFloat);
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

bool texture_format_can_be_swizzled(TextureFormat format) {
    switch (format) {
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
    case TextureFormat::B5G6R5Unorm:
    case TextureFormat::BGR5Unorm:
    case TextureFormat::BGR5A1Unorm:
    case TextureFormat::A1BGR5Unorm:
    case TextureFormat::BGRX8Unorm:
    case TextureFormat::BGRA8Unorm:
    case TextureFormat::BGRX8Unorm_sRGB:
    case TextureFormat::BGRA8Unorm_sRGB:
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
        return true;
    default:
        return false;
    }
}

SwizzleChannels
get_texture_format_default_swizzle_channels(const TextureFormat format) {
    if (!texture_format_can_be_swizzled(format))
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
