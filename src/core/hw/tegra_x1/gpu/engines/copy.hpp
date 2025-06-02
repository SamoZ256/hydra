#pragma once

#include "core/hw/tegra_x1/gpu/engines/engine_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {
class BufferBase;
class TextureBase;
} // namespace hydra::hw::tegra_x1::gpu::renderer

namespace hydra::hw::tegra_x1::gpu::engines {

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

struct RegsCopy {
    u32 padding_0x0[0xc0];

    u32 not_a_register_0xc0;

    u32 padding_0xc1[0x3f];

    // 0x100
    Iova offset_in;
    Iova offset_out;
    u32 stride_in;
    u32 stride_out;
    u32 line_length_in;
    u32 line_count;

    u32 padding_0x108[0xb8];

    // 0x1c0
    u32 remap_const1;
    u32 remap_const2;
    struct {
        u32 dst_x : 3;
        u32 padding1 : 1;
        u32 dst_y : 3;
        u32 padding2 : 1;
        u32 dst_z : 3;
        u32 padding3 : 1;
        u32 dst_w : 3;
        u32 padding4 : 1;
        u32 component_size_minus_one : 2;
        u32 padding5 : 2;
        u32 src_component_count_minus_one : 2;
        u32 padding6 : 2;
        u32 dst_component_count_minus_one : 2;
        u32 padding7 : 2;
        u32 padding8 : 4;
    } remap_components;
    TextureCopyInfo dst;

    u32 padding_0x1c9;

    // 0x1ca
    TextureCopyInfo src;

    // TODO
};

class Copy : public EngineWithRegsBase<RegsCopy> {
  public:
    void Method(u32 method, u32 arg) override;

  private:
    // Commands
    struct LaunchDMAData {
        TransferType type : 2;
        bool flush_enable : 1;
        SemaphoreType semaphore_type : 2;
        InterruptType interrupt_type : 2;
        MemoryLayout src_memory_layout : 1;
        MemoryLayout dst_memory_layout : 1;
        bool multi_line_enable : 1;
        bool remap_enable : 1;
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
    static renderer::BufferBase* GetBuffer(const Iova addr, const usize size);
    // static renderer::TextureBase* GetTexture(const u32 gpu_addr_lo,
    //                                          const u32 gpu_addr_hi,
    //                                          const TextureCopyInfo& info);
};

} // namespace hydra::hw::tegra_x1::gpu::engines

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::engines::BlockDim, OneGob,
                       "1 gob", TwoGobs, "2 gobs", FourGobs, "4 gobs",
                       EightGobs, "8 gobs", SixteenGobs, "16 gobs",
                       ThirtyTwoGobs, "32 gobs", QuarterGob, "quarter gob")
