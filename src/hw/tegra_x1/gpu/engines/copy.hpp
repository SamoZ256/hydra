#pragma once

#include "hw/tegra_x1/gpu/engines/engine_base.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer {
class BufferBase;
class TextureBase;
} // namespace Hydra::HW::TegraX1::GPU::Renderer

namespace Hydra::HW::TegraX1::GPU::Engines {

enum class TransferType : u32 {
    None,
    Pipelined,
    NonPipelined,
};

enum class SemaphoreType : u32 {
    None,
    ReleaseOneWord,
    ReleaseFourWords,
};

enum class InterruptType : u32 {
    None,
    Blocking,
    NonBlocking,
};

enum class SemaphoreReduction : u32 {
    Imin,
    Imax,
    Ixor,
    Iand,
    Ior,
    Iadd,
    Inc,
    Dec,
    Fadd = 10,
};

enum class BypassL2 {
    UsePteSetting,
    ForceVolatile,
};

enum class GobHeight : u32 {
    Tesla4,
    Fermi8,
};

struct BlockSize {
    BlockDim width : 4;
    BlockDim height : 4;
    BlockDim depth : 4;
    GobHeight gob_height : 4;
    // TODO: more
};

struct TextureCopyInfo {
    BlockSize block_size;
    u32 stride; // dst_width;
    u32 height;
    u32 depth;
    u32 layer;
    struct {
        u32 x : 16;
        u32 y : 16;
    } origin;
};

union RegsCopy {
    struct {
        u32 padding_0x0[0xc0];

        u32 not_a_register_0xc0;

        u32 padding_0xc1[0x3f];

        // 0x100
        u32 offset_in_hi;
        u32 offset_in_lo;
        u32 offset_out_hi;
        u32 offset_out_lo;
        u32 stride_in;
        u32 stride_out;
        u32 line_length_in;
        u32 line_count;

        u32 padding_0x108[0xb8];

        // 0x1c0
        u32 remap_const_hi;
        u32 remap_const_lo;
        u32 remap_components;
        TextureCopyInfo dst;

        u32 padding_0x1c9;

        // 0x1ca
        TextureCopyInfo src;

        // TODO
    };

    u32 raw[0x446];
};

class Copy : public EngineBase {
  public:
    void Method(u32 method, u32 arg) override;

  protected:
    void WriteReg(u32 reg, u32 value) override {
        LOG_DEBUG(Engines, "Writing to copy reg 0x{:08x} (value: 0x{:08x})",
                  reg, value);
        regs.raw[reg] = value;
    }

  private:
    RegsCopy regs;

    // Commands
    struct LaunchDMAData {
        TransferType type : 2;
        bool flush_enable : 1;
        SemaphoreType semaphore_type : 2;
        InterruptType interrupt_type : 2;
        MemoryLayout src_memory_layout : 1;
        MemoryLayout dst_memory_layout : 1;
        bool multi_line_enable : 1;
        bool force_rmw_disable : 1;
        bool src_memory_is_physical : 1;
        bool dst_memory_is_physical : 1;
        SemaphoreReduction semaphore_reduction : 4;
        bool semaphore_reduction_unsigned : 1;
        bool semaphore_reduction_enable : 1;
        BypassL2 bypass_l2 : 1;
    };

    void LaunchDMA(const u32 index, const LaunchDMAData data);

    // Helpers
    static Renderer::BufferBase*
    GetBuffer(const u32 gpu_addr_lo, const u32 gpu_addr_hi, const usize size);
    static Renderer::TextureBase* GetTexture(const u32 gpu_addr_lo,
                                             const u32 gpu_addr_hi,
                                             const TextureCopyInfo& info);
};

} // namespace Hydra::HW::TegraX1::GPU::Engines

ENABLE_ENUM_FORMATTING(Hydra::HW::TegraX1::GPU::Engines::BlockDim, OneGob,
                       "1 gob", TwoGobs, "2 gobs", FourGobs, "4 gobs",
                       EightGobs, "8 gobs", SixteenGobs, "16 gobs",
                       ThirtyTwoGobs, "32 gobs", QuarterGob, "quarter gob")
