#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

MTL::TextureType ToMtlTextureType(TextureType type) {
    switch (type) {
    case TextureType::_1D:
        return MTL::TextureType1D;
    case TextureType::_1DArray:
        return MTL::TextureType1DArray;
    case TextureType::_1DBuffer:
        return MTL::TextureTypeTextureBuffer;
    case TextureType::_2D:
        return MTL::TextureType2D;
    case TextureType::_2DArray:
        return MTL::TextureType2DArray;
    case TextureType::_3D:
        return MTL::TextureType3D;
    case TextureType::Cube:
        return MTL::TextureTypeCube;
    case TextureType::CubeArray:
        return MTL::TextureTypeCubeArray;
    }
}

#define PIXEL_FORMAT_ENTRY(format, pixel_format, has_depth, has_stencil,       \
                           component_indices)                                  \
    {                                                                          \
        TextureFormat::format, {                                               \
            MTL::PixelFormat##pixel_format, has_depth, has_stencil,            \
                PASS(component_indices)                                        \
        }                                                                      \
    }

#define DS_PIXEL_FORMAT_ENTRY(format, pixel_format, has_depth, has_stencil)    \
    PIXEL_FORMAT_ENTRY(format, pixel_format, has_depth, has_stencil,           \
                       PASS({0, 1, 2, 3}))

#define COLOR_PIXEL_FORMAT_ENTRY(format, pixel_format, component_indices)      \
    PIXEL_FORMAT_ENTRY(format, pixel_format, false, false,                     \
                       PASS(component_indices))

#define COLOR_PIXEL_FORMAT_ENTRY_RGBA(format, pixel_format)                    \
    COLOR_PIXEL_FORMAT_ENTRY(format, pixel_format, PASS({0, 1, 2, 3}))

