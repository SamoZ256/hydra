#pragma once

#include "hw/tegra_x1/gpu/const.hpp"
#include "hw/tegra_x1/gpu/engines/engine_base.hpp"

namespace Hydra::HW::TegraX1::GPU::Engines {

union Regs3D {
    struct {
        u32 padding[0x200];

        // Render targets
        struct {
            uptr addr;
            u32 width;
            u32 height;
            u32 format; // TODO: enum class as type
            struct {
                u32 width : 4;
                u32 height : 4;
                u32 depth : 4;
                bool is_linear : 1;
                u32 unused : 3;
                bool is_3d : 1;
            } tile_mode;
            struct {
                u16 layers;
                bool volume : 1; // TODO: what is this?
            } array_mode;
            u32 array_pitch;
            u32 base_layer;
            u32 mark; // TODO: what is this?
            u32 padding[6];
        } color_targets[COLOR_TARGET_COUNT];

        // Viewport transforms
        struct {
            float scale_x;
            float scale_y;
            float scale_z;
            float offset_x;
            float offset_y;
            float offset_z;
            struct {
                u8 x : 3;
                u8 y : 3;
                u8 z : 3;
                u8 w : 3;
                u32 unused : 20;
            } coord_swizzle;
            struct {
                u8 x : 5;
                u32 unused1 : 3;
                u8 y : 5;
                u32 unused2 : 19;
            } subpixel_precision_bias;
        } viewport_transforms[VIEWPORT_COUNT];

        // Viewports
        struct {
            struct {
                u16 x;
                u16 width;
            } horizontal;
            struct {
                u16 y;
                u16 height;
            } vertical;
            float near;
            float far;
        } viewports[VIEWPORT_COUNT];
    };

    u32 raw[MACRO_METHODS_REGION];
};

class ThreeD : public EngineBase {
  public:
    void Method(u32 method, u32 arg) override;

  protected:
    void WriteReg(u32 reg, u32 value) override {
        LOG_DEBUG(GPU, "Writing to 3d reg 0x{:08x}", reg);
        regs.raw[reg] = value;
    }

    void Macro(u32 method, u32 arg) override;

  private:
    Regs3D regs;

    // Macros
    u32 macro_instruction_ram[0x1000] = {0}; // TODO: what should be the size?
    u32 macro_instruction_ram_ptr;
    u32 macro_start_address_ram[0x100] = {0}; // TODO: what should be the size?
    u32 macro_start_address_ram_ptr;

    // Commands
    void LoadMmeInstructionRamPointer(u32 ptr);
    void LoadMmeInstructionRam(u32 data);
    void LoadMmeStartAddressRamPointer(u32 ptr);
    void LoadMmeStartAddressRam(u32 data);
};

} // namespace Hydra::HW::TegraX1::GPU::Engines
