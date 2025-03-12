#pragma once

#include "common/common.hpp"
#include "common/macros.hpp"

namespace Hydra::HW::TegraX1::GPU {

enum class NvLayout : u32 {
    Pitch = 1,
    Tiled = 2,
    BlockLinear = 3,
};

enum class NvDisplayScanFormat : u32 {
    Progressive = 0,
    Interlaced = 1,
};

enum class NvKind : u32 {
    Pitch = 0x0,
    Z16 = 0x1,
    Z16_2C = 0x2,
    Z16_MS2_2C = 0x3,
    Z16_MS4_2C = 0x4,
    Z16_MS8_2C = 0x5,
    Z16_MS16_2C = 0x6,
    Z16_2Z = 0x7,
    Z16_MS2_2Z = 0x8,
    Z16_MS4_2Z = 0x9,
    Z16_MS8_2Z = 0xa,
    Z16_MS16_2Z = 0xb,
    Z16_4CZ = 0xc,
    Z16_MS2_4CZ = 0xd,
    Z16_MS4_4CZ = 0xe,
    Z16_MS8_4CZ = 0xf,
    Z16_MS16_4CZ = 0x10,
    S8Z24 = 0x11,
    S8Z24_1Z = 0x12,
    S8Z24_MS2_1Z = 0x13,
    S8Z24_MS4_1Z = 0x14,
    S8Z24_MS8_1Z = 0x15,
    S8Z24_MS16_1Z = 0x16,
    S8Z24_2CZ = 0x17,
    S8Z24_MS2_2CZ = 0x18,
    S8Z24_MS4_2CZ = 0x19,
    S8Z24_MS8_2CZ = 0x1a,
    S8Z24_MS16_2CZ = 0x1b,
    S8Z24_2CS = 0x1C,
    S8Z24_MS2_2CS = 0x1d,
    S8Z24_MS4_2CS = 0x1e,
    S8Z24_MS8_2CS = 0x1f,
    S8Z24_MS16_2CS = 0x20,
    S8Z24_4CSZV = 0x21,
    S8Z24_MS2_4CSZV = 0x22,
    S8Z24_MS4_4CSZV = 0x23,
    S8Z24_MS8_4CSZV = 0x24,
    S8Z24_MS16_4CSZV = 0x25,
    V8Z24_MS4_VC12 = 0x26,
    V8Z24_MS4_VC4 = 0x27,
    V8Z24_MS8_VC8 = 0x28,
    V8Z24_MS8_VC24 = 0x29,
    S8 = 0x2a,
    S8_2S = 0x2b,
    V8Z24_MS4_VC12_1ZV = 0x2e,
    V8Z24_MS4_VC4_1ZV = 0x2f,
    V8Z24_MS8_VC8_1ZV = 0x30,
    V8Z24_MS8_VC24_1ZV = 0x31,
    V8Z24_MS4_VC12_2CS = 0x32,
    V8Z24_MS4_VC4_2CS = 0x33,
    V8Z24_MS8_VC8_2CS = 0x34,
    V8Z24_MS8_VC24_2CS = 0x35,
    V8Z24_MS4_VC12_2CZV = 0x3a,
    V8Z24_MS4_VC4_2CZV = 0x3b,
    V8Z24_MS8_VC8_2CZV = 0x3c,
    V8Z24_MS8_VC24_2CZV = 0x3d,
    V8Z24_MS4_VC12_2ZV = 0x3e,
    V8Z24_MS4_VC4_2ZV = 0x3f,
    V8Z24_MS8_VC8_2ZV = 0x40,
    V8Z24_MS8_VC24_2ZV = 0x41,
    V8Z24_MS4_VC12_4CSZV = 0x42,
    V8Z24_MS4_VC4_4CSZV = 0x43,
    V8Z24_MS8_VC8_4CSZV = 0x44,
    V8Z24_MS8_VC24_4CSZV = 0x45,
    Z24S8 = 0x46,
    Z24S8_1Z = 0x47,
    Z24S8_MS2_1Z = 0x48,
    Z24S8_MS4_1Z = 0x49,
    Z24S8_MS8_1Z = 0x4a,
    Z24S8_MS16_1Z = 0x4b,
    Z24S8_2CS = 0x4c,
    Z24S8_MS2_2CS = 0x4d,
    Z24S8_MS4_2CS = 0x4e,
    Z24S8_MS8_2CS = 0x4f,
    Z24S8_MS16_2CS = 0x50,
    Z24S8_2CZ = 0x51,
    Z24S8_MS2_2CZ = 0x52,
    Z24S8_MS4_2CZ = 0x53,
    Z24S8_MS8_2CZ = 0x54,
    Z24S8_MS16_2CZ = 0x55,
    Z24S8_4CSZV = 0x56,
    Z24S8_MS2_4CSZV = 0x57,
    Z24S8_MS4_4CSZV = 0x58,
    Z24S8_MS8_4CSZV = 0x59,
    Z24S8_MS16_4CSZV = 0x5a,
    Z24V8_MS4_VC12 = 0x5b,
    Z24V8_MS4_VC4 = 0x5C,
    Z24V8_MS8_VC8 = 0x5d,
    Z24V8_MS8_VC24 = 0x5e,
    Z24V8_MS4_VC12_1ZV = 0x63,
    Z24V8_MS4_VC4_1ZV = 0x64,
    Z24V8_MS8_VC8_1ZV = 0x65,
    Z24V8_MS8_VC24_1ZV = 0x66,
    Z24V8_MS4_VC12_2CS = 0x67,
    Z24V8_MS4_VC4_2CS = 0x68,
    Z24V8_MS8_VC8_2CS = 0x69,
    Z24V8_MS8_VC24_2CS = 0x6a,
    Z24V8_MS4_VC12_2CZV = 0x6f,
    Z24V8_MS4_VC4_2CZV = 0x70,
    Z24V8_MS8_VC8_2CZV = 0x71,
    Z24V8_MS8_VC24_2CZV = 0x72,
    Z24V8_MS4_VC12_2ZV = 0x73,
    Z24V8_MS4_VC4_2ZV = 0x74,
    Z24V8_MS8_VC8_2ZV = 0x75,
    Z24V8_MS8_VC24_2ZV = 0x76,
    Z24V8_MS4_VC12_4CSZV = 0x77,
    Z24V8_MS4_VC4_4CSZV = 0x78,
    Z24V8_MS8_VC8_4CSZV = 0x79,
    Z24V8_MS8_VC24_4CSZV = 0x7a,
    ZF32 = 0x7b,
    ZF32_1Z = 0x7C,
    ZF32_MS2_1Z = 0x7d,
    ZF32_MS4_1Z = 0x7e,
    ZF32_MS8_1Z = 0x7f,
    ZF32_MS16_1Z = 0x80,
    ZF32_2CS = 0x81,
    ZF32_MS2_2CS = 0x82,
    ZF32_MS4_2CS = 0x83,
    ZF32_MS8_2CS = 0x84,
    ZF32_MS16_2CS = 0x85,
    ZF32_2CZ = 0x86,
    ZF32_MS2_2CZ = 0x87,
    ZF32_MS4_2CZ = 0x88,
    ZF32_MS8_2CZ = 0x89,
    ZF32_MS16_2CZ = 0x8a,
    X8Z24_X16V8S8_MS4_VC12 = 0x8b,
    X8Z24_X16V8S8_MS4_VC4 = 0x8c,
    X8Z24_X16V8S8_MS8_VC8 = 0x8d,
    X8Z24_X16V8S8_MS8_VC24 = 0x8e,
    X8Z24_X16V8S8_MS4_VC12_1CS = 0x8f,
    X8Z24_X16V8S8_MS4_VC4_1CS = 0x90,
    X8Z24_X16V8S8_MS8_VC8_1CS = 0x91,
    X8Z24_X16V8S8_MS8_VC24_1CS = 0x92,
    X8Z24_X16V8S8_MS4_VC12_1ZV = 0x97,
    X8Z24_X16V8S8_MS4_VC4_1ZV = 0x98,
    X8Z24_X16V8S8_MS8_VC8_1ZV = 0x99,
    X8Z24_X16V8S8_MS8_VC24_1ZV = 0x9a,
    X8Z24_X16V8S8_MS4_VC12_1CZV = 0x9b,
    X8Z24_X16V8S8_MS4_VC4_1CZV = 0x9c,
    X8Z24_X16V8S8_MS8_VC8_1CZV = 0x9d,
    X8Z24_X16V8S8_MS8_VC24_1CZV = 0x9e,
    X8Z24_X16V8S8_MS4_VC12_2CS = 0x9f,
    X8Z24_X16V8S8_MS4_VC4_2CS = 0xa0,
    X8Z24_X16V8S8_MS8_VC8_2CS = 0xa1,
    X8Z24_X16V8S8_MS8_VC24_2CS = 0xa2,
    X8Z24_X16V8S8_MS4_VC12_2CSZV = 0xa3,
    X8Z24_X16V8S8_MS4_VC4_2CSZV = 0xa4,
    X8Z24_X16V8S8_MS8_VC8_2CSZV = 0xa5,
    X8Z24_X16V8S8_MS8_VC24_2CSZV = 0xa6,
    ZF32_X16V8S8_MS4_VC12 = 0xa7,
    ZF32_X16V8S8_MS4_VC4 = 0xa8,
    ZF32_X16V8S8_MS8_VC8 = 0xa9,
    ZF32_X16V8S8_MS8_VC24 = 0xaa,
    ZF32_X16V8S8_MS4_VC12_1CS = 0xab,
    ZF32_X16V8S8_MS4_VC4_1CS = 0xac,
    ZF32_X16V8S8_MS8_VC8_1CS = 0xad,
    ZF32_X16V8S8_MS8_VC24_1CS = 0xae,
    ZF32_X16V8S8_MS4_VC12_1ZV = 0xb3,
    ZF32_X16V8S8_MS4_VC4_1ZV = 0xb4,
    ZF32_X16V8S8_MS8_VC8_1ZV = 0xb5,
    ZF32_X16V8S8_MS8_VC24_1ZV = 0xb6,
    ZF32_X16V8S8_MS4_VC12_1CZV = 0xb7,
    ZF32_X16V8S8_MS4_VC4_1CZV = 0xb8,
    ZF32_X16V8S8_MS8_VC8_1CZV = 0xb9,
    ZF32_X16V8S8_MS8_VC24_1CZV = 0xba,
    ZF32_X16V8S8_MS4_VC12_2CS = 0xbb,
    ZF32_X16V8S8_MS4_VC4_2CS = 0xbc,
    ZF32_X16V8S8_MS8_VC8_2CS = 0xbd,
    ZF32_X16V8S8_MS8_VC24_2CS = 0xbe,
    ZF32_X16V8S8_MS4_VC12_2CSZV = 0xbf,
    ZF32_X16V8S8_MS4_VC4_2CSZV = 0xc0,
    ZF32_X16V8S8_MS8_VC8_2CSZV = 0xc1,
    ZF32_X16V8S8_MS8_VC24_2CSZV = 0xc2,
    ZF32_X24S8 = 0xc3,
    ZF32_X24S8_1CS = 0xc4,
    ZF32_X24S8_MS2_1CS = 0xc5,
    ZF32_X24S8_MS4_1CS = 0xc6,
    ZF32_X24S8_MS8_1CS = 0xc7,
    ZF32_X24S8_MS16_1CS = 0xc8,
    SmskedMessage = 0xca,
    SmhostMessage = 0xcb,
    C64_MS2_2CRA = 0xcd,
    ZF32_X24S8_2CSZV = 0xce,
    ZF32_X24S8_MS2_2CSZV = 0xcf,
    ZF32_X24S8_MS4_2CSZV = 0xd0,
    ZF32_X24S8_MS8_2CSZV = 0xd1,
    ZF32_X24S8_MS16_2CSZV = 0xd2,
    ZF32_X24S8_2CS = 0xd3,
    ZF32_X24S8_MS2_2CS = 0xd4,
    ZF32_X24S8_MS4_2CS = 0xd5,
    ZF32_X24S8_MS8_2CS = 0xd6,
    ZF32_X24S8_MS16_2CS = 0xd7,
    C32_2C = 0xd8,
    C32_2CBR = 0xd9,
    C32_2CBA = 0xda,
    C32_2CRA = 0xdb,
    C32_2BRA = 0xdc,
    C32_MS2_2C = 0xdd,
    C32_MS2_2CBR = 0xde,
    C32_MS2_2CRA = 0xcc,
    C32_MS4_2C = 0xdf,
    C32_MS4_2CBR = 0xe0,
    C32_MS4_2CBA = 0xe1,
    C32_MS4_2CRA = 0xe2,
    C32_MS4_2BRA = 0xe3,
    C32_MS8_MS16_2C = 0xe4,
    C32_MS8_MS16_2CRA = 0xe5,
    C64_2C = 0xe6,
    C64_2CBR = 0xe7,
    C64_2CBA = 0xe8,
    C64_2CRA = 0xe9,
    C64_2BRA = 0xea,
    C64_MS2_2C = 0xeb,
    C64_MS2_2CBR = 0xec,
    C64_MS4_2C = 0xed,
    C64_MS4_2CBR = 0xee,
    C64_MS4_2CBA = 0xef,
    C64_MS4_2CRA = 0xf0,
    C64_MS4_2BRA = 0xf1,
    C64_MS8_MS16_2C = 0xf2,
    C64_MS8_MS16_2CRA = 0xf3,
    C128_2C = 0xf4,
    C128_2CR = 0xf5,
    C128_MS2_2C = 0xf6,
    C128_MS2_2CR = 0xf7,
    C128_MS4_2C = 0xf8,
    C128_MS4_2CR = 0xf9,
    C128_MS8_MS16_2C = 0xfa,
    C128_MS8_MS16_2CR = 0xfb,
    X8C24 = 0xfc,
    PitchNoSwizzle = 0xfd,
    Generic_16BX2 = 0xfe,
    Invalid = 0xff,
};

// TODO: why do some formats have "_" in them?
enum class NvColorFormat : u64 {
    Unspecified = 0x0000000000UL,
    NonColor8 = 0x0009200408UL,
    NonColor16 = 0x0009200A10UL,
    NonColor24 = 0x0009201A18UL,
    NonColor32 = 0x0009201C20UL,
    X4C4 = 0x0009210508UL,
    A4L4 = 0x0100490508UL,
    A8L8 = 0x0100490E10UL,
    Float_A16L16 = 0x0100495D20UL,
    A1B5G5R5 = 0x0100531410UL,
    A4B4G4R4 = 0x0100531510UL,
    A5B5G5R1 = 0x0100531810UL,
    A2B10G10R10 = 0x0100532020UL,
    A8B8G8R8 = 0x0100532120UL,
    A16B16G16R16 = 0x0100532740UL,
    Float_A16B16G16R16 = 0x0100536740UL,
    A1R5G5B5 = 0x0100D11410UL,
    A4R4G4B4 = 0x0100D11510UL,
    A5R1G5B5 = 0x0100D11610UL,
    A2R10G10B10 = 0x0100D12020UL,
    A8R8G8B8 = 0x0100D12120UL,
    A1 = 0x0101240101UL,
    A2 = 0x0101240202UL,
    A4 = 0x0101240304UL,
    A8 = 0x0101240408UL,
    A16 = 0x0101240A10UL,
    A32 = 0x0101241C20UL,
    Float_A16 = 0x0101244A10UL,
    L4A4 = 0x0102000508UL,
    L8A8 = 0x0102000E10UL,
    B4G4R4A4 = 0x01060A1510UL,
    B5G5R1A5 = 0x01060A1710UL,
    B5G5R5A1 = 0x01060A1810UL,
    B8G8R8A8 = 0x01060A2120UL,
    B10G10R10A2 = 0x01060A2320UL,
    R1G5B5A5 = 0x0106881410UL,
    R4G4B4A4 = 0x0106881510UL,
    R5G5B5A1 = 0x0106881810UL,
    R8G8B8A8 = 0x0106882120UL,
    R10G10B10A2 = 0x0106882320UL,
    L1 = 0x010A000101UL,
    L2 = 0x010A000202UL,
    L4 = 0x010A000304UL,
    L8 = 0x010A000408UL,
    L16 = 0x010A000A10UL,
    L32 = 0x010A001C20UL,
    Float_L16 = 0x010A004A10UL,
    B5G6R5 = 0x010A0A1210UL,
    B6G5R5 = 0x010A0A1310UL,
    B5G5R5X1 = 0x010A0A1810UL,
    B8G8R8 = 0x010A0A1918UL,
    B8G8R8X8 = 0x010A0A2120UL,
    Float_B10G11R11 = 0x010A0A5E20UL,
    X1B5G5R5 = 0x010A531410UL,
    X8B8G8R8 = 0x010A532120UL,
    X16B16G16R16 = 0x010A532740UL,
    Float_X16B16G16R16 = 0x010A536740UL,
    R3G3B2 = 0x010A880608UL,
    R5G5B6 = 0x010A881110UL,
    R5G6B5 = 0x010A881210UL,
    R5G5B5X1 = 0x010A881810UL,
    R8G8B8 = 0x010A881918UL,
    R8G8B8X8 = 0x010A882120UL,
    X1R5G5B5 = 0x010AD11410UL,
    X8R8G8B8 = 0x010AD12120UL,
    R8G8 = 0x010B080E10UL,
    R16G16 = 0x010B081D20UL,
    Float_R16G16 = 0x010B085D20UL,
    R8 = 0x010B200408UL,
    R16 = 0x010B200A10UL,
    Float_R16 = 0x010B204A10UL,
    A2B10G10R10_sRGB = 0x0200532020UL,
    A8B8G8R8_sRGB = 0x0200532120UL,
    A16B16G16R16_sRGB = 0x0200532740UL,
    A2R10G10B10_sRGB = 0x0200D12020UL,
    B10G10R10A2_sRGB = 0x02060A2320UL,
    R10G10B10A2_sRGB = 0x0206882320UL,
    X8B8G8R8_sRGB = 0x020A532120UL,
    X16B16G16R16_sRGB = 0x020A532740UL,
    A2B10G10R10_709 = 0x0300532020UL,
    A8B8G8R8_709 = 0x0300532120UL,
    A16B16G16R16_709 = 0x0300532740UL,
    A2R10G10B10_709 = 0x0300D12020UL,
    B10G10R10A2_709 = 0x03060A2320UL,
    R10G10B10A2_709 = 0x0306882320UL,
    X8B8G8R8_709 = 0x030A532120UL,
    X16B16G16R16_709 = 0x030A532740UL,
    A2B10G10R10_709_Linear = 0x0400532020UL,
    A8B8G8R8_709_Linear = 0x0400532120UL,
    A16B16G16R16_709_Linear = 0x0400532740UL,
    A2R10G10B10_709_Linear = 0x0400D12020UL,
    B10G10R10A2_709_Linear = 0x04060A2320UL,
    R10G10B10A2_709_Linear = 0x0406882320UL,
    X8B8G8R8_709_Linear = 0x040A532120UL,
    X16B16G16R16_709_Linear = 0x040A532740UL,
    Float_A16B16G16R16_scRGB_Linear = 0x0500536740UL,
    A2B10G10R10_2020 = 0x0600532020UL,
    A8B8G8R8_2020 = 0x0600532120UL,
    A16B16G16R16_2020 = 0x0600532740UL,
    A2R10G10B10_2020 = 0x0600D12020UL,
    B10G10R10A2_2020 = 0x06060A2320UL,
    R10G10B10A2_2020 = 0x0606882320UL,
    X8B8G8R8_2020 = 0x060A532120UL,
    X16B16G16R16_2020 = 0x060A532740UL,
    A2B10G10R10_2020_Linear = 0x0700532020UL,
    A8B8G8R8_2020_Linear = 0x0700532120UL,
    A16B16G16R16_2020_Linear = 0x0700532740UL,
    Float_A16B16G16R16_2020_Linear = 0x0700536740UL,
    A2R10G10B10_2020_Linear = 0x0700D12020UL,
    B10G10R10A2_2020_Linear = 0x07060A2320UL,
    R10G10B10A2_2020_Linear = 0x0706882320UL,
    X8B8G8R8_2020_Linear = 0x070A532120UL,
    X16B16G16R16_2020_Linear = 0x070A532740UL,
    Float_A16B16G16R16_2020_PQ = 0x0800536740UL,
    A4I4 = 0x0901210508UL,
    A8I8 = 0x0901210E10UL,
    I4A4 = 0x0903200508UL,
    I8A8 = 0x0903200E10UL,
    I1 = 0x0909200101UL,
    I2 = 0x0909200202UL,
    I4 = 0x0909200304UL,
    I8 = 0x0909200408UL,
    A8Y8U8V8 = 0x0A00D12120UL,
    A16Y16U16V16 = 0x0A00D12740UL,
    Y8U8V8A8 = 0x0A06882120UL,
    V8_U8 = 0x0A080C0710UL,
    V8U8 = 0x0A080C0E10UL,
    V10U10 = 0x0A08142220UL,
    V12U12 = 0x0A08142420UL,
    V8 = 0x0A08240408UL,
    V10 = 0x0A08240F10UL,
    V12 = 0x0A08241010UL,
    U8_V8 = 0x0A08440710UL,
    U8V8 = 0x0A08440E10UL,
    U10V10 = 0x0A08842220UL,
    U12V12 = 0x0A08842420UL,
    U8 = 0x0A09040408UL,
    U10 = 0x0A09040F10UL,
    U12 = 0x0A09041010UL,
    Y8 = 0x0A09200408UL,
    Y10 = 0x0A09200F10UL,
    Y12 = 0x0A09201010UL,
    YVYU = 0x0A0A500810UL,
    VYUY = 0x0A0A500910UL,
    YUYV = 0x0A0A880810UL,
    UYVY = 0x0A0A880910UL,
    Y8_U8_V8 = 0x0A0A881918UL,
    V8_U8_RR = 0x0B080C0710UL,
    V8U8_RR = 0x0B080C0E10UL,
    V8_RR = 0x0B08240408UL,
    U8_V8_RR = 0x0B08440710UL,
    U8V8_RR = 0x0B08440E10UL,
    U8_RR = 0x0B09040408UL,
    Y8_RR = 0x0B09200408UL,
    V8_U8_ER = 0x0C080C0710UL,
    V8U8_ER = 0x0C080C0E10UL,
    V8_ER = 0x0C08240408UL,
    U8_V8_ER = 0x0C08440710UL,
    U8V8_ER = 0x0C08440E10UL,
    U8_ER = 0x0C09040408UL,
    Y8_ER = 0x0C09200408UL,
    V8_U8_709 = 0x0D080C0710UL,
    V8U8_709 = 0x0D080C0E10UL,
    V10U10_709 = 0x0D08142220UL,
    V12U12_709 = 0x0D08142420UL,
    V8_709 = 0x0D08240408UL,
    V10_709 = 0x0D08240F10UL,
    V12_709 = 0x0D08241010UL,
    U8_V8_709 = 0x0D08440710UL,
    U8V8_709 = 0x0D08440E10UL,
    U10V10_709 = 0x0D08842220UL,
    U12V12_709 = 0x0D08842420UL,
    U8_709 = 0x0D09040408UL,
    U10_709 = 0x0D09040F10UL,
    U12_709 = 0x0D09041010UL,
    Y8_709 = 0x0D09200408UL,
    Y10_709 = 0x0D09200F10UL,
    Y12_709 = 0x0D09201010UL,
    V8_U8_709_ER = 0x0E080C0710UL,
    V8U8_709_ER = 0x0E080C0E10UL,
    V10U10_709_ER = 0x0E08142220UL,
    V12U12_709_ER = 0x0E08142420UL,
    V8_709_ER = 0x0E08240408UL,
    V10_709_ER = 0x0E08240F10UL,
    V12_709_ER = 0x0E08241010UL,
    U8_V8_709_ER = 0x0E08440710UL,
    U8V8_709_ER = 0x0E08440E10UL,
    U10V10_709_ER = 0x0E08842220UL,
    U12V12_709_ER = 0x0E08842420UL,
    U8_709_ER = 0x0E09040408UL,
    U10_709_ER = 0x0E09040F10UL,
    U12_709_ER = 0x0E09041010UL,
    Y8_709_ER = 0x0E09200408UL,
    Y10_709_ER = 0x0E09200F10UL,
    Y12_709_ER = 0x0E09201010UL,
    V10U10_2020 = 0x0F08142220UL,
    V12U12_2020 = 0x0F08142420UL,
    V10_2020 = 0x0F08240F10UL,
    V12_2020 = 0x0F08241010UL,
    U10V10_2020 = 0x0F08842220UL,
    U12V12_2020 = 0x0F08842420UL,
    U10_2020 = 0x0F09040F10UL,
    U12_2020 = 0x0F09041010UL,
    Y10_2020 = 0x0F09200F10UL,
    Y12_2020 = 0x0F09201010UL,
    Bayer8RGGB = 0x1009200408UL,
    Bayer16RGGB = 0x1009200A10UL,
    BayerS16RGGB = 0x1009208A10UL,
    X2Bayer14RGGB = 0x1009210B10UL,
    X4Bayer12RGGB = 0x1009210C10UL,
    X6Bayer10RGGB = 0x1009210D10UL,
    Bayer8BGGR = 0x1109200408UL,
    Bayer16BGGR = 0x1109200A10UL,
    BayerS16BGGR = 0x1109208A10UL,
    X2Bayer14BGGR = 0x1109210B10UL,
    X4Bayer12BGGR = 0x1109210C10UL,
    X6Bayer10BGGR = 0x1109210D10UL,
    Bayer8GRBG = 0x1209200408UL,
    Bayer16GRBG = 0x1209200A10UL,
    BayerS16GRBG = 0x1209208A10UL,
    X2Bayer14GRBG = 0x1209210B10UL,
    X4Bayer12GRBG = 0x1209210C10UL,
    X6Bayer10GRBG = 0x1209210D10UL,
    Bayer8GBRG = 0x1309200408UL,
    Bayer16GBRG = 0x1309200A10UL,
    BayerS16GBRG = 0x1309208A10UL,
    X2Bayer14GBRG = 0x1309210B10UL,
    X4Bayer12GBRG = 0x1309210C10UL,
    X6Bayer10GBRG = 0x1309210D10UL,
    XYZ = 0x140A886640UL,
};

enum ColorSurfaceFormat : u32 {
    Bitmap = 0x1C,
    Unknown1D = 0x1D,
    RGBA32Float = 0xC0,
    RGBA32Sint = 0xC1,
    RGBA32Uint = 0xC2,
    RGBX32Float = 0xC3,
    RGBX32Sint = 0xC4,
    RGBX32Uint = 0xC5,
    RGBA16Unorm = 0xC6,
    RGBA16Snorm = 0xC7,
    RGBA16Sint = 0xC8,
    RGBA16Uint = 0xC9,
    RGBA16Float = 0xCA,
    RG32Float = 0xCB,
    RG32Sint = 0xCC,
    RG32Uint = 0xCD,
    RGBX16Float = 0xCE,
    BGRA8Unorm = 0xCF,
    BGRA8Srgb = 0xD0,
    RGB10A2Unorm = 0xD1,
    RGB10A2Uint = 0xD2,
    RGBA8Unorm = 0xD5,
    RGBA8Srgb = 0xD6,
    RGBA8Snorm = 0xD7,
    RGBA8Sint = 0xD8,
    RGBA8Uint = 0xD9,
    RG16Unorm = 0xDA,
    RG16Snorm = 0xDB,
    RG16Sint = 0xDC,
    RG16Uint = 0xDD,
    RG16Float = 0xDE,
    BGR10A2Unorm = 0xDF,
    R11G11B10Float = 0xE0,
    R32Sint = 0xE3,
    R32Uint = 0xE4,
    R32Float = 0xE5,
    BGRX8Unorm = 0xE6,
    BGRX8Srgb = 0xE7,
    B5G6R5Unorm = 0xE8,
    BGR5A1Unorm = 0xE9,
    RG8Unorm = 0xEA,
    RG8Snorm = 0xEB,
    RG8Sint = 0xEC,
    RG8Uint = 0xED,
    R16Unorm = 0xEE,
    R16Snorm = 0xEF,
    R16Sint = 0xF0,
    R16Uint = 0xF1,
    R16Float = 0xF2,
    R8Unorm = 0xF3,
    R8Snorm = 0xF4,
    R8Sint = 0xF5,
    R8Uint = 0xF6,
    A8Unorm = 0xF7,
    BGR5X1Unorm = 0xF8,
    RGBX8Unorm = 0xF9,
    RGBX8Srgb = 0xFA,
    BGR5X1UnormUnknownFB = 0xFB,
    BGR5X1UnormUnknownFC = 0xFC,
    BGRX8UnormUnknownFD = 0xFD,
    BGRX8UnormUnknownFE = 0xFE,
    Y32UintUnknownFF = 0xFF,
};

ColorSurfaceFormat to_color_surface_format(NvColorFormat color_format);

struct NvSurface {
    u32 width;
    u32 height;
    NvColorFormat color_format;
    NvLayout layout;
    u32 pitch;
    u32 unused; // usually this field contains the nvmap handle, but it's
                // completely unused/overwritten during marshalling
    u32 offset;
    NvKind kind;
    u32 block_height_log2;
    NvDisplayScanFormat scan;
    u32 second_field_offset;
    u64 flags;
    u64 size;
    u32 unk[6]; // compression related
} __attribute__((packed));

struct NvGraphicsBuffer {
    i32 unk0;       // -1
    i32 nvmap_id;   // nvmap object id
    u32 unk2;       // 0
    u32 magic;      // 0xDAFFCAFF
    u32 pid;        // 42
    u32 type;       // ?
    u32 usage;      // GRALLOC_USAGE_* bitmask
    u32 format;     // PIXEL_FORMAT_*
    u32 ext_format; // copy of the above (in most cases)
    u32 stride;     // in pixels!
    u32 total_size; // in bytes
    u32 num_planes; // usually 1
    u32 unk12;      // 0
    NvSurface planes[3];
    u64 unused; // official sw writes a pointer to bookkeeping data here, but
                // it's otherwise completely unused/overwritten during
                // marshalling
} __attribute__((packed));

struct TextureDescriptor {
    uptr ptr;
    ColorSurfaceFormat color_surface_format;
    NvKind kind;
    usize width;
    usize height;
    usize stride;
    usize block_height_log2;
    usize pitch;
    // TODO: more
};

struct Fence {
    u32 id;
    u32 value;
};

enum class GpfifoFlags : u32 {
    None = 0,
    FenceWait = BIT(0),
    FenceGet = BIT(1),
    HwFormat = BIT(2),
    SyncFence = BIT(3),
    SuppressWfi = BIT(4),
    SkipBufferRefcounting = BIT(5),
};

ENABLE_ENUM_BITMASK_OPERATORS(GpfifoFlags);

struct GpfifoEntry {
    uptr gpu_addr : 40;
    bool allow_flush : 1;
    bool is_push_buffer : 1;
    usize size : 18; // TODO: is the bit count correct?
    bool sync : 1;
};

constexpr u32 MACRO_METHODS_REGION = 0xe00;

constexpr usize COLOR_TARGET_COUNT = 8;
constexpr usize VIEWPORT_COUNT = 16;

} // namespace Hydra::HW::TegraX1::GPU