std::map<TextureFormat, PixelFormatInfo> pixel_format_lut = {
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R8Unorm, R8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R8Snorm, R8Snorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R8Uint, R8Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R8Sint, R8Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R16Float, R16Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R16Unorm, R16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R16Snorm, R16Snorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R16Uint, R16Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R16Sint, R16Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R32Float, R32Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R32Uint, R32Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R32Sint, R32Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG8Unorm, RG8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG8Snorm, RG8Snorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG8Uint, RG8Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG8Sint, RG8Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG16Float, RG16Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG16Unorm, RG16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG16Snorm, RG16Snorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG16Uint, RG16Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG16Sint, RG16Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG32Float, RG32Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG32Uint, RG32Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG32Sint, RG32Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGB32Float, RGBA32Float), // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGB32Uint, RGBA32Uint),   // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGB32Sint, RGBA32Sint),   // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA8Unorm, RGBA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA8Snorm, RGBA8Snorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA8Uint, RGBA8Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA8Sint, RGBA8Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA16Float, RGBA16Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA16Unorm, RGBA16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA16Snorm, RGBA16Snorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA16Uint, RGBA16Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA16Sint, RGBA16Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA32Float, RGBA32Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA32Uint, RGBA32Uint),
    DS_PIXEL_FORMAT_ENTRY(S8Uint, Stencil8, false, true),
    DS_PIXEL_FORMAT_ENTRY(Z16Unorm, Depth16Unorm, true, false),
    DS_PIXEL_FORMAT_ENTRY(Z24Unorm_X8Uint, Depth32Float_Stencil8, true,
                          true), // HACK
    DS_PIXEL_FORMAT_ENTRY(Z32Float, Depth32Float, true, false),
    DS_PIXEL_FORMAT_ENTRY(Z24Unorm_S8Uint, Depth32Float_Stencil8, true,
                          true), // HACK
    DS_PIXEL_FORMAT_ENTRY(Z32Float_X24S8Uint, Depth32Float_Stencil8, true,
                          true),                                     // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX8Unorm_sRGB, RGBA8Unorm_sRGB), // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA8Unorm_sRGB, RGBA8Unorm_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY(RGBA4Unorm, ABGR4Unorm, PASS({3, 2, 1, 0})),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGB5Unorm, BGR5A1Unorm), // HACK
    COLOR_PIXEL_FORMAT_ENTRY(R5G6B5Unorm, B5G6R5Unorm,
                             PASS({2, 1, 0, 3})), // TODO: correct?
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGB10A2Unorm, RGB10A2Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGB10A2Uint, RGB10A2Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG11B10Float, RG11B10Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(E5BGR9Float, RGB9E5Float), // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC1_RGB, BC1_RGBA),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC1_RGBA, BC1_RGBA),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC2_RGBA, BC2_RGBA),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC3_RGBA, BC3_RGBA),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC1_RGB_sRGB, BC1_RGBA_sRGB), // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC1_RGBA_sRGB, BC1_RGBA_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC2_RGBA_sRGB, BC2_RGBA_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC3_RGBA_sRGB, BC3_RGBA_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC4_RUnorm, BC4_RUnorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC4_RSnorm, BC4_RSnorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC5_RGUnorm, BC5_RGUnorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC5_RGSnorm, BC5_RGSnorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC7_RGBAUnorm, BC7_RGBAUnorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC7_RGBAUnorm_sRGB, BC7_RGBAUnorm_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC6H_RGBA_SF16_Float, Invalid), // TODO
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BC6H_RGBA_UF16_Float, Invalid), // TODO
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX8Unorm, RGBA8Unorm),        // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX8Snorm, RGBA8Snorm),        // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX8Uint, RGBA8Uint),          // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX8Sint, RGBA8Sint),          // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX16Float, RGBA16Float),      // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX16Unorm, RGBA16Unorm),      // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX16Snorm, RGBA16Snorm),      // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX16Uint, RGBA16Uint),        // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX16Sint, RGBA16Sint),        // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX32Float, RGBA32Float),      // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX32Uint, RGBA32Uint),        // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX32Sint, RGBA32Sint),        // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_4x4, ASTC_4x4_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_5x4, ASTC_5x4_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_5x5, ASTC_5x5_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_6x5, ASTC_6x5_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_6x6, ASTC_6x6_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_8x5, ASTC_8x5_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_8x6, ASTC_8x6_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_8x8, ASTC_8x8_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_10x5, ASTC_10x5_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_10x6, ASTC_10x6_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_10x8, ASTC_10x8_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_10x10, ASTC_10x10_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_12x10, ASTC_12x10_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_12x12, ASTC_12x12_LDR),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_4x4_sRGB, ASTC_4x4_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_5x4_sRGB, ASTC_5x4_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_5x5_sRGB, ASTC_5x5_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_6x5_sRGB, ASTC_6x5_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_6x6_sRGB, ASTC_6x6_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_8x5_sRGB, ASTC_8x5_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_8x6_sRGB, ASTC_8x6_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_8x8_sRGB, ASTC_8x8_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_10x5_sRGB, ASTC_10x5_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_10x6_sRGB, ASTC_10x6_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_10x8_sRGB, ASTC_10x8_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_10x10_sRGB, ASTC_10x10_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_12x10_sRGB, ASTC_12x10_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ASTC_RGBA_12x12_sRGB, ASTC_12x12_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(B5G6R5Unorm, B5G6R5Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGR5Unorm, BGR5A1Unorm), // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGR5A1Unorm, BGR5A1Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A1BGR5Unorm, A1BGR5Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGRX8Unorm, BGRA8Unorm), // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGRA8Unorm, BGRA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGRX8Unorm_sRGB, BGRA8Unorm_sRGB), // HACK
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGRA8Unorm_sRGB, BGRA8Unorm_sRGB),
    // TODO: implement ETC formats
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ETC2_R_Unorm, Invalid),      // TODO
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ETC2_R_Snorm, Invalid),      // TODO
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ETC2_RG_Unorm, Invalid),     // TODO
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ETC2_RG_Snorm, Invalid),     // TODO
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ETC2_RGB, Invalid),          // TODO
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(PTA_ETC2_RGB, Invalid),      // TODO
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ETC2_RGBA, Invalid),         // TODO
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ETC2_RGB_sRGB, Invalid),     // TODO
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(PTA_ETC2_RGB_sRGB, Invalid), // TODO
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(ETC2_RGBA_sRGB, Invalid),    // TODO
};

