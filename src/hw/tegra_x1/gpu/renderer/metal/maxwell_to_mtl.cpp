#include "hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer::Metal {

#define PIXEL_FORMAT_ENTRY(format, pixel_format, has_stencil)                  \
    {                                                                          \
        TextureFormat::format, { MTL::PixelFormat##pixel_format, has_stencil } \
    }

#define COLOR_PIXEL_FORMAT_ENTRY(format, pixel_format)                         \
    PIXEL_FORMAT_ENTRY(format, pixel_format, false)

std::map<TextureFormat, PixelFormatInfo> pixel_format_lut = {
    COLOR_PIXEL_FORMAT_ENTRY(R8Unorm, R8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(R8Snorm, R8Snorm),
    COLOR_PIXEL_FORMAT_ENTRY(R8Uint, R8Uint),
    COLOR_PIXEL_FORMAT_ENTRY(R8Sint, R8Sint),
    COLOR_PIXEL_FORMAT_ENTRY(R16Float, R16Float),
    COLOR_PIXEL_FORMAT_ENTRY(R16Unorm, R16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(R16Snorm, R16Snorm),
    COLOR_PIXEL_FORMAT_ENTRY(R16Uint, R16Uint),
    COLOR_PIXEL_FORMAT_ENTRY(R16Sint, R16Sint),
    COLOR_PIXEL_FORMAT_ENTRY(R32Float, R32Float),
    COLOR_PIXEL_FORMAT_ENTRY(R32Uint, R32Uint),
    COLOR_PIXEL_FORMAT_ENTRY(R32Sint, R32Sint),
    COLOR_PIXEL_FORMAT_ENTRY(RG8Unorm, RG8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(RG8Snorm, RG8Snorm),
    COLOR_PIXEL_FORMAT_ENTRY(RG8Uint, RG8Uint),
    COLOR_PIXEL_FORMAT_ENTRY(RG8Sint, RG8Sint),
    COLOR_PIXEL_FORMAT_ENTRY(RG16Float, RG16Float),
    COLOR_PIXEL_FORMAT_ENTRY(RG16Unorm, RG16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(RG16Snorm, RG16Snorm),
    COLOR_PIXEL_FORMAT_ENTRY(RG16Uint, RG16Uint),
    COLOR_PIXEL_FORMAT_ENTRY(RG16Sint, RG16Sint),
    COLOR_PIXEL_FORMAT_ENTRY(RG32Float, RG32Float),
    COLOR_PIXEL_FORMAT_ENTRY(RG32Uint, RG32Uint),
    COLOR_PIXEL_FORMAT_ENTRY(RG32Sint, RG32Sint),
    COLOR_PIXEL_FORMAT_ENTRY(RGB32Float, RGBA32Float), // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGB32Uint, RGBA32Uint),   // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGB32Sint, RGBA32Sint),   // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGBA8Unorm, RGBA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA8Snorm, RGBA8Snorm),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA8Uint, RGBA8Uint),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA8Sint, RGBA8Sint),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA16Float, RGBA16Float),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA16Unorm, RGBA16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA16Snorm, RGBA16Snorm),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA16Uint, RGBA16Uint),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA16Sint, RGBA16Sint),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA32Float, RGBA32Float),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA32Uint, RGBA32Uint),
    PIXEL_FORMAT_ENTRY(S8Uint, Stencil8, true),
    PIXEL_FORMAT_ENTRY(Z16Unorm, Depth16Unorm, false),
    PIXEL_FORMAT_ENTRY(Z24Unorm_X8Uint, Depth32Float_Stencil8, true), // HACK
    PIXEL_FORMAT_ENTRY(Z32Float, Depth32Float, false),
    PIXEL_FORMAT_ENTRY(Z24Unorm_S8Uint, Depth32Float_Stencil8, true), // HACK
    PIXEL_FORMAT_ENTRY(Z32Float_X24S8Uint, Invalid, true),            // TODO
    COLOR_PIXEL_FORMAT_ENTRY(RGBX8Unorm_sRGB, RGBA8Unorm_sRGB),       // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGBA8Unorm_sRGB, RGBA8Unorm_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA4Unorm, ABGR4Unorm),     // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGB5Unorm, BGR5A1Unorm),     // HACK
    COLOR_PIXEL_FORMAT_ENTRY(R5G6B5Unorm, B5G6R5Unorm),   // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGB10A2Unorm, RGB10A2Unorm), // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGB10A2Uint, RGB10A2Uint),   // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RG11B10Float, RG11B10Float), // HACK
    COLOR_PIXEL_FORMAT_ENTRY(E5BGR9Float, RGB9E5Float),   // HACK
    COLOR_PIXEL_FORMAT_ENTRY(BC1_RGB, BC1_RGBA),          // HACK
    COLOR_PIXEL_FORMAT_ENTRY(BC1_RGBA, BC1_RGBA),
    COLOR_PIXEL_FORMAT_ENTRY(BC2_RGBA, BC2_RGBA),
    COLOR_PIXEL_FORMAT_ENTRY(BC3_RGBA, BC3_RGBA),
    COLOR_PIXEL_FORMAT_ENTRY(BC1_RGB_sRGB, BC1_RGBA_sRGB), // HACK
    COLOR_PIXEL_FORMAT_ENTRY(BC1_RGBA_sRGB, BC1_RGBA_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(BC2_RGBA_sRGB, BC2_RGBA_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(BC3_RGBA_sRGB, BC3_RGBA_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(BC4_RUnorm, BC4_RUnorm),
    COLOR_PIXEL_FORMAT_ENTRY(BC4_RSnorm, BC4_RSnorm),
    COLOR_PIXEL_FORMAT_ENTRY(BC5_RGUnorm, BC5_RGUnorm),
    COLOR_PIXEL_FORMAT_ENTRY(BC5_RGSnorm, BC5_RGSnorm),
    COLOR_PIXEL_FORMAT_ENTRY(BC7_RGBAUnorm, BC7_RGBAUnorm),
    COLOR_PIXEL_FORMAT_ENTRY(BC7_RGBAUnorm_sRGB, BC7_RGBAUnorm_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(BC6H_RGBA_SF16_Float, Invalid), // TODO
    COLOR_PIXEL_FORMAT_ENTRY(BC6H_RGBA_UF16_Float, Invalid), // TODO
    COLOR_PIXEL_FORMAT_ENTRY(RGBX8Unorm, RGBA8Unorm),        // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGBX8Snorm, RGBA8Snorm),        // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGBX8Uint, RGBA8Uint),          // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGBX8Sint, RGBA8Sint),          // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGBX16Float, RGBA16Float),      // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGBX16Unorm, RGBA16Unorm),      // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGBX16Snorm, RGBA16Snorm),      // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGBX16Uint, RGBA16Uint),        // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGBX16Sint, RGBA16Sint),        // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGBX32Float, RGBA32Float),      // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGBX32Uint, RGBA32Uint),        // HACK
    COLOR_PIXEL_FORMAT_ENTRY(RGBX32Sint, RGBA32Sint),        // HACK
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_4x4, ASTC_4x4_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_5x4, ASTC_5x4_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_5x5, ASTC_5x5_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_6x5, ASTC_6x5_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_6x6, ASTC_6x6_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_8x5, ASTC_8x5_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_8x6, ASTC_8x6_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_8x8, ASTC_8x8_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_10x5, ASTC_10x5_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_10x6, ASTC_10x6_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_10x8, ASTC_10x8_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_10x10, ASTC_10x10_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_12x10, ASTC_12x10_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_12x12, ASTC_12x12_LDR),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_4x4_sRGB, ASTC_4x4_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_5x4_sRGB, ASTC_5x4_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_5x5_sRGB, ASTC_5x5_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_6x5_sRGB, ASTC_6x5_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_6x6_sRGB, ASTC_6x6_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_8x5_sRGB, ASTC_8x5_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_8x6_sRGB, ASTC_8x6_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_8x8_sRGB, ASTC_8x8_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_10x5_sRGB, ASTC_10x5_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_10x6_sRGB, ASTC_10x6_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_10x8_sRGB, ASTC_10x8_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_10x10_sRGB, ASTC_10x10_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_12x10_sRGB, ASTC_12x10_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(ASTC_RGBA_12x12_sRGB, ASTC_12x12_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(B5G6R5Unorm, B5G6R5Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(BGR5Unorm, BGR5A1Unorm), // HACK
    COLOR_PIXEL_FORMAT_ENTRY(BGR5A1Unorm, BGR5A1Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(A1BGR5Unorm, A1BGR5Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(BGRX8Unorm, BGRA8Unorm), // HACK
    COLOR_PIXEL_FORMAT_ENTRY(BGRA8Unorm, BGRA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(BGRX8Unorm_sRGB, BGRA8Unorm_sRGB), // HACK
    COLOR_PIXEL_FORMAT_ENTRY(BGRA8Unorm_sRGB, BGRA8Unorm_sRGB),
    // TODO: implement ETC formats
    COLOR_PIXEL_FORMAT_ENTRY(ETC2_R_Unorm, Invalid),      // TODO
    COLOR_PIXEL_FORMAT_ENTRY(ETC2_R_Snorm, Invalid),      // TODO
    COLOR_PIXEL_FORMAT_ENTRY(ETC2_RG_Unorm, Invalid),     // TODO
    COLOR_PIXEL_FORMAT_ENTRY(ETC2_RG_Snorm, Invalid),     // TODO
    COLOR_PIXEL_FORMAT_ENTRY(ETC2_RGB, Invalid),          // TODO
    COLOR_PIXEL_FORMAT_ENTRY(PTA_ETC2_RGB, Invalid),      // TODO
    COLOR_PIXEL_FORMAT_ENTRY(ETC2_RGBA, Invalid),         // TODO
    COLOR_PIXEL_FORMAT_ENTRY(ETC2_RGB_sRGB, Invalid),     // TODO
    COLOR_PIXEL_FORMAT_ENTRY(PTA_ETC2_RGB_sRGB, Invalid), // TODO
    COLOR_PIXEL_FORMAT_ENTRY(ETC2_RGBA_sRGB, Invalid),    // TODO
};

/*
std::map<TextureFormat, PixelFormatInfo> pixel_format_lut = {
    // Color
    COLOR_PIXEL_FORMAT_ENTRY(Bitmap, Invalid),
    // COLOR_PIXEL_FORMAT_ENTRY(Unknown1D, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA32Float, RGBA32Float),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA32Sint, RGBA32Sint),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA32Uint, RGBA32Uint),
    COLOR_PIXEL_FORMAT_ENTRY(RGBX32Float, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(RGBX32Sint, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(RGBX32Uint, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA16Unorm, RGBA16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA16Snorm, RGBA16Snorm),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA16Sint, RGBA16Sint),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA16Uint, RGBA16Uint),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA16Float, RGBA16Float),
    COLOR_PIXEL_FORMAT_ENTRY(RG32Float, RG32Float),
    COLOR_PIXEL_FORMAT_ENTRY(RG32Sint, RG32Sint),
    COLOR_PIXEL_FORMAT_ENTRY(RG32Uint, RG32Uint),
    COLOR_PIXEL_FORMAT_ENTRY(RGBX16Float, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(RGBX16Float, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(BGRA8Unorm, BGRA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(BGRA8Srgb, BGRA8Unorm_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(RGB10A2Unorm, RGB10A2Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(RGB10A2Uint, RGB10A2Uint),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA8Unorm, RGBA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA8Srgb, RGBA8Unorm_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA8Snorm, RGBA8Snorm),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA8Sint, RGBA8Sint),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA8Uint, RGBA8Uint),
    COLOR_PIXEL_FORMAT_ENTRY(RG16Unorm, RG16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(RG16Snorm, RG16Snorm),
    COLOR_PIXEL_FORMAT_ENTRY(RG16Sint, RG16Sint),
    COLOR_PIXEL_FORMAT_ENTRY(RG16Uint, RG16Uint),
    COLOR_PIXEL_FORMAT_ENTRY(RG16Float, RG16Float),
    COLOR_PIXEL_FORMAT_ENTRY(BGR10A2Unorm, BGR10A2Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(R11G11B10Float, RG11B10Float),
    COLOR_PIXEL_FORMAT_ENTRY(R32Sint, R32Sint),
    COLOR_PIXEL_FORMAT_ENTRY(R32Uint, R32Uint),
    COLOR_PIXEL_FORMAT_ENTRY(R32Float, R32Float),
    COLOR_PIXEL_FORMAT_ENTRY(BGRX8Unorm, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(BGRX8Srgb, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(B5G6R5Unorm, B5G6R5Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(BGR5A1Unorm, BGR5A1Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(RG8Unorm, RG8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(RG8Snorm, RG8Snorm),
    COLOR_PIXEL_FORMAT_ENTRY(RG8Sint, RG8Sint),
    COLOR_PIXEL_FORMAT_ENTRY(RG8Uint, RG8Uint),
    COLOR_PIXEL_FORMAT_ENTRY(R16Unorm, R16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(R16Snorm, R16Snorm),
    COLOR_PIXEL_FORMAT_ENTRY(R16Sint, R16Sint),
    COLOR_PIXEL_FORMAT_ENTRY(R16Uint, R16Uint),
    COLOR_PIXEL_FORMAT_ENTRY(R16Float, R16Float),
    COLOR_PIXEL_FORMAT_ENTRY(R8Unorm, R8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(R8Snorm, R8Snorm),
    COLOR_PIXEL_FORMAT_ENTRY(R8Sint, R8Sint),
    COLOR_PIXEL_FORMAT_ENTRY(R8Uint, R8Uint),
    COLOR_PIXEL_FORMAT_ENTRY(A8Unorm, A8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(BGR5X1Unorm, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(RGBX8Unorm, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(RGBX8Srgb, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(BGR5X1UnormUnknownFB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(BGR5X1UnormUnknownFC, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(BGRX8UnormUnknownFD, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(BGRX8UnormUnknownFE, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(Y32UintUnknownFF, Invalid),

    // Depth stencil
    PIXEL_FORMAT_ENTRY(Z32Float, Depth32Float, false),
    PIXEL_FORMAT_ENTRY(Z16Unorm, Depth16Unorm, false),
    PIXEL_FORMAT_ENTRY(S8Z24Unorm, Depth24Unorm_Stencil8,
                       true), // TODO: swizzle?
    PIXEL_FORMAT_ENTRY(Z24X8Unorm, Invalid, false),
    PIXEL_FORMAT_ENTRY(Z24S8Unorm, Depth24Unorm_Stencil8, true),
    PIXEL_FORMAT_ENTRY(S8Uint, Stencil8, true),
    PIXEL_FORMAT_ENTRY(Z24C8Unorm, Invalid, false),
    PIXEL_FORMAT_ENTRY(Z32S8X24Float, Invalid, false),
    PIXEL_FORMAT_ENTRY(Z24X8S8C8X16Unorm, Invalid, false),
    PIXEL_FORMAT_ENTRY(Z32X8C8X16Float, Invalid, false),
    PIXEL_FORMAT_ENTRY(Z32S8C8X16Float, Invalid, false),
};
*/

/*
std::map<NvColorFormat, PixelFormatInfo> pixel_format_lut = {
    PIXEL_FORMAT_ENTRY(Unspecified, Invalid, false),
    PIXEL_FORMAT_ENTRY(NonColor8, Invalid, false),
    PIXEL_FORMAT_ENTRY(NonColor16, Invalid, false),
    PIXEL_FORMAT_ENTRY(NonColor24, Invalid, false),
    PIXEL_FORMAT_ENTRY(NonColor32, Invalid, false),
    COLOR_PIXEL_FORMAT_ENTRY(X4C4, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(A4L4, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(A8L8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(Float_A16L16, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(A1B5G5R5, A1BGR5Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(A4B4G4R4, ABGR4Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(A5B5G5R1, RGBA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(A2B10G10R10, RGB10A2Unorm), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(A8B8G8R8, RGBA8Unorm),      // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY(A16B16G16R16, RGBA16Unorm), // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY(Float_A16B16G16R16, RGBA16Float),
    COLOR_PIXEL_FORMAT_ENTRY(A1R5G5B5, A1BGR5Unorm),     // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY(A4R4G4B4, ABGR4Unorm),      // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY(A5R1G5B5, A1BGR5Unorm),     // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(A2R10G10B10, RGB10A2Unorm), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(A8R8G8B8, RGBA8Unorm),      // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY(A1, A8Unorm),               // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(A2, A8Unorm),               // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(A4, A8Unorm),               // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(A8, A8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(A16, RGBA16Unorm), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(A32, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(Float_A16, RGBA16Float), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(L4A4, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(L8A8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(B4G4R4A4, ABGR4Unorm),  // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY(B5G5R1A5, A1BGR5Unorm), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(B5G5R5A1, BGR5A1Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(B8G8R8A8, BGRA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(B10G10R10A2, BGR10A2Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(R1G5B5A5, A1BGR5Unorm), // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY(R4G4B4A4, ABGR4Unorm),  // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY(R5G5B5A1, BGR5A1Unorm), // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY(R8G8B8A8, RGBA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(R10G10B10A2, RGB10A2Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(L1, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(L2, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(L4, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(L8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(L16, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(L32, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(Float_L16, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(B5G6R5, B5G6R5Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(B6G5R5, RGBA8Unorm), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(B5G5R5X1, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(B8G8R8, BGRA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(B8G8R8X8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(Float_B10G11R11,
                             RG11B10Float), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(X1B5G5R5, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(X8B8G8R8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(X16B16G16R16, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(Float_X16B16G16R16, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(R3G3B2, B5G6R5Unorm), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(R5G5B6, B5G6R5Unorm), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(R5G6B5, B5G6R5Unorm), // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY(R5G5B5X1, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(R8G8B8, RGBA8Unorm), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY(R8G8B8X8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(X1R5G5B5, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(X8R8G8B8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(R8G8, RG8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(R16G16, RG16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(Float_R16G16, RG16Float),
    COLOR_PIXEL_FORMAT_ENTRY(R8, R8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(R16, R16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY(Float_R16, R16Float),
    COLOR_PIXEL_FORMAT_ENTRY(A2B10G10R10_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(A8B8G8R8_sRGB, RGBA8Unorm_sRGB), // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY(A16B16G16R16_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(A2R10G10B10_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(B10G10R10A2_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(R10G10B10A2_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(X8B8G8R8_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(X16B16G16R16_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(A2B10G10R10_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(A8B8G8R8_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(A16B16G16R16_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(A2R10G10B10_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(B10G10R10A2_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(R10G10B10A2_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(X8B8G8R8_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY(X16B16G16R16_709, Invalid),
    // TODO: finish
};
*/

const PixelFormatInfo& to_mtl_pixel_format_info(TextureFormat format) {
    auto it = pixel_format_lut.find(format);
    ASSERT_DEBUG(it != pixel_format_lut.end(), MetalRenderer,
                 "Unknown format {}", format);

    const auto& info = it->second;
    ASSERT_DEBUG(info.pixel_format != MTL::PixelFormatInvalid, MetalRenderer,
                 "Unimplemented format {}", format);

    return info;
}

MTL::PrimitiveType
to_mtl_primitive_type(const Engines::PrimitiveType primitive_type) {
    switch (primitive_type) {
    case Engines::PrimitiveType::Points:
        return MTL::PrimitiveTypePoint;
    case Engines::PrimitiveType::Lines:
        return MTL::PrimitiveTypeLine;
    case Engines::PrimitiveType::LineStrip:
        return MTL::PrimitiveTypeLineStrip;
    case Engines::PrimitiveType::Triangles:
        return MTL::PrimitiveTypeTriangle;
    case Engines::PrimitiveType::TriangleStrip:
        return MTL::PrimitiveTypeTriangleStrip;
    default:
        LOG_NOT_IMPLEMENTED(MetalRenderer, "Primitive type {}", primitive_type);
        return MTL::PrimitiveTypeTriangle;
    }
}

MTL::IndexType to_mtl_index_type(Engines::IndexType index_type) {
    switch (index_type) {
    case Engines::IndexType::UInt16:
        return MTL::IndexTypeUInt16;
    case Engines::IndexType::UInt32:
        return MTL::IndexTypeUInt32;
    default:
        LOG_NOT_IMPLEMENTED(MetalRenderer, "Index type {}", index_type);
        return MTL::IndexTypeUInt16;
    }
}

const MTL::VertexFormat to_mtl_vertex_format(Engines::VertexAttribType type,
                                             Engines::VertexAttribSize size,
                                             bool bgra) {
    ASSERT_DEBUG(!bgra || (type == Engines::VertexAttribType::Unorm &&
                           size == Engines::VertexAttribSize::_4x8),
                 MetalRenderer,
                 "BGRA vertex formats are only supported for uchar4 normalized "
                 "types (type: {}, size: {})",
                 type, size);

    switch (type) {
    case Engines::VertexAttribType::Snorm:
        switch (size) {
        case Engines::VertexAttribSize::_1x32:
        case Engines::VertexAttribSize::_2x32:
        case Engines::VertexAttribSize::_3x32:
        case Engines::VertexAttribSize::_4x32:
            LOG_NOT_IMPLEMENTED(
                MetalRenderer,
                "Signed normalized 32-bit integer vertex formats");
            return MTL::VertexFormatInvalid;

        case Engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatShortNormalized;
        case Engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatShort2Normalized;
        case Engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatShort3Normalized;
        case Engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatShort4Normalized;

        case Engines::VertexAttribSize::_1x8:
            return MTL::VertexFormatCharNormalized;
        case Engines::VertexAttribSize::_2x8:
            return MTL::VertexFormatChar2Normalized;
        case Engines::VertexAttribSize::_3x8:
            return MTL::VertexFormatChar3Normalized;
        case Engines::VertexAttribSize::_4x8:
            return MTL::VertexFormatChar4Normalized;

        case Engines::VertexAttribSize::_10_10_10_2:
            return MTL::VertexFormatInt1010102Normalized;

        default:
            LOG_ERROR(MetalRenderer,
                      "Invalid vertex attribute size {} for snorm type", size);
            return MTL::VertexFormatInvalid;
        }
    case Engines::VertexAttribType::Unorm:
        switch (size) {
        case Engines::VertexAttribSize::_1x32:
        case Engines::VertexAttribSize::_2x32:
        case Engines::VertexAttribSize::_3x32:
        case Engines::VertexAttribSize::_4x32:
            LOG_NOT_IMPLEMENTED(
                MetalRenderer,
                "Unsigned normalized 32-bit integer vertex formats");
            return MTL::VertexFormatInvalid;

        case Engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatUShortNormalized;
        case Engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatUShort2Normalized;
        case Engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatUShort3Normalized;
        case Engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatUShort4Normalized;

        case Engines::VertexAttribSize::_1x8:
            return MTL::VertexFormatUCharNormalized;
        case Engines::VertexAttribSize::_2x8:
            return MTL::VertexFormatUChar2Normalized;
        case Engines::VertexAttribSize::_3x8:
            return MTL::VertexFormatUChar3Normalized;
        case Engines::VertexAttribSize::_4x8:
            if (bgra)
                return MTL::VertexFormatUChar4Normalized_BGRA;
            else
                return MTL::VertexFormatUChar4Normalized;

        case Engines::VertexAttribSize::_10_10_10_2:
            return MTL::VertexFormatUInt1010102Normalized;

        default:
            LOG_ERROR(MetalRenderer,
                      "Invalid vertex attribute size {} for unorm type", size);
            return MTL::VertexFormatInvalid;
        }
    case Engines::VertexAttribType::Sint:
        switch (size) {
        case Engines::VertexAttribSize::_1x32:
            return MTL::VertexFormatInt;
        case Engines::VertexAttribSize::_2x32:
            return MTL::VertexFormatInt2;
        case Engines::VertexAttribSize::_3x32:
            return MTL::VertexFormatInt3;
        case Engines::VertexAttribSize::_4x32:
            return MTL::VertexFormatInt4;

        case Engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatShort;
        case Engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatShort2;
        case Engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatShort3;
        case Engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatShort4;

        case Engines::VertexAttribSize::_1x8:
            return MTL::VertexFormatChar;
        case Engines::VertexAttribSize::_2x8:
            return MTL::VertexFormatChar2;
        case Engines::VertexAttribSize::_3x8:
            return MTL::VertexFormatChar3;
        case Engines::VertexAttribSize::_4x8:
            return MTL::VertexFormatChar4;

        default:
            LOG_ERROR(MetalRenderer,
                      "Invalid vertex attribute size {} for sint type", size);
            return MTL::VertexFormatInvalid;
        }
    case Engines::VertexAttribType::Uint:
        switch (size) {
        case Engines::VertexAttribSize::_1x32:
            return MTL::VertexFormatUInt;
        case Engines::VertexAttribSize::_2x32:
            return MTL::VertexFormatUInt2;
        case Engines::VertexAttribSize::_3x32:
            return MTL::VertexFormatUInt3;
        case Engines::VertexAttribSize::_4x32:
            return MTL::VertexFormatUInt4;

        case Engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatUShort;
        case Engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatUShort2;
        case Engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatUShort3;
        case Engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatUShort4;

        case Engines::VertexAttribSize::_1x8:
            return MTL::VertexFormatUChar;
        case Engines::VertexAttribSize::_2x8:
            return MTL::VertexFormatUChar2;
        case Engines::VertexAttribSize::_3x8:
            return MTL::VertexFormatUChar3;
        case Engines::VertexAttribSize::_4x8:
            return MTL::VertexFormatUChar4;

        default:
            LOG_ERROR(MetalRenderer,
                      "Invalid vertex attribute size {} for uint type", size);
            return MTL::VertexFormatInvalid;
        }
    case Engines::VertexAttribType::Sscaled:
        switch (size) {
        case Engines::VertexAttribSize::_1x32:
            return MTL::VertexFormatInt;
        case Engines::VertexAttribSize::_2x32:
            return MTL::VertexFormatInt2;
        case Engines::VertexAttribSize::_3x32:
            return MTL::VertexFormatInt3;
        case Engines::VertexAttribSize::_4x32:
            return MTL::VertexFormatInt4;

        case Engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatShort;
        case Engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatShort2;
        case Engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatShort3;
        case Engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatShort4;

        case Engines::VertexAttribSize::_1x8:
            return MTL::VertexFormatChar;
        case Engines::VertexAttribSize::_2x8:
            return MTL::VertexFormatChar2;
        case Engines::VertexAttribSize::_3x8:
            return MTL::VertexFormatChar3;
        case Engines::VertexAttribSize::_4x8:
            return MTL::VertexFormatChar4;

        default:
            LOG_ERROR(MetalRenderer,
                      "Invalid vertex attribute size {} for sint type", size);
            return MTL::VertexFormatInvalid;
        }
    case Engines::VertexAttribType::Uscaled:
        switch (size) {
        case Engines::VertexAttribSize::_1x32:
            return MTL::VertexFormatUInt;
        case Engines::VertexAttribSize::_2x32:
            return MTL::VertexFormatUInt2;
        case Engines::VertexAttribSize::_3x32:
            return MTL::VertexFormatUInt3;
        case Engines::VertexAttribSize::_4x32:
            return MTL::VertexFormatUInt4;

        case Engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatUShort;
        case Engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatUShort2;
        case Engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatUShort3;
        case Engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatUShort4;

        case Engines::VertexAttribSize::_1x8:
            return MTL::VertexFormatUChar;
        case Engines::VertexAttribSize::_2x8:
            return MTL::VertexFormatUChar2;
        case Engines::VertexAttribSize::_3x8:
            return MTL::VertexFormatUChar3;
        case Engines::VertexAttribSize::_4x8:
            return MTL::VertexFormatUChar4;

        default:
            LOG_ERROR(MetalRenderer,
                      "Invalid vertex attribute size {} for uint type", size);
            return MTL::VertexFormatInvalid;
        }
    case Engines::VertexAttribType::Float:
        switch (size) {
        case Engines::VertexAttribSize::_1x32:
            return MTL::VertexFormatFloat;
        case Engines::VertexAttribSize::_2x32:
            return MTL::VertexFormatFloat2;
        case Engines::VertexAttribSize::_3x32:
            return MTL::VertexFormatFloat3;
        case Engines::VertexAttribSize::_4x32:
            return MTL::VertexFormatFloat4;

        case Engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatHalf;
        case Engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatHalf2;
        case Engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatHalf3;
        case Engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatHalf4;

        case Engines::VertexAttribSize::_1x8:
        case Engines::VertexAttribSize::_2x8:
        case Engines::VertexAttribSize::_3x8:
        case Engines::VertexAttribSize::_4x8:
            LOG_NOT_IMPLEMENTED(MetalRenderer,
                                "8-bit floating-point vertex formats");
            return MTL::VertexFormatInvalid;

        case Engines::VertexAttribSize::_11_11_10:
            return MTL::VertexFormatFloatRG11B10;

        default:
            LOG_ERROR(MetalRenderer,
                      "Invalid vertex attribute size {} for uint type", size);
            return MTL::VertexFormatInvalid;
        }
    default:
        LOG_ERROR(MetalRenderer, "Unimplemented vertex attribute type {}",
                  type);
        return MTL::VertexFormatInvalid;
    }
}

MTL::CompareFunction
to_mtl_compare_func(Engines::DepthTestFunc depth_test_func) {
    switch (depth_test_func) {
    case Engines::DepthTestFunc::Never:
        return MTL::CompareFunctionNever;
    case Engines::DepthTestFunc::Less:
        return MTL::CompareFunctionLess;
    case Engines::DepthTestFunc::Equal:
        return MTL::CompareFunctionEqual;
    case Engines::DepthTestFunc::LessEqual:
        return MTL::CompareFunctionLessEqual;
    case Engines::DepthTestFunc::Greater:
        return MTL::CompareFunctionGreater;
    case Engines::DepthTestFunc::NotEqual:
        return MTL::CompareFunctionNotEqual;
    case Engines::DepthTestFunc::GreaterEqual:
        return MTL::CompareFunctionGreaterEqual;
    case Engines::DepthTestFunc::Always:
        return MTL::CompareFunctionAlways;
    default:
        LOG_NOT_IMPLEMENTED(MetalRenderer, "Depth test func {}",
                            depth_test_func);
        return MTL::CompareFunctionAlways;
    }
}

} // namespace Hydra::HW::TegraX1::GPU::Renderer::Metal
