#include "core/emulation_context.hpp"

#include <fmt/chrono.h>
#include <hatch/hatch.hpp>
#include <stb_image_write.h>

#include "core/audio/null/core.hpp"
#include "core/horizon/applets/album/const.hpp"
#include "core/horizon/applets/const.hpp"
#include "core/horizon/applets/controller/const.hpp"
#include "core/horizon/applets/error/const.hpp"
#include "core/horizon/applets/mii_edit/const.hpp"
#include "core/horizon/applets/software_keyboard/const.hpp"
#include "core/horizon/loader/nca_loader.hpp"
#include "core/horizon/loader/nro_loader.hpp"
#include "core/horizon/loader/nso_loader.hpp"
#include "core/horizon/services/am/library_applet_controller.hpp"
#include "core/hw/tegra_x1/cpu/dynarmic/cpu.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"
#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_base.hpp"
#include "core/input/device_manager.hpp"

#if HYDRA_HYPERVISOR_ENABLED
#include "core/hw/tegra_x1/cpu/hypervisor/cpu.hpp"
#endif

#if HYDRA_CUBEB_ENABLED
#include "core/audio/cubeb/core.hpp"
#endif

namespace hydra {

namespace {

constexpr auto STARTUP_MOVIE_FADE_IN_DURATION = 100ms;
constexpr auto STARTUP_MOVIE_BREAK_AFTER_FADE_IN_DURATION = 200ms;

} // namespace

EmulationContext::EmulationContext(horizon::ui::HandlerBase& ui_handler) {
    LOGGER_INSTANCE.SetOutput(CONFIG_INSTANCE.GetLogOutput());

    // Random
    srand(static_cast<u32>(time(0)));

    // Initialize
    switch (CONFIG_INSTANCE.GetCpuBackend()) {
    case CpuBackend::AppleHypervisor:
#if HYDRA_HYPERVISOR_ENABLED
        cpu = new hw::tegra_x1::cpu::hypervisor::Cpu();
#else
        LOG_FATAL(Other, "Apple Hypervisor not supported");
#endif
        break;
    case CpuBackend::Dynarmic:
        cpu = new hw::tegra_x1::cpu::dynarmic::Cpu();
        break;
    default:
        // TODO: return an error instead
        LOG_FATAL(Other, "Unknown CPU backend {}",
                  CONFIG_INSTANCE.GetCpuBackend());
        break;
    }

    gpu = new hw::tegra_x1::gpu::Gpu();

    switch (CONFIG_INSTANCE.GetAudioBackend()) {
    case AudioBackend::Null:
        audio_core = new audio::null::Core();
        break;
    case AudioBackend::Cubeb:
#if HYDRA_CUBEB_ENABLED
        audio_core = new audio::cubeb::Core();
#else
        LOG_FATAL(Other, "cubeb not supported");
#endif
        break;
    default:
        // TODO: return an error instead
        LOG_FATAL(Other, "Unknown audio backend {}",
                  CONFIG_INSTANCE.GetAudioBackend());
        break;
    }

    os = new horizon::OS(*audio_core, ui_handler);

    // Filesystem
    /*
    for (const auto& root_path : CONFIG_INSTANCE.GetRootPaths()) {
        const auto res =
            horizon::Filesystem::Filesystem::GetInstance().AddEntry(
                root_path.guest_path, root_path.host_path, true);
        ASSERT(res == horizon::Filesystem::FsResult::Success, Services,
               "Failed to get root path: {}", res);
        LOG_INFO(Other, "Mapped {} to {}", root_path.guest_path,
                 root_path.host_path);
    }
    */
}

EmulationContext::~EmulationContext() {
    delete os;
    delete audio_core;
    delete gpu;
    delete cpu;

    LOGGER_INSTANCE.SetOutput(LogOutput::StdOut);
}

void EmulationContext::LoadAndStart(horizon::loader::LoaderBase* loader) {
    // Process
    ASSERT(process == nullptr, Other, "Process already exists");
    process =
        os->GetKernel().GetProcessManager().CreateProcess("Guest process");
    loader->LoadProcess(process);

    // Check for firmware applets
    auto controller = new horizon::services::am::LibraryAppletController(
        horizon::LibraryAppletMode::AllForeground);
    switch (loader->GetTitleID()) {
    case 0x0100000000001003: { // controller
        // Common args
        auto common_args = new horizon::applets::CommonArguments{
            .version = 1,
            .size = sizeof(horizon::applets::CommonArguments),
            .library_applet_api_version = 1, // TODO: correct?
            .theme_color = 0,                // HACK
            .play_startup_sound = false,     // HACK
            .system_tick = get_absolute_time(),
        };
        controller->PushInData(
            new horizon::services::am::IStorage(common_args));

        // Arg
        horizon::applets::controller::SupportArg<4> arg{
            .player_count_min = 0,
            .player_count_max = 2,
            .enable_take_over_connection = true,
            .enable_left_justify = false,
            .enable_permit_joy_dual = true,
            .enable_single_mode = true,
            .enable_identification_color = false,
            .enable_explain_text = false,
        };

        // Private arg
        auto private_arg = new horizon::applets::controller::ArgPrivate{
            .size = sizeof(horizon::applets::controller::ArgPrivate),
            .controller_support_arg_size = sizeof(arg),
            .flag0 = 0,
            .flag1 = 0,
            .mode = horizon::applets::controller::Mode::ShowControllerSupport,
            .caller = horizon::applets::controller::Caller::Application,
            .npad_style_set = horizon::hid::NpadStyleSet::JoyDual,
            .npad_joy_hold_type = horizon::hid::NpadJoyHoldType::Vertical,
        };
        controller->PushInData(
            new horizon::services::am::IStorage(private_arg));

        auto arg_ptr = (u8*)malloc(sizeof(arg));
        memcpy(arg_ptr, &arg, sizeof(arg));
        controller->PushInData(new horizon::services::am::IStorage(arg_ptr));

        break;
    }
    case 0x0100000000001005: { // error
        // Common args
        auto common_args = new horizon::applets::CommonArguments{
            .version = 1,
            .size = sizeof(horizon::applets::CommonArguments),
            .library_applet_api_version = 1, // TODO: correct?
            .theme_color = 0,                // HACK
            .play_startup_sound = false,     // HACK
            .system_tick = get_absolute_time(),
        };
        controller->PushInData(
            new horizon::services::am::IStorage(common_args));

        // Param common
        auto param_common = new horizon::applets::error::ParamCommon{
            .type = horizon::applets::error::ErrorType::ApplicationError,
            .is_jump_enabled = false,
        };
        controller->PushInData(
            new horizon::services::am::IStorage(param_common));

        // Param for application error
        auto param_for_application_error =
            new horizon::applets::error::ParamForApplicationError{
                .version = 1,
                .error_code_number = MAKE_RESULT(Svc, 0),
                .language_code = horizon::LanguageCode::AmericanEnglish,
                .dialog_message = "Dialog message",
                .fullscreen_message = "Fullscreen message",
            };
        controller->PushInData(
            new horizon::services::am::IStorage(param_for_application_error));

        break;
    }
    case 0x0100000000001008: { // swkbd
        // Common args
        auto common_args = new horizon::applets::CommonArguments{
            .version = 1,
            .size = sizeof(horizon::applets::CommonArguments),
            .library_applet_api_version = 1, // TODO: correct?
            .theme_color = 0,                // HACK
            .play_startup_sound = false,     // HACK
            .system_tick = get_absolute_time(),
        };
        controller->PushInData(
            new horizon::services::am::IStorage(common_args));

        // Config
        auto config =
            new horizon::applets::software_keyboard::KeyboardConfigCommon{
                .mode = horizon::applets::software_keyboard::KeyboardMode::Full,
                // TODO: more
            };
        controller->PushInData(new horizon::services::am::IStorage(config));

        break;
    }
    case 0x0100000000001009: { // miiEdit
        // Args
        auto args = new horizon::applets::mii_edit::AppletInput{
            ._unknown_x0 = 0x3,
            .mode = horizon::applets::mii_edit::AppletMode::ShowMiiEdit,
        };
        controller->PushInData(new horizon::services::am::IStorage(args));

        break;
    }
    case 0x010000000000100d: { // photoViewer
        // Common args
        auto common_args = new horizon::applets::CommonArguments{
            .version = 1,
            .size = sizeof(horizon::applets::CommonArguments),
            .library_applet_api_version = 1, // TODO: correct?
            .theme_color = 0,                // HACK
            .play_startup_sound = false,     // HACK
            .system_tick = get_absolute_time(),
        };
        controller->PushInData(
            new horizon::services::am::IStorage(common_args));

        // Arg
        auto arg = new horizon::applets::album::Arg{
            horizon::applets::album::Arg::ShowAllAlbumFilesForHomeMenu};
        controller->PushInData(new horizon::services::am::IStorage(arg));

        break;
    }
    default:
        break;
    }

    os->SetLibraryAppletSelfController(controller);

    // Loading screen assets
    {
        usize width, height;
        if (auto data = loader->LoadNintendoLogo(width, height)) {
            hw::tegra_x1::gpu::renderer::TextureDescriptor descriptor(
                0x0, hw::tegra_x1::gpu::renderer::TextureFormat::RGBA8Unorm,
                hw::tegra_x1::gpu::NvKind::Generic_16BX2, width, height, 0x0,
                width * 4);
            nintendo_logo = gpu->GetRenderer().CreateTexture(descriptor);
            nintendo_logo->CopyFrom(reinterpret_cast<uptr>(data));
            free(data);
        }
    }
    {
        usize width, height;
        u32 frame_count;
        if (auto data = loader->LoadStartupMovie(startup_movie_delays, width,
                                                 height, frame_count)) {
            hw::tegra_x1::gpu::renderer::TextureDescriptor descriptor(
                0x0, hw::tegra_x1::gpu::renderer::TextureFormat::RGBA8Unorm,
                hw::tegra_x1::gpu::NvKind::Generic_16BX2, width, height, 0x0,
                width * 4);
            startup_movie.reserve(frame_count);
            for (u32 i = 0; i < frame_count; i++) {
                auto frame = gpu->GetRenderer().CreateTexture(descriptor);
                frame->CopyFrom(
                    reinterpret_cast<uptr>(data + i * height * width));
                startup_movie.push_back(frame);
            }
            free(data);

            // Extend the last frame's time
            startup_movie_delays.back() = 5s;
        }
    }

    LOG_INFO(Other, "-------- Title info --------");
    LOG_INFO(Other, "Title ID: {:016x}", loader->GetTitleID());

    // Patch
    const auto target_patch_filename =
        fmt::format("{:016x}.hatch", loader->GetTitleID());
    // TODO: iterate recursively
    for (const auto& patch_path : CONFIG_INSTANCE.GetPatchPaths().Get()) {
        if (!std::filesystem::exists(patch_path)) {
            LOG_ERROR(Other, "Patch path does not exist: {}", patch_path);
            continue;
        }

        if (!std::filesystem::is_directory(patch_path)) {
            // File
            TryApplyPatch(process, target_patch_filename, patch_path);
        } else {
            // Directory
            // TODO: iterate recursively
            for (const auto& dir_entry :
                 std::filesystem::directory_iterator{patch_path}) {
                TryApplyPatch(process, target_patch_filename,
                              dir_entry.path().string());
            }
        }
    }

    LOG_INFO(Other, "-------- Config --------");
    CONFIG_INSTANCE.Log();

    LOG_INFO(Other, "-------- Run --------");

    // Enter focus
    // HACK: games expect focus change to be the second message?
    process->GetAppletState().SendMessage(
        horizon::kernel::AppletMessage::Resume);
    process->GetAppletState().SetFocusState(
        horizon::kernel::AppletFocusState::InFocus);

    // Preselected user
    auto user_id = CONFIG_INSTANCE.GetUserID().Get();
    if (user_id == horizon::services::account::internal::INVALID_USER_ID) {
        // If there is just a single user, use that
        if (USER_MANAGER_INSTANCE.GetUserCount() == 1) {
            user_id = USER_MANAGER_INSTANCE.GetUserIDs()[0];
        } else {
            // TODO: launch a select user applet in case the game requires it
            LOG_FATAL(Other, "Multiple user accounts");
        }
    }

    if (user_id != horizon::services::account::internal::INVALID_USER_ID) {
        process->GetAppletState().PushPreselectedUser(user_id);
        LOG_INFO(Other, "Preselected user with ID {:032x}", user_id);
    }

    process->Start();

    // Activate GDB server
    if (CONFIG_INSTANCE.GetGdbEnabled()) {
        if (CONFIG_INSTANCE.GetGdbWaitForClient())
            process->GetMainThread()->SupervisorPause();

        // HACK: spinlock until the main thread is running
        while (!process->IsRunning())
            std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        DEBUGGER_MANAGER_INSTANCE.GetDebugger(process).ActivateGdbServer();
    }

    // Loading screen
    loading = true;

    const auto crnt_time = clock_t::now();
    next_startup_movie_frame_time = crnt_time + STARTUP_MOVIE_FADE_IN_DURATION +
                                    STARTUP_MOVIE_BREAK_AFTER_FADE_IN_DURATION;
    startup_movie_fade_in_time = crnt_time + STARTUP_MOVIE_FADE_IN_DURATION;
}

void EmulationContext::RequestStop() {
    // We don't request the processes to stop yet, instead we send a message to
    // all of them and give them some time to react
    for (auto it = os->GetKernel().GetProcessManager().Begin();
         it != os->GetKernel().GetProcessManager().End(); ++it)
        (*it)->GetAppletState().SendMessage(
            horizon::kernel::AppletMessage::Exit);
}

void EmulationContext::ForceStop() {
    // Request all processes to stop immediately
    for (auto it = os->GetKernel().GetProcessManager().Begin();
         it != os->GetKernel().GetProcessManager().End(); ++it)
        (*it)->Stop();

    // Wait a small amount of time for all threads to catch up
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if all processes have stopped
    if (IsRunning()) {
        // If some processes are still running, just abort
        LOG_FATAL(Other, "Failed to stop all processes");
    }
}

void EmulationContext::ProgressFrame(u32 width, u32 height,
                                     bool& out_dt_average_updated) {
    // Set the resolution for OS
    os->SetSurfaceResolution({width, height});

    // Input
    INPUT_DEVICE_MANAGER_INSTANCE.Poll();

    // Present

    // Acquire surface
    auto& renderer = gpu->GetRenderer();
    // NOTE: this waits for a surface to be available. We don't lock the mutex,
    // as that would block all other rendering for a long time. The mutex also
    // doesn't need to be locked, as all surface related operations are done on
    // this thread.
    if (!renderer.AcquireNextSurface())
        return;

    // Delta time
    {
        auto layer = os->GetDisplayDriver().GetFirstLayerForProcess(process);
        if (layer)
            accumulated_dt += layer->GetAccumulatedDT();
    }

    renderer.LockMutex();

    // Acquire present textures
    bool acquired = os->GetDisplayDriver().AcquirePresentTextures();

    // Render pass
    renderer.BeginSurfaceRenderPass();
    os->GetDisplayDriver().Present(width, height);

    if (acquired && loading) {
        // TODO: till when should the loading screen be shown?
        // Stop the loading screen on the first present
        loading = false;

        // Free loading assets
        if (nintendo_logo) {
            delete nintendo_logo;
            nintendo_logo = nullptr;
        }
        if (!startup_movie.empty()) {
            for (auto frame : startup_movie)
                delete frame;
            startup_movie.clear();
            startup_movie.shrink_to_fit();
            startup_movie_delays.clear();
            startup_movie_delays.shrink_to_fit();
        }
    } else if (loading) {
        const auto crnt_time = clock_t::now();

        // Display loading screen

        // Fade in
        f32 opacity = 1.0f;
        if (crnt_time < startup_movie_fade_in_time)
            opacity =
                1.0f -
                std::chrono::duration_cast<std::chrono::duration<f32>>(
                    startup_movie_fade_in_time - crnt_time) /
                    std::chrono::duration_cast<std::chrono::duration<f32>>(
                        STARTUP_MOVIE_FADE_IN_DURATION);

        // Nintendo logo
        if (nintendo_logo) {
            int2 size = {(i32)nintendo_logo->GetDescriptor().width,
                         (i32)nintendo_logo->GetDescriptor().height};
            int2 dst_offset = {32, 32};
            renderer.DrawTextureToSurface(
                nintendo_logo, IntRect2D({0, 0}, size),
                IntRect2D(dst_offset, size), true, opacity);
        }

        // Startup movie
        if (!startup_movie.empty()) {
            // Progress frame
            while (crnt_time > next_startup_movie_frame_time) {
                startup_movie_frame =
                    (startup_movie_frame + 1) % startup_movie.size();
                next_startup_movie_frame_time +=
                    startup_movie_delays[startup_movie_frame];
            }

            auto frame = startup_movie[startup_movie_frame];
            int2 size = {(i32)frame->GetDescriptor().width,
                         (i32)frame->GetDescriptor().height};
            int2 dst_offset = {(i32)width - size.x() - 32,
                               (i32)height - size.y() - 32};
            renderer.DrawTextureToSurface(frame, IntRect2D({0, 0}, size),
                                          IntRect2D(dst_offset, size), true,
                                          opacity);
        }
    }

    if (!loading) {
        // Delta time
        const auto now = clock_t::now();
        const auto time_since_last_dt_averaging = now - last_dt_averaging_time;
        if (time_since_last_dt_averaging > 1s) {
            if (bool(accumulated_dt))
                last_dt_average = f32(accumulated_dt);
            else
                last_dt_average = 0.f;
            accumulated_dt = {};
            last_dt_averaging_time = now;

            out_dt_average_updated = true;
        } else {
            out_dt_average_updated = false;
        }
    }

    renderer.EndSurfaceRenderPass();
    renderer.PresentSurface();
    renderer.EndCommandBuffer();
    renderer.UnlockMutex();

    // Signal V-Sync
    os->GetDisplayDriver().SignalVSync();
}

void EmulationContext::TakeScreenshot() {
    auto layer = os->GetDisplayDriver().GetFirstLayerForProcess(process);
    if (!layer)
        return;

    auto texture = layer->GetPresentTexture();
    if (!texture)
        return;

    std::thread thread([=]() {
        // Get the image data
        auto rect = layer->GetSrcRect();

        // Check if the image is flipped
        ASSERT(rect.size.x() > 0, Other, "Invalid width {}", rect.size.x());
        bool flip_y = false;
        if (rect.size.y() < 0) {
            rect.origin.y() += rect.size.y();
            rect.size.y() = -rect.size.y();
            flip_y = true;
        }

        // Copy to a buffer
        RENDERER_INSTANCE.LockMutex();
        auto buffer = RENDERER_INSTANCE.AllocateTemporaryBuffer(
            rect.size.y() * rect.size.x() * 4);
        buffer->CopyFrom(texture, rect.origin, rect.size);
        RENDERER_INSTANCE.EndCommandBuffer();
        RENDERER_INSTANCE.UnlockMutex();

        // TODO: wait for the command buffer to finish

        // Save the image to file
        auto now = std::chrono::system_clock::now();
        // TODO: use title name in the filename
        std::string filename =
            fmt::format("{}/screenshot_{:%Y-%m-%d_%H-%M-%S}.jpg",
                        CONFIG_INSTANCE.GetPicturesPath(), now);

        stbi_flip_vertically_on_write(flip_y);
        if (!stbi_write_jpg(filename.c_str(), rect.size.x(), rect.size.y(), 4,
                            (void*)buffer->GetDescriptor().ptr, 100))
            LOG_ERROR(Other, "Failed to save screenshot to {}", filename);
        stbi_flip_vertically_on_write(false);

        // Free the buffer
        RENDERER_INSTANCE.LockMutex();
        RENDERER_INSTANCE.FreeTemporaryBuffer(buffer);
        RENDERER_INSTANCE.UnlockMutex();
    });
    thread.detach();
}

void EmulationContext::CaptureGpuFrame() {
    gpu->GetRenderer().LockMutex();
    gpu->GetRenderer().CaptureFrame();
    gpu->GetRenderer().UnlockMutex();
}

void EmulationContext::TryApplyPatch(horizon::kernel::Process* process,
                                     const std::string_view target_filename,
                                     const std::filesystem::path path) {
    if (to_lower(path.filename().string()) != target_filename)
        return;

    LOG_INFO(Other, "Applying patch \"{}\"", path.string());

    std::ifstream ifs(path);

    // Deserialize
    Hatch::Deserializer deserializer;
    deserializer.Deserialize(ifs);

    const auto& hatch = deserializer.GetHatch();

    // Memory patch
    for (const auto& entry : hatch.GetMemoryPatch())
        process->GetMmu()->Write<u32>(entry.addr, entry.value);

    ifs.close();
}

} // namespace hydra