ENABLE_ENUM_FORMATTING(Hydra::HW::TegraX1::GPU::NvLayout, Pitch, "pitch", Tiled,
                       "tiled", BlockLinear, "block linear")

ENABLE_ENUM_FORMATTING(Hydra::HW::TegraX1::GPU::NvDisplayScanFormat,
                       Progressive, "progressive", Interlaced, "interlaced")

ENABLE_ENUM_FORMATTING(
    Hydra::HW::TegraX1::GPU::NvKind, Pitch, "pitch", Z16, "z16", Z16_2C,
    "z16_2c", Z16_MS2_2C, "z16_ms2_2c", Z16_MS4_2C, "z16_ms4_2c", Z16_MS8_2C,
    "z16_ms8_2c", Z16_MS16_2C, "z16_ms16_2c", Z16_2Z, "z16_2z", Z16_MS2_2Z,
    "z16_ms2_2z", Z16_MS4_2Z, "z16_ms4_2z", Z16_MS8_2Z, "z16_ms8_2z",
    Z16_MS16_2Z, "z16_ms16_2z", Z16_4CZ, "z16_4cz", Z16_MS2_4CZ, "z16_ms2_4cz",
    Z16_MS4_4CZ, "z16_ms4_4cz", Z16_MS8_4CZ, "z16_ms8_4cz", Z16_MS16_4CZ,
    "z16_ms16_4cz", S8Z24, "s8z24", S8Z24_1Z, "s8z24_1z", S8Z24_MS2_1Z,
    "s8z24_ms2_1z", S8Z24_MS4_1Z, "s8z24_ms4_1z", S8Z24_MS8_1Z, "s8z24_ms8_1z",
    S8Z24_MS16_1Z, "s8z24_ms16_1z", S8Z24_2CZ, "s8z24_2cz", S8Z24_MS2_2CZ,
    "s8z24_ms2_2cz", S8Z24_MS4_2CZ, "s8z24_ms4_2cz", S8Z24_MS8_2CZ,
    "s8z24_ms8_2cz", S8Z24_MS16_2CZ, "s8z24_ms16_2cz", S8Z24_2CS, "s8z24_2cs",
    S8Z24_MS2_2CS, "s8z24_ms2_2cs", S8Z24_MS4_2CS, "s8z24_ms4_2cs",
    S8Z24_MS8_2CS, "s8z24_ms8_2cs", S8Z24_MS16_2CS, "s8z24_ms16_2cs",
    S8Z24_4CSZV, "s8z24_4cszv", S8Z24_MS2_4CSZV, "s8z24_ms2_4cszv",
    S8Z24_MS4_4CSZV, "s8z24_ms4_4cszv", S8Z24_MS8_4CSZV, "s8z24_ms8_4cszv",
    S8Z24_MS16_4CSZV, "s8z24_ms16_4cszv", V8Z24_MS4_VC12, "v8z24_ms4_vc12",
    V8Z24_MS4_VC4, "v8z24_ms4_vc4", V8Z24_MS8_VC8, "v8z24_ms8_vc8",
    V8Z24_MS8_VC24, "v8z24_ms8_vc24", S8, "s8", S8_2S, "s8_2s",
    V8Z24_MS4_VC12_1ZV, "v8z24_ms4_vc12_1zv", V8Z24_MS4_VC4_1ZV,
    "v8z24_ms4_vc4_1zv", V8Z24_MS8_VC8_1ZV, "v8z24_ms8_vc8_1zv",
    V8Z24_MS8_VC24_1ZV, "v8z24_ms8_vc24_1zv", V8Z24_MS4_VC12_2CS,
    "v8z24_ms4_vc12_2cs", V8Z24_MS4_VC4_2CS, "v8z24_ms4_vc4_2cs",
    V8Z24_MS8_VC8_2CS, "v8z24_ms8_vc8_2cs", V8Z24_MS8_VC24_2CS,
    "v8z24_ms8_vc24_2cs", V8Z24_MS4_VC12_2CZV, "v8z24_ms4_vc12_2czv",
    V8Z24_MS4_VC4_2CZV, "v8z24_ms4_vc4_2czv", V8Z24_MS8_VC8_2CZV,
    "v8z24_ms8_vc8_2czv", V8Z24_MS8_VC24_2CZV, "v8z24_ms8_vc24_2czv",
    V8Z24_MS4_VC12_2ZV, "v8z24_ms4_vc12_2zv", V8Z24_MS4_VC4_2ZV,
    "v8z24_ms4_vc4_2zv", V8Z24_MS8_VC8_2ZV, "v8z24_ms8_vc8_2zv",
    V8Z24_MS8_VC24_2ZV, "v8z24_ms8_vc24_2zv", V8Z24_MS4_VC12_4CSZV,
    "v8z24_ms4_vc12_4cszv", V8Z24_MS4_VC4_4CSZV, "v8z24_ms4_vc4_4cszv",
    V8Z24_MS8_VC8_4CSZV, "v8z24_ms8_vc8_4cszv", V8Z24_MS8_VC24_4CSZV,
    "v8z24_ms8_vc24_4cszv", Z24S8, "z24s8", Z24S8_1Z, "z24s8_1z", Z24S8_MS2_1Z,
    "z24s8_ms2_1z", Z24S8_MS4_1Z, "z24s8_ms4_1z", Z24S8_MS8_1Z, "z24s8_ms8_1z",
    Z24S8_MS16_1Z, "z24s8_ms16_1z", Z24S8_2CS, "z24s8_2cs", Z24S8_MS2_2CS,
    "z24s8_ms2_2cs", Z24S8_MS4_2CS, "z24s8_ms4_2cs", Z24S8_MS8_2CS,
    "z24s8_ms8_2cs", Z24S8_MS16_2CS, "z24s8_ms16_2cs", Z24S8_2CZ, "z24s8_2cz",
    Z24S8_MS2_2CZ, "z24s8_ms2_2cz", Z24S8_MS4_2CZ, "z24s8_ms4_2cz",
    Z24S8_MS8_2CZ, "z24s8_ms8_2cz", Z24S8_MS16_2CZ, "z24s8_ms16_2cz",
    Z24S8_4CSZV, "z24s8_4cszv", Z24S8_MS2_4CSZV, "z24s8_ms2_4cszv",
    Z24S8_MS4_4CSZV, "z24s8_ms4_4cszv", Z24S8_MS8_4CSZV, "z24s8_ms8_4cszv",
    Z24S8_MS16_4CSZV, "z24s8_ms16_4cszv", Z24V8_MS4_VC12, "z24v8_ms4_vc12",
    Z24V8_MS4_VC4, "z24v8_ms4_vc4", Z24V8_MS8_VC8, "z24v8_ms8_vc8",
    Z24V8_MS8_VC24, "z24v8_ms8_vc24", Z24V8_MS4_VC12_1ZV, "z24v8_ms4_vc12_1zv",
    Z24V8_MS4_VC4_1ZV, "z24v8_ms4_vc4_1zv", Z24V8_MS8_VC8_1ZV,
    "z24v8_ms8_vc8_1zv", Z24V8_MS8_VC24_1ZV, "z24v8_ms8_vc24_1zv",
    Z24V8_MS4_VC12_2CS, "z24v8_ms4_vc12_2cs", Z24V8_MS4_VC4_2CS,
    "z24v8_ms4_vc4_2cs", Z24V8_MS8_VC8_2CS, "z24v8_ms8_vc8_2cs",
    Z24V8_MS8_VC24_2CS, "z24v8_ms8_vc24_2cs", Z24V8_MS4_VC12_2CZV,
    "z24v8_ms4_vc12_2czv", Z24V8_MS4_VC4_2CZV, "z24v8_ms4_vc4_2czv",
    Z24V8_MS8_VC8_2CZV, "z24v8_ms8_vc8_2czv", Z24V8_MS8_VC24_2CZV,
    "z24v8_ms8_vc24_2czv", Z24V8_MS4_VC12_2ZV, "z24v8_ms4_vc12_2zv",
    Z24V8_MS4_VC4_2ZV, "z24v8_ms4_vc4_2zv", Z24V8_MS8_VC8_2ZV,
    "z24v8_ms8_vc8_2zv", Z24V8_MS8_VC24_2ZV, "z24v8_ms8_vc24_2zv",
    Z24V8_MS4_VC12_4CSZV, "z24v8_ms4_vc12_4cszv", Z24V8_MS4_VC4_4CSZV,
    "z24v8_ms4_vc4_4cszv", Z24V8_MS8_VC8_4CSZV, "z24v8_ms8_vc8_4cszv",
    Z24V8_MS8_VC24_4CSZV, "z24v8_ms8_vc24_4cszv", ZF32, "zf32", ZF32_1Z,
    "zf32_1z", ZF32_MS2_1Z, "zf32_ms2_1z", ZF32_MS4_1Z, "zf32_ms4_1z",
    ZF32_MS8_1Z, "zf32_ms8_1z", ZF32_MS16_1Z, "zf32_ms16_1z", ZF32_2CS,
    "zf32_2cs", ZF32_MS2_2CS, "zf32_ms2_2cs", ZF32_MS4_2CS, "zf32_ms4_2cs",
    ZF32_MS8_2CS, "zf32_ms8_2cs", ZF32_MS16_2CS, "zf32_ms16_2cs", ZF32_2CZ,
    "zf32_2cz", ZF32_MS2_2CZ, "zf32_ms2_2cz", ZF32_MS4_2CZ, "zf32_ms4_2cz",
    ZF32_MS8_2CZ, "zf32_ms8_2cz", ZF32_MS16_2CZ, "zf32_ms16_2cz",
    X8Z24_X16V8S8_MS4_VC12, "x8z24_x16v8s8_ms4_vc12", X8Z24_X16V8S8_MS4_VC4,
    "x8z24_x16v8s8_ms4_vc4", X8Z24_X16V8S8_MS8_VC8, "x8z24_x16v8s8_ms8_vc8",
    X8Z24_X16V8S8_MS8_VC24, "x8z24_x16v8s8_ms8_vc24",
    X8Z24_X16V8S8_MS4_VC12_1CS, "x8z24_x16v8s8_ms4_vc12_1cs",
    X8Z24_X16V8S8_MS4_VC4_1CS, "x8z24_x16v8s8_ms4_vc4_1cs",
    X8Z24_X16V8S8_MS8_VC8_1CS, "x8z24_x16v8s8_ms8_vc8_1cs",
    X8Z24_X16V8S8_MS8_VC24_1CS, "x8z24_x16v8s8_ms8_vc24_1cs",
    X8Z24_X16V8S8_MS4_VC12_1ZV, "x8z24_x16v8s8_ms4_vc12_1zv",
    X8Z24_X16V8S8_MS4_VC4_1ZV, "x8z24_x16v8s8_ms4_vc4_1zv",
    X8Z24_X16V8S8_MS8_VC8_1ZV, "x8z24_x16v8s8_ms8_vc8_1zv",
    X8Z24_X16V8S8_MS8_VC24_1ZV, "x8z24_x16v8s8_ms8_vc24_1zv",
    X8Z24_X16V8S8_MS4_VC12_1CZV, "x8z24_x16v8s8_ms4_vc12_1czv",
    X8Z24_X16V8S8_MS4_VC4_1CZV, "x8z24_x16v8s8_ms4_vc4_1czv",
    X8Z24_X16V8S8_MS8_VC8_1CZV, "x8z24_x16v8s8_ms8_vc8_1czv",
    X8Z24_X16V8S8_MS8_VC24_1CZV, "x8z24_x16v8s8_ms8_vc24_1czv",
    X8Z24_X16V8S8_MS4_VC12_2CS, "x8z24_x16v8s8_ms4_vc12_2cs",
    X8Z24_X16V8S8_MS4_VC4_2CS, "x8z24_x16v8s8_ms4_vc4_2cs",
    X8Z24_X16V8S8_MS8_VC8_2CS, "x8z24_x16v8s8_ms8_vc8_2cs",
    X8Z24_X16V8S8_MS8_VC24_2CS, "x8z24_x16v8s8_ms8_vc24_2cs",
    X8Z24_X16V8S8_MS4_VC12_2CSZV, "x8z24_x16v8s8_ms4_vc12_2cszv",
    X8Z24_X16V8S8_MS4_VC4_2CSZV, "x8z24_x16v8s8_ms4_vc4_2cszv",
    X8Z24_X16V8S8_MS8_VC8_2CSZV, "x8z24_x16v8s8_ms8_vc8_2cszv",
    X8Z24_X16V8S8_MS8_VC24_2CSZV, "x8z24_x16v8s8_ms8_vc24_2cszv",
    ZF32_X16V8S8_MS4_VC12, "zf32_x16v8s8_ms4_vc12", ZF32_X16V8S8_MS4_VC4,
    "zf32_x16v8s8_ms4_vc4", ZF32_X16V8S8_MS8_VC8, "zf32_x16v8s8_ms8_vc8",
    ZF32_X16V8S8_MS8_VC24, "zf32_x16v8s8_ms8_vc24", ZF32_X16V8S8_MS4_VC12_1CS,
    "zf32_x16v8s8_ms4_vc12_1cs", ZF32_X16V8S8_MS4_VC4_1CS,
    "zf32_x16v8s8_ms4_vc4_1cs", ZF32_X16V8S8_MS8_VC8_1CS,
    "zf32_x16v8s8_ms8_vc8_1cs", ZF32_X16V8S8_MS8_VC24_1CS,
    "zf32_x16v8s8_ms8_vc24_1cs", ZF32_X16V8S8_MS4_VC12_1ZV,
    "zf32_x16v8s8_ms4_vc12_1zv", ZF32_X16V8S8_MS4_VC4_1ZV,
    "zf32_x16v8s8_ms4_vc4_1zv", ZF32_X16V8S8_MS8_VC8_1ZV,
    "zf32_x16v8s8_ms8_vc8_1zv", ZF32_X16V8S8_MS8_VC24_1ZV,
    "zf32_x16v8s8_ms8_vc24_1zv", ZF32_X16V8S8_MS4_VC12_1CZV,
    "zf32_x16v8s8_ms4_vc12_1czv", ZF32_X16V8S8_MS4_VC4_1CZV,
    "zf32_x16v8s8_ms4_vc4_1czv", ZF32_X16V8S8_MS8_VC8_1CZV,
    "zf32_x16v8s8_ms8_vc8_1czv", ZF32_X16V8S8_MS8_VC24_1CZV,
    "zf32_x16v8s8_ms8_vc24_1czv", ZF32_X16V8S8_MS4_VC12_2CS,
    "zf32_x16v8s8_ms4_vc12_2cs", ZF32_X16V8S8_MS4_VC4_2CS,
    "zf32_x16v8s8_ms4_vc4_2cs", ZF32_X16V8S8_MS8_VC8_2CS,
    "zf32_x16v8s8_ms8_vc8_2cs", ZF32_X16V8S8_MS8_VC24_2CS,
    "zf32_x16v8s8_ms8_vc24_2cs", ZF32_X16V8S8_MS4_VC12_2CSZV,
    "zf32_x16v8s8_ms4_vc12_2cszv", ZF32_X16V8S8_MS4_VC4_2CSZV,
    "zf32_x16v8s8_ms4_vc4_2cszv", ZF32_X16V8S8_MS8_VC8_2CSZV,
    "zf32_x16v8s8_ms8_vc8_2cszv", ZF32_X16V8S8_MS8_VC24_2CSZV,
    "zf32_x16v8s8_ms8_vc24_2cszv", ZF32_X24S8, "zf32_x24s8", ZF32_X24S8_1CS,
    "zf32_x24s8_1cs", ZF32_X24S8_MS2_1CS, "zf32_x24s8_ms2_1cs",
    ZF32_X24S8_MS4_1CS, "zf32_x24s8_ms4_1cs", ZF32_X24S8_MS8_1CS,
    "zf32_x24s8_ms8_1cs", ZF32_X24S8_MS16_1CS, "zf32_x24s8_ms16_1cs",
    SmskedMessage, "smskedmessage", SmhostMessage, "smhostmessage",
    C64_MS2_2CRA, "c64_ms2_2cra", ZF32_X24S8_2CSZV, "zf32_x24s8_2cszv",
    ZF32_X24S8_MS2_2CSZV, "zf32_x24s8_ms2_2cszv", ZF32_X24S8_MS4_2CSZV,
    "zf32_x24s8_ms4_2cszv", ZF32_X24S8_MS8_2CSZV, "zf32_x24s8_ms8_2cszv",
    ZF32_X24S8_MS16_2CSZV, "zf32_x24s8_ms16_2cszv", ZF32_X24S8_2CS,
    "zf32_x24s8_2cs", ZF32_X24S8_MS2_2CS, "zf32_x24s8_ms2_2cs",
    ZF32_X24S8_MS4_2CS, "zf32_x24s8_ms4_2cs", ZF32_X24S8_MS8_2CS,
    "zf32_x24s8_ms8_2cs", ZF32_X24S8_MS16_2CS, "zf32_x24s8_ms16_2cs", C32_2C,
    "c32_2c", C32_2CBR, "c32_2cbr", C32_2CBA, "c32_2cba", C32_2CRA, "c32_2cra",
    C32_2BRA, "c32_2bra", C32_MS2_2C, "c32_ms2_2c", C32_MS2_2CBR,
    "c32_ms2_2cbr", C32_MS2_2CRA, "c32_ms2_2cra", C32_MS4_2C, "c32_ms4_2c",
    C32_MS4_2CBR, "c32_ms4_2cbr", C32_MS4_2CBA, "c32_ms4_2cba", C32_MS4_2CRA,
    "c32_ms4_2cra", C32_MS4_2BRA, "c32_ms4_2bra", C32_MS8_MS16_2C,
    "c32_ms8_ms16_2c", C32_MS8_MS16_2CRA, "c32_ms8_ms16_2cra", C64_2C, "c64_2c",
    C64_2CBR, "c64_2cbr", C64_2CBA, "c64_2cba", C64_2CRA, "c64_2cra", C64_2BRA,
    "c64_2bra", C64_MS2_2C, "c64_ms2_2c", C64_MS2_2CBR, "c64_ms2_2cbr",
    C64_MS4_2C, "c64_ms4_2c", C64_MS4_2CBR, "c64_ms4_2cbr", C64_MS4_2CBA,
    "c64_ms4_2cba", C64_MS4_2CRA, "c64_ms4_2cra", C64_MS4_2BRA, "c64_ms4_2bra",
    C64_MS8_MS16_2C, "c64_ms8_ms16_2c", C64_MS8_MS16_2CRA, "c64_ms8_ms16_2cra",
    C128_2C, "c128_2c", C128_2CR, "c128_2cr", C128_MS2_2C, "c128_ms2_2c",
    C128_MS2_2CR, "c128_ms2_2cr", C128_MS4_2C, "c128_ms4_2c", C128_MS4_2CR,
    "c128_ms4_2cr", C128_MS8_MS16_2C, "c128_ms8_ms16_2c", C128_MS8_MS16_2CR,
    "c128_ms8_ms16_2cr", X8C24, "x8c24", PitchNoSwizzle, "pitchnoswizzle",
    Generic_16BX2, "generic_16bx2", Invalid, "invalid")