/*
std::map<TextureFormat, PixelFormatInfo> pixel_format_lut = {
    // Color
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(Bitmap, Invalid),
    // COLOR_PIXEL_FORMAT_ENTRY_RGBA(Unknown1D, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA32Float, RGBA32Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA32Sint, RGBA32Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA32Uint, RGBA32Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX32Float, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX32Sint, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX32Uint, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA16Unorm, RGBA16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA16Snorm, RGBA16Snorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA16Sint, RGBA16Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA16Uint, RGBA16Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA16Float, RGBA16Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG32Float, RG32Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG32Sint, RG32Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG32Uint, RG32Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX16Float, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX16Float, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGRA8Unorm, BGRA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGRA8Srgb, BGRA8Unorm_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGB10A2Unorm, RGB10A2Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGB10A2Uint, RGB10A2Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA8Unorm, RGBA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA8Srgb, RGBA8Unorm_sRGB),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA8Snorm, RGBA8Snorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA8Sint, RGBA8Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBA8Uint, RGBA8Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG16Unorm, RG16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG16Snorm, RG16Snorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG16Sint, RG16Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG16Uint, RG16Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG16Float, RG16Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGR10A2Unorm, BGR10A2Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R11G11B10Float, RG11B10Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R32Sint, R32Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R32Uint, R32Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R32Float, R32Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGRX8Unorm, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGRX8Srgb, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(B5G6R5Unorm, B5G6R5Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGR5A1Unorm, BGR5A1Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG8Unorm, RG8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG8Snorm, RG8Snorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG8Sint, RG8Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RG8Uint, RG8Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R16Unorm, R16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R16Snorm, R16Snorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R16Sint, R16Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R16Uint, R16Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R16Float, R16Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R8Unorm, R8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R8Snorm, R8Snorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R8Sint, R8Sint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R8Uint, R8Uint),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A8Unorm, A8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGR5X1Unorm, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX8Unorm, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(RGBX8Srgb, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGR5X1UnormUnknownFB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGR5X1UnormUnknownFC, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGRX8UnormUnknownFD, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(BGRX8UnormUnknownFE, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(Y32UintUnknownFF, Invalid),

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
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(X4C4, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A4L4, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A8L8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(Float_A16L16, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A1B5G5R5, A1BGR5Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A4B4G4R4, ABGR4Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A5B5G5R1, RGBA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A2B10G10R10, RGB10A2Unorm), // TODO: format
decoder COLOR_PIXEL_FORMAT_ENTRY_RGBA(A8B8G8R8, RGBA8Unorm),      // TODO:
swizzle COLOR_PIXEL_FORMAT_ENTRY_RGBA(A16B16G16R16, RGBA16Unorm), // TODO:
swizzle COLOR_PIXEL_FORMAT_ENTRY_RGBA(Float_A16B16G16R16, RGBA16Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A1R5G5B5, A1BGR5Unorm),     // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A4R4G4B4, ABGR4Unorm),      // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A5R1G5B5, A1BGR5Unorm),     // TODO: format
decoder COLOR_PIXEL_FORMAT_ENTRY_RGBA(A2R10G10B10, RGB10A2Unorm), // TODO:
format decoder COLOR_PIXEL_FORMAT_ENTRY_RGBA(A8R8G8B8, RGBA8Unorm),      //
TODO: swizzle COLOR_PIXEL_FORMAT_ENTRY_RGBA(A1, A8Unorm),               // TODO:
format decoder COLOR_PIXEL_FORMAT_ENTRY_RGBA(A2, A8Unorm),               //
TODO: format decoder COLOR_PIXEL_FORMAT_ENTRY_RGBA(A4, A8Unorm), // TODO: format
decoder COLOR_PIXEL_FORMAT_ENTRY_RGBA(A8, A8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A16, RGBA16Unorm), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A32, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(Float_A16, RGBA16Float), // TODO: format
decoder COLOR_PIXEL_FORMAT_ENTRY_RGBA(L4A4, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(L8A8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(B4G4R4A4, ABGR4Unorm),  // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(B5G5R1A5, A1BGR5Unorm), // TODO: format
decoder COLOR_PIXEL_FORMAT_ENTRY_RGBA(B5G5R5A1, BGR5A1Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(B8G8R8A8, BGRA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(B10G10R10A2, BGR10A2Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R1G5B5A5, A1BGR5Unorm), // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R4G4B4A4, ABGR4Unorm),  // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R5G5B5A1, BGR5A1Unorm), // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R8G8B8A8, RGBA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R10G10B10A2, RGB10A2Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(L1, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(L2, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(L4, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(L8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(L16, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(L32, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(Float_L16, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(B5G6R5, B5G6R5Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(B6G5R5, RGBA8Unorm), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(B5G5R5X1, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(B8G8R8, BGRA8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(B8G8R8X8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(Float_B10G11R11,
                             RG11B10Float), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(X1B5G5R5, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(X8B8G8R8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(X16B16G16R16, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(Float_X16B16G16R16, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R3G3B2, B5G6R5Unorm), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R5G5B6, B5G6R5Unorm), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R5G6B5, B5G6R5Unorm), // TODO: swizzle
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R5G5B5X1, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R8G8B8, RGBA8Unorm), // TODO: format decoder
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R8G8B8X8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(X1R5G5B5, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(X8R8G8B8, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R8G8, RG8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R16G16, RG16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(Float_R16G16, RG16Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R8, R8Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R16, R16Unorm),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(Float_R16, R16Float),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A2B10G10R10_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A8B8G8R8_sRGB, RGBA8Unorm_sRGB), // TODO:
swizzle COLOR_PIXEL_FORMAT_ENTRY_RGBA(A16B16G16R16_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A2R10G10B10_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(B10G10R10A2_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R10G10B10A2_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(X8B8G8R8_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(X16B16G16R16_sRGB, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A2B10G10R10_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A8B8G8R8_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A16B16G16R16_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(A2R10G10B10_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(B10G10R10A2_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(R10G10B10A2_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(X8B8G8R8_709, Invalid),
    COLOR_PIXEL_FORMAT_ENTRY_RGBA(X16B16G16R16_709, Invalid),
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

MTL::CullMode ToMtlCullMode(const engines::CullFaceMode mode) {
    switch (mode) {
    case engines::CullFaceMode::Front:
        return MTL::CullModeFront;
    case engines::CullFaceMode::Back:
        return MTL::CullModeBack;
    case engines::CullFaceMode::FrontAndBack:
        LOG_NOT_IMPLEMENTED(MetalRenderer, "Cull face mode front and back");
        return MTL::CullModeFront;
    }
}

MTL::Winding ToMtlWinding(const engines::Winding winding) {
    switch (winding) {
    case engines::Winding::Clockwise:
        return MTL::WindingClockwise;
    case engines::Winding::CounterClockwise:
        return MTL::WindingCounterClockwise;
    }
}

MTL::PrimitiveType
to_mtl_primitive_type(const engines::PrimitiveType primitive_type) {
    switch (primitive_type) {
    case engines::PrimitiveType::Points:
        return MTL::PrimitiveTypePoint;
    case engines::PrimitiveType::Lines:
        return MTL::PrimitiveTypeLine;
    case engines::PrimitiveType::LineStrip:
        return MTL::PrimitiveTypeLineStrip;
    case engines::PrimitiveType::Triangles:
        return MTL::PrimitiveTypeTriangle;
    case engines::PrimitiveType::TriangleStrip:
        return MTL::PrimitiveTypeTriangleStrip;
    default:
        LOG_NOT_IMPLEMENTED(MetalRenderer, "Primitive type {}", primitive_type);
        return MTL::PrimitiveTypeTriangle;
    }
}

MTL::IndexType to_mtl_index_type(engines::IndexType index_type) {
    switch (index_type) {
    case engines::IndexType::UInt16:
        return MTL::IndexTypeUInt16;
    case engines::IndexType::UInt32:
        return MTL::IndexTypeUInt32;
    default:
        LOG_NOT_IMPLEMENTED(MetalRenderer, "Index type {}", index_type);
        return MTL::IndexTypeUInt16;
    }
}

MTL::VertexFormat to_mtl_vertex_format(engines::VertexAttribType type,
                                       engines::VertexAttribSize size,
                                       bool bgra) {
    ASSERT_DEBUG(!bgra || (type == engines::VertexAttribType::Unorm &&
                           size == engines::VertexAttribSize::_4x8),
                 MetalRenderer,
                 "BGRA vertex formats are only supported for uchar4 normalized "
                 "types (type: {}, size: {})",
                 type, size);

    switch (type) {
    case engines::VertexAttribType::Snorm:
        switch (size) {
        case engines::VertexAttribSize::_1x32:
        case engines::VertexAttribSize::_2x32:
        case engines::VertexAttribSize::_3x32:
        case engines::VertexAttribSize::_4x32:
            LOG_NOT_IMPLEMENTED(
                MetalRenderer,
                "Signed normalized 32-bit integer vertex formats");
            return MTL::VertexFormatInvalid;

        case engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatShortNormalized;
        case engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatShort2Normalized;
        case engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatShort3Normalized;
        case engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatShort4Normalized;

        case engines::VertexAttribSize::_1x8:
            return MTL::VertexFormatCharNormalized;
        case engines::VertexAttribSize::_2x8:
            return MTL::VertexFormatChar2Normalized;
        case engines::VertexAttribSize::_3x8:
            return MTL::VertexFormatChar3Normalized;
        case engines::VertexAttribSize::_4x8:
            return MTL::VertexFormatChar4Normalized;

        case engines::VertexAttribSize::_10_10_10_2:
            return MTL::VertexFormatInt1010102Normalized;

        default:
            LOG_ERROR(MetalRenderer,
                      "Invalid vertex attribute size {} for snorm type", size);
            return MTL::VertexFormatInvalid;
        }
    case engines::VertexAttribType::Unorm:
        switch (size) {
        case engines::VertexAttribSize::_1x32:
        case engines::VertexAttribSize::_2x32:
        case engines::VertexAttribSize::_3x32:
        case engines::VertexAttribSize::_4x32:
            LOG_NOT_IMPLEMENTED(
                MetalRenderer,
                "Unsigned normalized 32-bit integer vertex formats");
            return MTL::VertexFormatInvalid;

        case engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatUShortNormalized;
        case engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatUShort2Normalized;
        case engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatUShort3Normalized;
        case engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatUShort4Normalized;

        case engines::VertexAttribSize::_1x8:
            return MTL::VertexFormatUCharNormalized;
        case engines::VertexAttribSize::_2x8:
            return MTL::VertexFormatUChar2Normalized;
        case engines::VertexAttribSize::_3x8:
            return MTL::VertexFormatUChar3Normalized;
        case engines::VertexAttribSize::_4x8:
            if (bgra)
                return MTL::VertexFormatUChar4Normalized_BGRA;
            else
                return MTL::VertexFormatUChar4Normalized;

        case engines::VertexAttribSize::_10_10_10_2:
            return MTL::VertexFormatUInt1010102Normalized;

        default:
            LOG_ERROR(MetalRenderer,
                      "Invalid vertex attribute size {} for unorm type", size);
            return MTL::VertexFormatInvalid;
        }
    case engines::VertexAttribType::Sint:
        switch (size) {
        case engines::VertexAttribSize::_1x32:
            return MTL::VertexFormatInt;
        case engines::VertexAttribSize::_2x32:
            return MTL::VertexFormatInt2;
        case engines::VertexAttribSize::_3x32:
            return MTL::VertexFormatInt3;
        case engines::VertexAttribSize::_4x32:
            return MTL::VertexFormatInt4;

        case engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatShort;
        case engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatShort2;
        case engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatShort3;
        case engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatShort4;

        case engines::VertexAttribSize::_1x8:
            return MTL::VertexFormatChar;
        case engines::VertexAttribSize::_2x8:
            return MTL::VertexFormatChar2;
        case engines::VertexAttribSize::_3x8:
            return MTL::VertexFormatChar3;
        case engines::VertexAttribSize::_4x8:
            return MTL::VertexFormatChar4;

        default:
            LOG_ERROR(MetalRenderer,
                      "Invalid vertex attribute size {} for sint type", size);
            return MTL::VertexFormatInvalid;
        }
    case engines::VertexAttribType::Uint:
        switch (size) {
        case engines::VertexAttribSize::_1x32:
            return MTL::VertexFormatUInt;
        case engines::VertexAttribSize::_2x32:
            return MTL::VertexFormatUInt2;
        case engines::VertexAttribSize::_3x32:
            return MTL::VertexFormatUInt3;
        case engines::VertexAttribSize::_4x32:
            return MTL::VertexFormatUInt4;

        case engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatUShort;
        case engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatUShort2;
        case engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatUShort3;
        case engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatUShort4;

        case engines::VertexAttribSize::_1x8:
            return MTL::VertexFormatUChar;
        case engines::VertexAttribSize::_2x8:
            return MTL::VertexFormatUChar2;
        case engines::VertexAttribSize::_3x8:
            return MTL::VertexFormatUChar3;
        case engines::VertexAttribSize::_4x8:
            return MTL::VertexFormatUChar4;

        default:
            LOG_ERROR(MetalRenderer,
                      "Invalid vertex attribute size {} for uint type", size);
            return MTL::VertexFormatInvalid;
        }
    case engines::VertexAttribType::Sscaled:
        switch (size) {
        case engines::VertexAttribSize::_1x32:
            return MTL::VertexFormatInt;
        case engines::VertexAttribSize::_2x32:
            return MTL::VertexFormatInt2;
        case engines::VertexAttribSize::_3x32:
            return MTL::VertexFormatInt3;
        case engines::VertexAttribSize::_4x32:
            return MTL::VertexFormatInt4;

        case engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatShort;
        case engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatShort2;
        case engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatShort3;
        case engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatShort4;

        case engines::VertexAttribSize::_1x8:
            return MTL::VertexFormatChar;
        case engines::VertexAttribSize::_2x8:
            return MTL::VertexFormatChar2;
        case engines::VertexAttribSize::_3x8:
            return MTL::VertexFormatChar3;
        case engines::VertexAttribSize::_4x8:
            return MTL::VertexFormatChar4;

        default:
            LOG_ERROR(MetalRenderer,
                      "Invalid vertex attribute size {} for sint type", size);
            return MTL::VertexFormatInvalid;
        }
    case engines::VertexAttribType::Uscaled:
        switch (size) {
        case engines::VertexAttribSize::_1x32:
            return MTL::VertexFormatUInt;
        case engines::VertexAttribSize::_2x32:
            return MTL::VertexFormatUInt2;
        case engines::VertexAttribSize::_3x32:
            return MTL::VertexFormatUInt3;
        case engines::VertexAttribSize::_4x32:
            return MTL::VertexFormatUInt4;

        case engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatUShort;
        case engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatUShort2;
        case engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatUShort3;
        case engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatUShort4;

        case engines::VertexAttribSize::_1x8:
            return MTL::VertexFormatUChar;
        case engines::VertexAttribSize::_2x8:
            return MTL::VertexFormatUChar2;
        case engines::VertexAttribSize::_3x8:
            return MTL::VertexFormatUChar3;
        case engines::VertexAttribSize::_4x8:
            return MTL::VertexFormatUChar4;

        default:
            LOG_ERROR(MetalRenderer,
                      "Invalid vertex attribute size {} for uint type", size);
            return MTL::VertexFormatInvalid;
        }
    case engines::VertexAttribType::Float:
        switch (size) {
        case engines::VertexAttribSize::_1x32:
            return MTL::VertexFormatFloat;
        case engines::VertexAttribSize::_2x32:
            return MTL::VertexFormatFloat2;
        case engines::VertexAttribSize::_3x32:
            return MTL::VertexFormatFloat3;
        case engines::VertexAttribSize::_4x32:
            return MTL::VertexFormatFloat4;

        case engines::VertexAttribSize::_1x16:
            return MTL::VertexFormatHalf;
        case engines::VertexAttribSize::_2x16:
            return MTL::VertexFormatHalf2;
        case engines::VertexAttribSize::_3x16:
            return MTL::VertexFormatHalf3;
        case engines::VertexAttribSize::_4x16:
            return MTL::VertexFormatHalf4;

        case engines::VertexAttribSize::_1x8:
        case engines::VertexAttribSize::_2x8:
        case engines::VertexAttribSize::_3x8:
        case engines::VertexAttribSize::_4x8:
            LOG_NOT_IMPLEMENTED(MetalRenderer,
                                "8-bit floating-point vertex formats");
            return MTL::VertexFormatInvalid;

        case engines::VertexAttribSize::_11_11_10:
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

MTL::CompareFunction to_mtl_compare_func(engines::CompareOp depth_compare_op) {
    switch (depth_compare_op) {
    case engines::CompareOp::Never:
        return MTL::CompareFunctionNever;
    case engines::CompareOp::Less:
        return MTL::CompareFunctionLess;
    case engines::CompareOp::Equal:
        return MTL::CompareFunctionEqual;
    case engines::CompareOp::LessEqual:
        return MTL::CompareFunctionLessEqual;
    case engines::CompareOp::Greater:
        return MTL::CompareFunctionGreater;
    case engines::CompareOp::NotEqual:
        return MTL::CompareFunctionNotEqual;
    case engines::CompareOp::GreaterEqual:
        return MTL::CompareFunctionGreaterEqual;
    case engines::CompareOp::Always:
        return MTL::CompareFunctionAlways;
    default:
        LOG_NOT_IMPLEMENTED(MetalRenderer, "Depth test func {}",
                            depth_compare_op);
        return MTL::CompareFunctionAlways;
    }
}

MTL::TextureSwizzle to_mtl_swizzle(const ImageSwizzle swizzle) {
    switch (swizzle) {
    case ImageSwizzle::Zero:
        return MTL::TextureSwizzleZero;
    case ImageSwizzle::R:
        return MTL::TextureSwizzleRed;
    case ImageSwizzle::G:
        return MTL::TextureSwizzleGreen;
    case ImageSwizzle::B:
        return MTL::TextureSwizzleBlue;
    case ImageSwizzle::A:
        return MTL::TextureSwizzleAlpha;
    case ImageSwizzle::OneInt:
    case ImageSwizzle::OneFloat:
        return MTL::TextureSwizzleOne;
    }
}

MTL::BlendOperation to_mtl_blend_operation(const BlendOperation blend_op) {
    switch (blend_op) {
    case BlendOperation::Add:
        return MTL::BlendOperationAdd;
    case BlendOperation::Sub:
        return MTL::BlendOperationSubtract;
    case BlendOperation::RevSub:
        return MTL::BlendOperationReverseSubtract;
    case BlendOperation::Min:
        return MTL::BlendOperationMin;
    case BlendOperation::Max:
        return MTL::BlendOperationMax;
    }
}

MTL::BlendFactor to_mtl_blend_factor(const BlendFactor blend_factor) {
    switch (blend_factor) {
    case BlendFactor::Zero:
        return MTL::BlendFactorZero;
    case BlendFactor::One:
        return MTL::BlendFactorOne;
    case BlendFactor::SrcColor:
        return MTL::BlendFactorSourceColor;
    case BlendFactor::InvSrcColor:
        return MTL::BlendFactorOneMinusSourceColor;
    case BlendFactor::SrcAlpha:
        return MTL::BlendFactorSourceAlpha;
    case BlendFactor::InvSrcAlpha:
        return MTL::BlendFactorOneMinusSourceAlpha;
    case BlendFactor::DstAlpha:
        return MTL::BlendFactorDestinationAlpha;
    case BlendFactor::InvDstAlpha:
        return MTL::BlendFactorOneMinusDestinationAlpha;
    case BlendFactor::DstColor:
        return MTL::BlendFactorDestinationColor;
    case BlendFactor::InvDstColor:
        return MTL::BlendFactorOneMinusDestinationColor;
    case BlendFactor::SrcAlphaSaturate:
        return MTL::BlendFactorSourceAlphaSaturated;
    case BlendFactor::Src1Color:
        return MTL::BlendFactorSource1Color;
    case BlendFactor::InvSrc1Color:
        return MTL::BlendFactorOneMinusSource1Color;
    case BlendFactor::Src1Alpha:
        return MTL::BlendFactorSource1Alpha;
    case BlendFactor::InvSrc1Alpha:
        return MTL::BlendFactorOneMinusSource1Alpha;
    case BlendFactor::ConstColor:
        return MTL::BlendFactorBlendColor;
    case BlendFactor::InvConstColor:
        return MTL::BlendFactorOneMinusBlendColor;
    case BlendFactor::ConstAlpha:
        return MTL::BlendFactorBlendAlpha;
    case BlendFactor::InvConstAlpha:
        return MTL::BlendFactorOneMinusBlendAlpha;
    }
}

MTL::SamplerMinMagFilter
to_mtl_sampler_min_mag_filter(const SamplerFilter filter) {
    switch (filter) {
    case SamplerFilter::Nearest:
        return MTL::SamplerMinMagFilterNearest;
    case SamplerFilter::Linear:
        return MTL::SamplerMinMagFilterLinear;
    }
}

MTL::SamplerMipFilter to_mtl_sampler_mip_filter(const SamplerMipFilter filter) {
    switch (filter) {
    case SamplerMipFilter::NotMipmapped:
        return MTL::SamplerMipFilterNotMipmapped;
    case SamplerMipFilter::Nearest:
        return MTL::SamplerMipFilterNearest;
    case SamplerMipFilter::Linear:
        return MTL::SamplerMipFilterLinear;
    }
}

MTL::SamplerAddressMode
to_mtl_sampler_address_mode(const SamplerAddressMode address_mode) {
    switch (address_mode) {
    case SamplerAddressMode::Repeat:
        return MTL::SamplerAddressModeRepeat;
    case SamplerAddressMode::MirroredRepeat:
        return MTL::SamplerAddressModeMirrorRepeat;
    case SamplerAddressMode::ClampToEdge:
        return MTL::SamplerAddressModeClampToEdge;
    case SamplerAddressMode::ClampToBorder:
        LOG_NOT_IMPLEMENTED(MetalRenderer, "ClampToBorder");
        return MTL::SamplerAddressModeClampToBorderColor;
    case SamplerAddressMode::Clamp:
        LOG_NOT_IMPLEMENTED(MetalRenderer, "Clamp");
        return MTL::SamplerAddressModeClampToEdge;
    case SamplerAddressMode::MirrorClampToEdge:
        return MTL::SamplerAddressModeMirrorClampToEdge;
    case SamplerAddressMode::MirrorClampToBorder:
        LOG_NOT_IMPLEMENTED(MetalRenderer, "MirrorClampToBorder");
        return MTL::SamplerAddressModeClampToBorderColor;
    case SamplerAddressMode::MirrorClamp:
        LOG_NOT_IMPLEMENTED(MetalRenderer, "MirrorClamp");
        return MTL::SamplerAddressModeMirrorClampToEdge;
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
