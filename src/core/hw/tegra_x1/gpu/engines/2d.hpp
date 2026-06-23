#pragma once

#include "core/hw/tegra_x1/gpu/engines/engine_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu {
class Gpu;
}

namespace hydra::hw::tegra_x1::gpu::renderer {
class ITextureView;
}

namespace hydra::hw::tegra_x1::gpu::engines {

struct Texture2DInfo {
    ColorSurfaceFormat format;
    MemoryLayout layout;
    struct {
        u32 : 4;
        u32 block_height_gobs_log2 : 3;
        u32 : 1;
        u32 block_depth_gobs_log2 : 3;
        u32 : 21;
    };
    u32 depth;
    u32 layer;
    u32 stride;
    u32 width;
    u32 height;
    Iova addr;
};

struct FixedPoint {
    u32 fractional;
    u32 integer;

    explicit operator f64() const {
        return static_cast<f64>(integer) + (static_cast<f64>(fractional) / static_cast<f64>(1llu << 32llu));
    }
};

// TODO: operate directly on FixedPoint instead of casting to f64?
/*
inline FixedPoint operator*(const FixedPoint& a, const FixedPoint& b) {
    return FixedPoint{a.fractional * b.fractional, a.integer * b.integer};
}

inline FixedPoint operator/(const FixedPoint& a, const FixedPoint& b) {
    return FixedPoint{a.fractional / b.fractional, a.integer / b.integer};
}
*/

struct PixelsFromMemory {
    u32 dst_x0;
    u32 dst_y0;
    u32 dst_width;
    u32 dst_height;
    FixedPoint dudx;
    FixedPoint dvdy;
    FixedPoint src_x0;
    FixedPoint src_y0;
};

struct Regs2D {
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

class TwoD : public EngineWithRegsBase<Regs2D> {
  public:
    TwoD(Gpu& gpu_) : gpu{gpu_} {}

    void Method(u32 method, u32 arg) override;

  private:
    Gpu& gpu;

    // Commands
    void Copy(const u32 index, const u32 pixels_from_memory_src_y0_int);

    // Helpers
    renderer::ITextureView* GetTexture(const Texture2DInfo& info,
                                       renderer::TextureUsage usage);
};

} // namespace hydra::hw::tegra_x1::gpu::engines
