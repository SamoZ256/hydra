#pragma once

#include "hw/tegra_x1/gpu/engines/engine_base.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer {
class TextureBase;
}

namespace Hydra::HW::TegraX1::GPU::Engines {

struct Texture2DInfo {
    ColorSurfaceFormat format;
    MemoryLayout layout;
    BlockDim block_dim;
    u32 depth;
    u32 layer; // InvalidateTextureDataCache in case of src
    u32 stride;
    u32 width;
    u32 height;
    Iova addr;
};

struct SplitFloat {
    u32 fractional;
    u32 integer;
};

struct PixelsFromMemory {
    u32 dst_x0;
    u32 dst_y0;
    u32 dst_width;
    u32 dst_height;
    SplitFloat dudx;
    SplitFloat dvdy;
    SplitFloat src_x0;
    SplitFloat src_y0;
};

union Regs2D {
    struct {
        u32 padding_0x0[0x80];

        // 0x80
        Texture2DInfo dst;

        u32 padding_0x8a[0x2];

        // 0x8c
        Texture2DInfo src;

        u32 padding_0x96[0x196];

        // 0x22c
        PixelsFromMemory pixels_from_memory;
    };
    u32 raw[0xe00];
};

class TwoD : public EngineBase {
  public:
    void Method(u32 method, u32 arg) override;

  protected:
    void WriteReg(u32 reg, u32 value) override {
        LOG_DEBUG(Engines, "Writing to 2d reg 0x{:08x} (value: 0x{:08x})", reg,
                  value);
        regs.raw[reg] = value;
    }

  private:
    Regs2D regs{};

    // Commands
    void Copy(const u32 index, const u32 pixels_from_memory_src_y0_int);

    // Helpers
    static Renderer::TextureBase* GetTexture(const Texture2DInfo& info);
};

} // namespace Hydra::HW::TegraX1::GPU::Engines
