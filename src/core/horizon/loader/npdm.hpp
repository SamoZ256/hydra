#pragma once

namespace hydra::horizon::loader {

enum class NpdmFlags : u8 {
    None = 0,
    Is64BitInstruction = ZTD_BIT(0),
    AddressSpace32Bit = 0x0 << 1,
    AddressSpace64BitOld = 0x1 << 1,
    AddressSpace32BitNoReserved = 0x2 << 1,
    AddressSpace64Bit = 0x3 << 1,
    OptimizeMemoryAllocation = ZTD_BIT(4),       // 7.0.0+
    DisableDeviceAddressSpaceMerge = ZTD_BIT(5), // 11.0.0+
    EnableAliasRegionExtraSize = ZTD_BIT(6),     // 18.0.0+
    PreventCodeReads = ZTD_BIT(7),               // 19.0.0+
};
ZTD_ENABLE_ENUM_BITWISE_OPERATORS(NpdmFlags)

struct NpdmMeta {
    u32 magic;
    u32 signature_key_generation; // 9.0.0+
    u32 _reserved_x8;
    NpdmFlags flags;
    u8 _reserved_xd;
    u8 main_thread_priority;
    u8 main_thread_core_number;
    u32 _reserved_x10;
    u32 system_resource_size; // 3.0.0+
    u32 version;
    u32 main_thread_stack_size;
    char name[0x10];
    u8 product_code[0x10];
    u8 _reserved_x40[0x30];
    u32 aci_offset;
    u32 aci_size;
    u32 acid_offset;
    u32 acid_size;
};

} // namespace hydra::horizon::loader
