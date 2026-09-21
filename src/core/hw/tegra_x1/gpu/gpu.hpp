#pragma once

#include "core/hw/tegra_x1/gpu/const.hpp"
#include "core/hw/tegra_x1/gpu/engines/2d.hpp"
#include "core/hw/tegra_x1/gpu/engines/3d.hpp"
#include "core/hw/tegra_x1/gpu/engines/compute.hpp"
#include "core/hw/tegra_x1/gpu/engines/copy.hpp"
#include "core/hw/tegra_x1/gpu/engines/inline.hpp"
#include "core/hw/tegra_x1/gpu/pfifo.hpp"
#include "core/hw/tegra_x1/gpu/renderer/renderer.hpp"

// TODO: remove dependency
#include "core/horizon/handle_pool.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu {

namespace renderer {
class ICommandBuffer;
}

struct MemoryMap {
    uptr addr = 0;
    u64 size;
    bool write;
    // TODO: alignment
    // TODO: kind
};

constexpr usize SUBCHANNEL_COUNT = 5; // TODO: correct?

inline thread_local GMmu* tls_crnt_gmmu = nullptr;
inline thread_local renderer::ICommandBuffer* tls_crnt_command_buffer = nullptr;

class Gpu {
  public:
    Gpu() noexcept;
    ~Gpu() noexcept = default;

    ZTD_MAKE_NON_COPYABLE(Gpu);
    ZTD_MAKE_NON_MOVABLE(Gpu);

    // Memory map
    Handle createMap(u64 size) { return memory_maps.insert(0, size).value(); }

    void allocateMap(Handle handle, uptr addr, bool write) {
        // TODO: error?
        ZTD_ASSIGN_OR_RETURN(auto memory_map, memory_maps.get(handle));
        memory_map->addr = addr;
        memory_map->write = write;
    }

    void freeMap(Handle handle) {
        ASSERT_DEBUG(memory_maps.free(handle), Gpu, "Failed to free map {}",
                     handle);
    }

    std::optional<MemoryMap*> getMap(Handle handle) {
        return memory_maps.get(handle);
    }

    // Engines
    std::optional<engines::EngineBase*> getEngineAtSubchannel(u32 subchannel) {
        if (subchannel > SUBCHANNEL_COUNT)
            return std::nullopt;
        return subchannels[subchannel];
    }

    void subchannelMethod(u32 subchannel, u32 method, u32 arg);

    void subchannelFlushMacro(u32 subchannel) {
        const auto engine = getEngineAtSubchannel(subchannel);
        if (!engine)
            LOG_FATAL(Gpu, "Invalid subchannel {}", subchannel);

        (*engine)->flushMacro();
    }

    // Texture
    renderer::ITextureView* getTexture(renderer::ICommandBuffer* command_buffer,
                                       cpu::IMmu* mmu,
                                       const NvGraphicsBuffer& buff);

    // Getters
    Pfifo& getPfifo() { return pfifo; }
    renderer::IRenderer& getRenderer() const { return *renderer; }

  private:
    // Pfifo
    Pfifo pfifo;

    // Engines
    engines::ThreeD three_d_engine;
    engines::Compute compute_engine;
    engines::Inline inline_engine;
    engines::TwoD two_d_engine;
    engines::Copy copy_engine;
    std::optional<engines::EngineBase*> subchannels[SUBCHANNEL_COUNT] = {};

    // Renderer
    std::unique_ptr<renderer::IRenderer> renderer;

    // Memory
    // TODO: move to nvmap
    // TODO: dynamic pool?
    horizon::StaticHandlePool<MemoryMap, 2048> memory_maps;
};

} // namespace hydra::hw::tegra_x1::gpu
