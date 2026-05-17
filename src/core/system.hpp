#pragma once

#include "core/horizon/os.hpp"
#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/wall_clock.hpp"
#include "core/input/device_manager.hpp"

namespace hydra::horizon::loader {
class LoaderBase;
}

namespace hydra {

struct CombinedTextureView {
    hw::tegra_x1::gpu::renderer::ITexture* base;
    hw::tegra_x1::gpu::renderer::ITextureView* view;

    ~CombinedTextureView();
};

class System {
    using clock_t = std::chrono::steady_clock;

  public:
    System(horizon::ui::IHandler& ui_handler_);
    ~System();

    void SetSurface(void* surface) { gpu.GetRenderer().SetSurface(surface); }

    enum class LoadAndStartError {
        ProcessAlreadyExists,
    };

    void LoadAndStart(horizon::loader::LoaderBase* loader);
    void RequestStop();
    void ForceStop();

    void Pause();
    void Resume();

    void NotifyOperationModeChanged() { os.NotifyOperationModeChanged(); }

    // TODO: rename?
    void ProgressFrame(u32 width, u32 height, bool& out_dt_average_updated);

    bool IsRunning() const;
    f32 GetLastDeltaTimeAverage() const { return last_dt_average; }

    void TakeScreenshot();
    void CaptureGpuFrame();

  private:
    horizon::ui::IHandler& ui_handler;

    hw::WallClock wall_clock;
    std::unique_ptr<hw::tegra_x1::cpu::ICpu> cpu;
    hw::tegra_x1::gpu::Gpu gpu;
    input::DeviceManager input_device_manager;
    std::unique_ptr<audio::ICore> audio_core;
    horizon::OS os;

    // Loading screen assets
    std::optional<CombinedTextureView> nintendo_logo{};
    std::vector<CombinedTextureView> startup_movie; // TODO: texture array?
    std::vector<std::chrono::milliseconds> startup_movie_delays;
    clock_t::time_point next_startup_movie_frame_time;
    clock_t::time_point startup_movie_fade_in_time;
    u32 startup_movie_frame{0};

    bool loading{false};

    // Process
    horizon::kernel::Process* main_process{nullptr};

    // Delta time
    f32 last_dt_average{0.0f};
    horizon::display::AccumulatedTime accumulated_dt;
    clock_t::time_point last_dt_averaging_time{clock_t::now()};

    // Helpers
    void TryApplyPatch(horizon::kernel::Process* process,
                       const std::string_view target_filename,
                       const std::filesystem::path path);

  public:
    GETTER(ui_handler, GetUIHandler);
    REF_GETTER(wall_clock, GetWallClock);
    hw::tegra_x1::cpu::ICpu& GetCpu() { return *cpu.get(); }
    REF_GETTER(gpu, GetGpu);
    REF_GETTER(input_device_manager, GetInputDeviceManager);
    audio::ICore& GetAudioCore() { return *audio_core.get(); }
    REF_GETTER(os, GetOS);
};

} // namespace hydra
