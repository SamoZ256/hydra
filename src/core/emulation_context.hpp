#pragma once

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/os.hpp"
#include "core/hw/tegra_x1/cpu/cpu.hpp"
#include "core/hw/tegra_x1/gpu/gpu.hpp"

namespace hydra {

namespace horizon::loader {
class LoaderBase;
}

class EmulationContext {
    using clock_t = std::chrono::steady_clock;

  public:
    EmulationContext(horizon::ui::HandlerBase& ui_handler);
    ~EmulationContext();

    void SetSurface(void* surface) { gpu->GetRenderer()->SetSurface(surface); }
    void LoadAndStart(horizon::loader::LoaderBase* loader);
    void RequestStop();
    void ForceStop();

    // TODO: rename?
    void ProgressFrame(u32 width, u32 height, bool& out_dt_average_updated);

    bool IsRunning() const {
        return os->GetKernel().GetProcessManager().HasRunningProcesses();
    }
    f32 GetLastDeltaTimeAverage() const { return last_dt_average; }

  private:
    // Objects
    hw::tegra_x1::cpu::ICpu* cpu;
    hw::tegra_x1::gpu::Gpu* gpu;
    audio::ICore* audio_core;
    horizon::OS* os;

    // Loading screen assets
    hw::tegra_x1::gpu::renderer::TextureBase* nintendo_logo = nullptr;
    std::vector<hw::tegra_x1::gpu::renderer::TextureBase*>
        startup_movie; // TODO: texture array?
    std::vector<std::chrono::milliseconds> startup_movie_delays;
    clock_t::time_point next_startup_movie_frame_time;
    clock_t::time_point startup_movie_fade_in_time;
    i32 startup_movie_frame{0};

    bool loading{false};

    // Delta time
    f32 last_dt_average{0.0f};
    std::chrono::nanoseconds accumulated_dt{0};
    u32 dt_sample_count{0};
    clock_t::time_point last_dt_averaging_time{clock_t::now()};

    bool Present(u32 width, u32 height,
                 std::vector<std::chrono::nanoseconds>& out_dt_ns_list);

    // Helpers
    void TryApplyPatch(horizon::kernel::Process* process,
                       const std::string_view target_filename,
                       const std::filesystem::path path);
};

} // namespace hydra
