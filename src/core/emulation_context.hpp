#pragma once

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/os.hpp"
#include "core/hw/bus.hpp"
#include "core/hw/display/display.hpp"
#include "core/hw/tegra_x1/cpu/cpu_base.hpp"
#include "core/hw/tegra_x1/gpu/gpu.hpp"

namespace hydra {

class EmulationContext {
    using clock_t = std::chrono::steady_clock;

  public:
    EmulationContext(horizon::ui::HandlerBase& ui_handler);
    ~EmulationContext();

    void SetSurface(void* surface) { gpu->GetRenderer()->SetSurface(surface); }
    void LoadRom(const std::string& rom_filename);
    void Run();

    // TODO: rename?
    void ProgressFrame(u32 width, u32 height, bool& out_dt_average_updated);

    // Getters
    hw::tegra_x1::cpu::CPUBase* GetCPU() const { return cpu; }
    hw::tegra_x1::gpu::GPU* GetGPU() const { return gpu; }
    hw::Bus* GetBus() const { return bus; }
    horizon::OS* GetOS() const { return os; }

    u64 GetTitleID() const { return os->GetKernel().GetTitleID(); }

    bool IsRunning() const { return is_running; }
    f32 GetLastDeltaTimeAverage() const { return last_dt_average; }

  private:
    // Objects
    hw::tegra_x1::cpu::CPUBase* cpu;
    hw::tegra_x1::gpu::GPU* gpu;
    hw::display::Display* builtin_display;
    hw::Bus* bus;
    audio::CoreBase* audio_core;
    horizon::OS* os;

    horizon::kernel::Process* process;

    std::atomic_bool is_running = false;

    // Delta time
    f32 last_dt_average{0.0f};

    u64 accumulated_dt_ns{0};
    u32 dt_sample_count{0};
    clock_t::time_point last_dt_averaging_time{clock_t::now()};

    void Present(u32 width, u32 height, std::vector<u64>& out_dt_ns_list);

    // Helpers
    void TryApplyPatch(const std::string_view target_filename,
                       const std::filesystem::path path);
};

} // namespace hydra
