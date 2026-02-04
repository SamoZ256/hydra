#pragma once

#include "core/hw/tegra_x1/gpu/const.hpp"
#include "core/hw/tegra_x1/gpu/gmmu.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu {

class GMmu;

struct GpfifoEntryList {
    GMmu& gmmu;
    std::vector<GpfifoEntry> entries;
    GpfifoFlags flags;
};

class Pfifo {
  public:
    Pfifo();
    ~Pfifo();

    void SubmitEntries(GMmu& gmmu, std::span<const GpfifoEntry> entries,
                       GpfifoFlags flags);

  private:
    std::mutex mutex;
    std::condition_variable cond_var;

    std::queue<GpfifoEntryList> entry_lists;
    bool stop{false};

    std::thread thread; // TODO: jthread

    void ThreadFunc();

    void SubmitEntry(const GpfifoEntry entry);
    bool SubmitCommand(uptr& gpu_addr); // TODO: return void

    // Helpers
    void ProcessMethodArg(u32 subchannel, uptr& gpu_addr, u32& method,
                          bool increment);
};

} // namespace hydra::hw::tegra_x1::gpu
