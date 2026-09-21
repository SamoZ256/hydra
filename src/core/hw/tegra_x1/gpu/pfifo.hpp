#pragma once

#include "core/hw/tegra_x1/gpu/const.hpp"
#include "core/hw/tegra_x1/gpu/gmmu.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu {

class Gpu;

struct GpfifoEntryList {
    GMmu& gmmu;
    std::vector<GpfifoEntry> entries;
    GpfifoFlags flags;
};

class Pfifo {
  public:
    explicit Pfifo(Gpu& gpu_);
    ~Pfifo();

    void submitEntries(GMmu& gmmu, std::span<const GpfifoEntry> entries,
                       GpfifoFlags flags);

  private:
    Gpu& gpu;

    std::mutex mutex;
    std::condition_variable cond_var;

    std::queue<GpfifoEntryList> entry_lists;
    bool stop{false};

    std::thread thread; // TODO: jthread

    void threadFunc();

    void submitEntry(const GpfifoEntry entry);
    bool submitCommand(uptr& gpu_addr); // TODO: return void

    // Helpers
    void processMethodArg(u32 subchannel, uptr& gpu_addr, u32& method,
                          bool increment);
};

} // namespace hydra::hw::tegra_x1::gpu
