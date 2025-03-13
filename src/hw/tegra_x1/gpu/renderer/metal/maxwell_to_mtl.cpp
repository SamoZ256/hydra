#include "hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer::Metal {

#define PIXEL_FORMAT_ENTRY(surface_format, pixel_format, has_stencil)          \
    {                                                                          \
        SurfaceFormat::surface_format, {                                       \
            MTL::PixelFormat##pixel_format, has_stencil                        \
        }                                                                      \
    }

#define COLOR_PIXEL_FORMAT_ENTRY(surface_format, pixel_format)                 \
    PIXEL_FORMAT_ENTRY(surface_format, pixel_format, false)

std::map<SurfaceFormat, PixelFormatInfo> pixel_format_lut = {
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

const PixelFormatInfo& get_mtl_pixel_format_info(SurfaceFormat surface_format) {
    auto it = pixel_format_lut.find(surface_format);
    ASSERT_DEBUG(it != pixel_format_lut.end(), MetalRenderer,
                 "Unknown surface format {}", surface_format);

    const auto& info = it->second;
    ASSERT_DEBUG(info.pixel_format != MTL::PixelFormatInvalid, MetalRenderer,
                 "Unimplemented surface format {}", surface_format);

    return info;
}

} // namespace Hydra::HW::TegraX1::GPU::Renderer::Metal
