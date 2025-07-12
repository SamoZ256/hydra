#pragma once

#include "core/hw/tegra_x1/gpu/const.hpp"
#include "core/hw/tegra_x1/gpu/gpu_mmu.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu {

class GpuMmu;

class Pfifo {
  public:
    Pfifo(GpuMmu& gpu_mmu_) : gpu_mmu{gpu_mmu_} {}

    // TODO: use std::span instead
    void SubmitEntries(const std::vector<GpfifoEntry>& entries,
                       GpfifoFlags flags);

  private:
    GpuMmu& gpu_mmu;

    void SubmitEntry(const GpfifoEntry entry);
    bool SubmitCommand(uptr& gpu_addr); // TODO: return void

    // Helpers
    template <typename T>
    T Read(uptr& gpu_addr) {
        T word = gpu_mmu.Load<T>(gpu_addr);
        gpu_addr += sizeof(T);

        return word;
    }

    void ProcessMethodArg(u32 subchannel, uptr& gpu_addr, u32& method,
                          bool increment);
};

} // namespace hydra::hw::tegra_x1::gpu
