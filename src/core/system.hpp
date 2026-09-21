#pragma once

#include "core/horizon/os.hpp"
#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/wall_clock.hpp"
#include "core/input/device_manager.hpp"

namespace hydra::horizon::loader {
class ILoader;
}

namespace hydra {

struct CombinedTextureView {
    hw::tegra_x1::gpu::renderer::ITexture* base;
    hw::tegra_x1::gpu::renderer::ITextureView* view;

    // TODO: uncomment
    // delete view;
    // delete base;
    ~CombinedTextureView() = default;
};

class System {
    using clock_t = std::chrono::steady_clock;

  public:
    explicit System(horizon::ui::IHandler& ui_handler_);
    ~System();

    void setSurface(void* surface) { gpu.getRenderer().setSurface(surface); }

    void loadAndStart(horizon::loader::ILoader* loader);
    void requestStop();
    void forceStop();

    void pause();
    void resume();

    void notifyOperationModeChanged() { os.notifyOperationModeChanged(); }

    // TODO: rename?
    void progressFrame(u32 width, u32 height, bool& out_dt_average_updated);

    bool isRunning() const;
    f32 getLastDeltaTimeAverage() const { return last_dt_average; }

    void takeScreenshot();
    void captureGpuFrame();

  private:
    horizon::ui::IHandler& ui_handler;

    hw::WallClock wall_clock;
    std::unique_ptr<hw::tegra_x1::cpu::ICpu> cpu;
    hw::tegra_x1::gpu::Gpu gpu;
    input::DeviceManager input_device_manager;
    std::unique_ptr<audio::ICore> audio_core;
    horizon::OS os;

    // Loading screen assets
    std::optional<CombinedTextureView> nintendo_logo;
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
    static void tryApplyPatch(horizon::kernel::Process* process,
                              const std::string_view target_filename,
                              const std::filesystem::path& path);

  public:
    GETTER(ui_handler, getUiHandler);
    REF_GETTER(wall_clock, getWallClock);
    hw::tegra_x1::cpu::ICpu& getCpu() { return *cpu; }
    REF_GETTER(gpu, getGpu);
    REF_GETTER(input_device_manager, getInputDeviceManager);
    audio::ICore& getAudioCore() { return *audio_core; }
    REF_GETTER(os, getOs);
};

} // namespace hydra