ENABLE_ENUM_FORMATTING(
    Hydra::HW::TegraX1::GPU::NvColorFormat, NonColor8, "noncolor8", NonColor16,
    "noncolor16", NonColor24, "noncolor24", NonColor32, "noncolor32", X4C4,
    "x4c4", A4L4, "a4l4", A8L8, "a8l8", Float_A16L16, "float_a16l16", A1B5G5R5,
    "a1b5g5r5", A4B4G4R4, "a4b4g4r4", A5B5G5R1, "a5b5g5r1", A2B10G10R10,
    "a2b10g10r10", A8B8G8R8, "a8b8g8r8", A16B16G16R16, "a16b16g16r16",
    Float_A16B16G16R16, "float_a16b16g16r16", A1R5G5B5, "a1r5g5b5", A4R4G4B4,
    "a4r4g4b4", A5R1G5B5, "a5r1g5b5", A2R10G10B10, "a2r10g10b10", A8R8G8B8,
    "a8r8g8b8", A1, "a1", A2, "a2", A4, "a4", A8, "a8", A16, "a16", A32, "a32",
    Float_A16, "float_a16", L4A4, "l4a4", L8A8, "l8a8", B4G4R4A4, "b4g4r4a4",
    B5G5R1A5, "b5g5r1a5", B5G5R5A1, "b5g5r5a1", B8G8R8A8, "b8g8r8a8",
    B10G10R10A2, "b10g10r10a2", R1G5B5A5, "r1g5b5a5", R4G4B4A4, "r4g4b4a4",
    R5G5B5A1, "r5g5b5a1", R8G8B8A8, "r8g8b8a8", R10G10B10A2, "r10g10b10a2", L1,
    "l1", L2, "l2", L4, "l4", L8, "l8", L16, "l16", L32, "l32", Float_L16,
    "float_l16", B5G6R5, "b5g6r5", B6G5R5, "b6g5r5", B5G5R5X1, "b5g5r5x1",
    B8G8R8, "b8g8r8", B8G8R8X8, "b8g8r8x8", Float_B10G11R11, "float_b10g11r11",
    X1B5G5R5, "x1b5g5r5", X8B8G8R8, "x8b8g8r8", X16B16G16R16, "x16b16g16r16",
    Float_X16B16G16R16, "float_x16b16g16r16", R3G3B2, "r3g3b2", R5G5B6,
    "r5g5b6", R5G6B5, "r5g6b5", R5G5B5X1, "r5g5b5x1", R8G8B8, "r8g8b8",
    R8G8B8X8, "r8g8b8x8", X1R5G5B5, "x1r5g5b5", X8R8G8B8, "x8r8g8b8", R8G8,
    "r8g8", R16G16, "r16g16", Float_R16G16, "float_r16g16", R8, "r8", R16,
    "r16", Float_R16, "float_r16", A2B10G10R10_sRGB, "a2b10g10r10_srgb",
    A8B8G8R8_sRGB, "a8b8g8r8_srgb", A16B16G16R16_sRGB, "a16b16g16r16_srgb",
    A2R10G10B10_sRGB, "a2r10g10b10_srgb", B10G10R10A2_sRGB, "b10g10r10a2_srgb",
    R10G10B10A2_sRGB, "r10g10b10a2_srgb", X8B8G8R8_sRGB, "x8b8g8r8_srgb",
    X16B16G16R16_sRGB, "x16b16g16r16_srgb", A2B10G10R10_709, "a2b10g10r10_709",
    A8B8G8R8_709, "a8b8g8r8_709", A16B16G16R16_709, "a16b16g16r16_709",
    A2R10G10B10_709, "a2r10g10b10_709", B10G10R10A2_709, "b10g10r10a2_709",
    R10G10B10A2_709, "r10g10b10a2_709", X8B8G8R8_709, "x8b8g8r8_709",
    X16B16G16R16_709, "x16b16g16r16_709", A2B10G10R10_709_Linear,
    "a2b10g10r10_709_linear", A8B8G8R8_709_Linear, "a8b8g8r8_709_linear",
    A16B16G16R16_709_Linear, "a16b16g16r16_709_linear", A2R10G10B10_709_Linear,
    "a2r10g10b10_709_linear", B10G10R10A2_709_Linear, "b10g10r10a2_709_linear",
    R10G10B10A2_709_Linear, "r10g10b10a2_709_linear", X8B8G8R8_709_Linear,
    "x8b8g8r8_709_linear", X16B16G16R16_709_Linear, "x16b16g16r16_709_linear",
    Float_A16B16G16R16_scRGB_Linear, "float_a16b16g16r16_scrgb_linear",
    A2B10G10R10_2020, "a2b10g10r10_2020", A8B8G8R8_2020, "a8b8g8r8_2020",
    A16B16G16R16_2020, "a16b16g16r16_2020", A2R10G10B10_2020,
    "a2r10g10b10_2020", B10G10R10A2_2020, "b10g10r10a2_2020", R10G10B10A2_2020,
    "r10g10b10a2_2020", X8B8G8R8_2020, "x8b8g8r8_2020", X16B16G16R16_2020,
    "x16b16g16r16_2020", A2B10G10R10_2020_Linear, "a2b10g10r10_2020_linear",
    A8B8G8R8_2020_Linear, "a8b8g8r8_2020_linear", A16B16G16R16_2020_Linear,
    "a16b16g16r16_2020_linear", Float_A16B16G16R16_2020_Linear,
    "float_a16b16g16r16_2020_linear", A2R10G10B10_2020_Linear,
    "a2r10g10b10_2020_linear", B10G10R10A2_2020_Linear,
    "b10g10r10a2_2020_linear", R10G10B10A2_2020_Linear,
    "r10g10b10a2_2020_linear", X8B8G8R8_2020_Linear, "x8b8g8r8_2020_linear",
    X16B16G16R16_2020_Linear, "x16b16g16r16_2020_linear",
    Float_A16B16G16R16_2020_PQ, "float_a16b16g16r16_2020_pq", A4I4, "a4i4",
    A8I8, "a8i8", I4A4, "i4a4", I8A8, "i8a8", I1, "i1", I2, "i2", I4, "i4", I8,
    "i8", A8Y8U8V8, "a8y8u8v8", A16Y16U16V16, "a16y16u16v16", Y8U8V8A8,
    "y8u8v8a8", V8_U8, "v8_u8", V8U8, "v8u8", V10U10, "v10u10", V12U12,
    "v12u12", V8, "v8", V10, "v10", V12, "v12", U8_V8, "u8_v8", U8V8, "u8v8",
    U10V10, "u10v10", U12V12, "u12v12", U8, "u8", U10, "u10", U12, "u12", Y8,
    "y8", Y10, "y10", Y12, "y12", YVYU, "yvyu", VYUY, "vyuy", YUYV, "yuyv",
    UYVY, "uyvy", Y8_U8_V8, "y8_u8_v8", V8_U8_RR, "v8_u8_rr", V8U8_RR,
    "v8u8_rr", V8_RR, "v8_rr", U8_V8_RR, "u8_v8_rr", U8V8_RR, "u8v8_rr", U8_RR,
    "u8_rr", Y8_RR, "y8_rr", V8_U8_ER, "v8_u8_er", V8U8_ER, "v8u8_er", V8_ER,
    "v8_er", U8_V8_ER, "u8_v8_er", U8V8_ER, "u8v8_er", U8_ER, "u8_er", Y8_ER,
    "y8_er", V8_U8_709, "v8_u8_709", V8U8_709, "v8u8_709", V10U10_709,
    "v10u10_709", V12U12_709, "v12u12_709", V8_709, "v8_709", V10_709,
    "v10_709", V12_709, "v12_709", U8_V8_709, "u8_v8_709", U8V8_709, "u8v8_709",
    U10V10_709, "u10v10_709", U12V12_709, "u12v12_709", U8_709, "u8_709",
    U10_709, "u10_709", U12_709, "u12_709", Y8_709, "y8_709", Y10_709,
    "y10_709", Y12_709, "y12_709", V8_U8_709_ER, "v8_u8_709_er", V8U8_709_ER,
    "v8u8_709_er", V10U10_709_ER, "v10u10_709_er", V12U12_709_ER,
    "v12u12_709_er", V8_709_ER, "v8_709_er", V10_709_ER, "v10_709_er",
    V12_709_ER, "v12_709_er", U8_V8_709_ER, "u8_v8_709_er", U8V8_709_ER,
    "u8v8_709_er", U10V10_709_ER, "u10v10_709_er", U12V12_709_ER,
    "u12v12_709_er", U8_709_ER, "u8_709_er", U10_709_ER, "u10_709_er",
    U12_709_ER, "u12_709_er", Y8_709_ER, "y8_709_er", Y10_709_ER, "y10_709_er",
    Y12_709_ER, "y12_709_er", V10U10_2020, "v10u10_2020", V12U12_2020,
    "v12u12_2020", V10_2020, "v10_2020", V12_2020, "v12_2020", U10V10_2020,
    "u10v10_2020", U12V12_2020, "u12v12_2020", U10_2020, "u10_2020", U12_2020,
    "u12_2020", Y10_2020, "y10_2020", Y12_2020, "y12_2020", Bayer8RGGB,
    "bayer8rggb", Bayer16RGGB, "bayer16rggb", BayerS16RGGB, "bayers16rggb",
    X2Bayer14RGGB, "x2bayer14rggb", X4Bayer12RGGB, "x4bayer12rggb",
    X6Bayer10RGGB, "x6bayer10rggb", Bayer8BGGR, "bayer8bggr", Bayer16BGGR,
    "bayer16bggr", BayerS16BGGR, "bayers16bggr", X2Bayer14BGGR, "x2bayer14bggr",
    X4Bayer12BGGR, "x4bayer12bggr", X6Bayer10BGGR, "x6bayer10bggr", Bayer8GRBG,
    "bayer8grbg", Bayer16GRBG, "bayer16grbg", BayerS16GRBG, "bayers16grbg",
    X2Bayer14GRBG, "x2bayer14grbg", X4Bayer12GRBG, "x4bayer12grbg",
    X6Bayer10GRBG, "x6bayer10grbg", Bayer8GBRG, "bayer8gbrg", Bayer16GBRG,
    "bayer16gbrg", BayerS16GBRG, "bayers16gbrg", X2Bayer14GBRG, "x2bayer14gbrg",
    X4Bayer12GBRG, "x4bayer12gbrg", X6Bayer10GBRG, "x6bayer10gbrg", XYZ, "xyz")

