#pragma once

#include "core/hw/tegra_x1/gpu/const.hpp"
#include "core/hw/tegra_x1/gpu/engines/2d.hpp"
#include "core/hw/tegra_x1/gpu/engines/3d.hpp"
#include "core/hw/tegra_x1/gpu/engines/compute.hpp"
#include "core/hw/tegra_x1/gpu/engines/copy.hpp"
#include "core/hw/tegra_x1/gpu/engines/inline.hpp"
#include "core/hw/tegra_x1/gpu/gpu_mmu.hpp"
#include "core/hw/tegra_x1/gpu/pfifo.hpp"
#include "core/hw/tegra_x1/gpu/renderer/renderer_base.hpp"

#define GPU_INSTANCE hw::tegra_x1::gpu::Gpu::GetInstance()
#define RENDERER_INSTANCE GPU_INSTANCE.GetRenderer()

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu {

struct MemoryMap {
    uptr addr = 0;
    usize size;
    bool write;
    // TODO: alignment
    // TODO: kind
};

constexpr usize GPU_PAGE_SIZE = 0x20000; // Big page size (TODO: correct?)

constexpr usize SUBCHANNEL_COUNT = 5; // TODO: correct?

class Gpu {
  public:
    static Gpu& GetInstance();

    Gpu();
    ~Gpu();

    // Memory map
    u32 CreateMap(usize size) {
        handle_id_t handle_id = memory_maps.AllocateForIndex();
        MemoryMap& memory_map = memory_maps.GetRef(handle_id);
        memory_map = {};
        memory_map.size = size;

        // HACK: allocate one more index. Games are probably confused with
        // handle IDs and IDs
        memory_maps.AllocateForIndex();

        return handle_id;
    }

    void AllocateMap(handle_id_t handle_id, uptr addr, bool write) {
        MemoryMap& memory_map = memory_maps.GetRef(handle_id);
        memory_map.addr = addr;
        memory_map.write = write;
    }

    void FreeMap(handle_id_t handle_id) { memory_maps.Free(handle_id); }

    u32 GetMapId(handle_id_t handle_id) { return handle_id + 1; }

    handle_id_t GetMapHandleId(u32 id) { return id - 1; }

    MemoryMap& GetMap(handle_id_t handle_id) {
        return memory_maps.GetRef(handle_id);
    }

    MemoryMap& GetMapById(u32 id) { return GetMap(GetMapHandleId(id)); }

    // Address space
    uptr CreateAddressSpace(uptr ptr, usize size, uptr gpu_addr) {
        if (ptr == 0x0)
            ptr = reinterpret_cast<uptr>(malloc(size));

        AddressSpace as;
        as.ptr = ptr;
        as.size = size;

        if (gpu_addr == invalid<uptr>()) {
            gpu_addr =
                address_space_base; // TODO: ask the Mmu for a base address
            address_space_base += align(size, GPU_PAGE_SIZE);
        }
        gpu_mmu.Map(gpu_addr, as);

        return gpu_addr;
    }

    uptr AllocatePrivateAddressSpace(usize size, uptr gpu_addr) {
        return CreateAddressSpace(0x0, size, gpu_addr);
    }

    uptr MapBufferToAddressSpace(uptr ptr, usize size, uptr gpu_addr) {
        return CreateAddressSpace(ptr, size, gpu_addr);
    }

    // TODO: correct?
    void ModifyAddressSpace(uptr ptr, usize size, uptr gpu_addr) {
        auto& as = gpu_mmu.UnmapAddrToAddressSpace(gpu_addr);
        ASSERT_DEBUG(size == as.size, Gpu, "Size mismatch: {} != {}", size,
                     as.size)

        as.ptr = ptr;
    }

    // Engines
    engines::EngineBase* GetEngineAtSubchannel(u32 subchannel) {
        ASSERT_DEBUG(subchannel <= SUBCHANNEL_COUNT, Gpu,
                     "Invalid subchannel {}", subchannel);

        auto engine = subchannels[subchannel];
        ASSERT_DEBUG(engine, Gpu, "Subchannel {} does not have a bound engine",
                     subchannel);

        return engine;
    }

    void SubchannelMethod(u32 subchannel, u32 method, u32 arg);

    void SubchannelFlushMacro(u32 subchannel) {
        GetEngineAtSubchannel(subchannel)->FlushMacro();
    }

    // Texture
    renderer::TextureBase* GetTexture(cpu::IMmu* mmu,
                                      const NvGraphicsBuffer& buff);

    // Getters
    GpuMmu& GetGpuMmu() { return gpu_mmu; }
    Pfifo& GetPfifo() { return pfifo; }
    renderer::RendererBase* GetRenderer() const { return renderer; }

  private:
    // Address space
    GpuMmu gpu_mmu;
    uptr address_space_base{GPU_PAGE_SIZE};

    // Pfifo
    Pfifo pfifo;

    // Engines
    engines::ThreeD three_d_engine;
    engines::Compute compute_engine;
    engines::Inline inline_engine;
    engines::TwoD two_d_engine;
    engines::Copy copy_engine;
    engines::EngineBase* subchannels[SUBCHANNEL_COUNT] = {nullptr};

    // Renderer
    renderer::RendererBase* renderer;

    // Memory
    DynamicPool<MemoryMap> memory_maps;
};

} // namespace hydra::hw::tegra_x1::gpu
