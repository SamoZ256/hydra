#pragma once

#include "core/hw/tegra_x1/gpu/const.hpp"
#include "core/hw/tegra_x1/gpu/engines/2d.hpp"
#include "core/hw/tegra_x1/gpu/engines/3d.hpp"
#include "core/hw/tegra_x1/gpu/engines/compute.hpp"
#include "core/hw/tegra_x1/gpu/engines/copy.hpp"
#include "core/hw/tegra_x1/gpu/engines/inline.hpp"
#include "core/hw/tegra_x1/gpu/pfifo.hpp"
#include "core/hw/tegra_x1/gpu/renderer/renderer_base.hpp"

#define GPU_INSTANCE hw::tegra_x1::gpu::Gpu::GetInstance()
#define RENDERER_INSTANCE GPU_INSTANCE.GetRenderer()

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu {

namespace renderer {
class ICommandBuffer;
}

struct MemoryMap {
    uptr addr = 0;
    usize size;
    bool write;
    // TODO: alignment
    // TODO: kind
};

constexpr usize SUBCHANNEL_COUNT = 5; // TODO: correct?

inline thread_local GMmu* tls_crnt_gmmu = nullptr;
inline thread_local renderer::ICommandBuffer* tls_crnt_command_buffer = nullptr;

class Gpu {
  public:
    static Gpu& GetInstance();

    Gpu();
    ~Gpu();

    // Memory map
    u32 CreateMap(usize size) {
        handle_id_t handle_id = memory_maps.AllocateHandle();
        MemoryMap& memory_map = memory_maps.Get(handle_id);
        memory_map = {};
        memory_map.size = size;

        // TODO: is this hack still needed?
        // HACK: allocate one more index. Games are probably confused with
        // handle IDs and IDs
        memory_maps.AllocateHandle();

        return handle_id;
    }

    void AllocateMap(handle_id_t handle_id, uptr addr, bool write) {
        MemoryMap& memory_map = memory_maps.Get(handle_id);
        memory_map.addr = addr;
        memory_map.write = write;
    }

    void FreeMap(handle_id_t handle_id) { memory_maps.Free(handle_id); }

    MemoryMap& GetMap(handle_id_t handle_id) {
        return memory_maps.Get(handle_id);
    }

    // Engines
    enum class GetEngineAtSubchannelError {
        InvalidSubchannel,
        NoEngineBound,
    };
    engines::EngineBase* GetEngineAtSubchannel(u32 subchannel) {
        ASSERT_THROWING_DEBUG(subchannel <= SUBCHANNEL_COUNT, Gpu,
                              GetEngineAtSubchannelError::InvalidSubchannel,
                              "Invalid subchannel {}", subchannel);

        auto engine = subchannels[subchannel];
        ASSERT_THROWING_DEBUG(
            engine, Gpu, GetEngineAtSubchannelError::NoEngineBound,
            "Subchannel {} does not have a bound engine", subchannel);

        return engine;
    }

    void SubchannelMethod(u32 subchannel, u32 method, u32 arg);

    void SubchannelFlushMacro(u32 subchannel) {
        GetEngineAtSubchannel(subchannel)->FlushMacro();
    }

    // Texture
    renderer::TextureBase* GetTexture(renderer::ICommandBuffer* command_buffer,
                                      cpu::IMmu* mmu,
                                      const NvGraphicsBuffer& buff);

    // Getters
    Pfifo& GetPfifo() { return pfifo; }
    renderer::RendererBase& GetRenderer() const { return *renderer; }

  private:
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