ENABLE_ENUM_FORMATTING(
    Hydra::HW::TegraX1::GPU::ColorSurfaceFormat, Bitmap, "bitmap", Unknown1D,
    "unknown1d", RGBA32Float, "rgba32float", RGBA32Sint, "rgba32sint",
    RGBA32Uint, "rgba32uint", RGBX32Float, "rgbx32float", RGBX32Sint,
    "rgbx32sint", RGBX32Uint, "rgbx32uint", RGBA16Unorm, "rgba16unorm",
    RGBA16Snorm, "rgba16snorm", RGBA16Sint, "rgba16sint", RGBA16Uint,
    "rgba16uint", RGBA16Float, "rgba16float", RG32Float, "rg32float", RG32Sint,
    "rg32sint", RG32Uint, "rg32uint", RGBX16Float, "rgbx16float", BGRA8Unorm,
    "bgra8unorm", BGRA8Srgb, "bgra8srgb", RGB10A2Unorm, "rgb10a2unorm",
    RGB10A2Uint, "rgb10a2uint", RGBA8Unorm, "rgba8unorm", RGBA8Srgb,
    "rgba8srgb", RGBA8Snorm, "rgba8snorm", RGBA8Sint, "rgba8sint", RGBA8Uint,
    "rgba8uint", RG16Unorm, "rg16unorm", RG16Snorm, "rg16snorm", RG16Sint,
    "rg16sint", RG16Uint, "rg16uint", RG16Float, "rg16float", BGR10A2Unorm,
    "bgr10a2unorm", R11G11B10Float, "r11g11b10float", R32Sint, "r32sint",
    R32Uint, "r32uint", R32Float, "r32float", BGRX8Unorm, "bgrx8unorm",
    BGRX8Srgb, "bgrx8srgb", B5G6R5Unorm, "b5g6r5unorm", BGR5A1Unorm,
    "bgr5a1unorm", RG8Unorm, "rg8unorm", RG8Snorm, "rg8snorm", RG8Sint,
    "rg8sint", RG8Uint, "rg8uint", R16Unorm, "r16unorm", R16Snorm, "r16snorm",
    R16Sint, "r16sint", R16Uint, "r16uint", R16Float, "r16float", R8Unorm,
    "r8unorm", R8Snorm, "r8snorm", R8Sint, "r8sint", R8Uint, "r8uint", A8Unorm,
    "a8unorm", BGR5X1Unorm, "bgr5x1unorm", RGBX8Unorm, "rgbx8unorm", RGBX8Srgb,
    "rgbx8srgb", BGR5X1UnormUnknownFB, "bgr5x1unormunknownfb",
    BGR5X1UnormUnknownFC, "bgr5x1unormunknownfc", BGRX8UnormUnknownFD,
    "bgrx8unormunknownfd", BGRX8UnormUnknownFE, "bgrx8unormunknownfe",
    Y32UintUnknownFF, "y32uintunknownff")

ENABLE_ENUM_FLAGS_FORMATTING(Hydra::HW::TegraX1::GPU::GpfifoFlags, FenceWait,
                             "fence wait", FenceGet, "fence get", HwFormat,
                             "hw format", SyncFence, "sync fence", SuppressWfi,
                             "suppress WFI", SkipBufferRefcounting,
                             "skip buffer refcounting")
