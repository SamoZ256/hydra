#include "core/system.hpp"

#include <fmt/chrono.h>
#include <stb_image_write.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"

#include <cstddef>
#include <hatch/hatch.hpp>

#pragma GCC diagnostic pop

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
#include "core/horizon/services/am/internal/library_applet_controller.hpp"
#include "core/hw/tegra_x1/cpu/dynarmic/cpu.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"
#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/command_buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/surface_compositor.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_view.hpp"
#include "core/input/device_manager.hpp"

#ifdef HYDRA_HYPERVISOR_ENABLED
#include "core/hw/tegra_x1/cpu/hypervisor/cpu.hpp"
#endif

#ifdef HYDRA_CUBEB_ENABLED
#include "core/audio/cubeb/core.hpp"
#endif

namespace hydra {

namespace {

constexpr auto STARTUP_MOVIE_FADE_IN_DURATION = 100ms;
constexpr auto STARTUP_MOVIE_BREAK_AFTER_FADE_IN_DURATION = 200ms;

hw::tegra_x1::cpu::ICpu* createCpu() {
    switch (CONFIG_INSTANCE.getCpuBackend()) {
    case CpuBackend::AppleHypervisor:
#ifdef HYDRA_HYPERVISOR_ENABLED
        return new hw::tegra_x1::cpu::hypervisor::Cpu();
#else
        LOG_FATAL(Other, "Apple Hypervisor not supported");
#endif
    case CpuBackend::Dynarmic:
        return new hw::tegra_x1::cpu::dynarmic::Cpu();
    default:
        // TODO: return an error instead
        LOG_FATAL(Other, "Unknown CPU backend {}",
                  CONFIG_INSTANCE.getCpuBackend());
    }
}

audio::ICore* createAudioCore() {
    switch (CONFIG_INSTANCE.getAudioBackend()) {
    case AudioBackend::Null:
        return new audio::null::Core();
    case AudioBackend::Cubeb:
#ifdef HYDRA_CUBEB_ENABLED
        return new audio::cubeb::Core();
#else
        LOG_FATAL(Other, "cubeb not supported");
#endif
    default:
        // TODO: return an error instead
        LOG_FATAL(Other, "Unknown audio backend {}",
                  CONFIG_INSTANCE.getAudioBackend());
    }
}

} // namespace

System::System(horizon::ui::IHandler& ui_handler_)
    : ui_handler{ui_handler_}, cpu{createCpu()}, audio_core{createAudioCore()},
      os(*this) {
    // TODO: set this elsewhere
    LOGGER_INSTANCE.setOutput(CONFIG_INSTANCE.getLogOutput());
}

System::~System() {
    // TODO: set this elsewhere
    LOGGER_INSTANCE.setOutput(LogOutput::StdOut);
}

void System::loadAndStart(horizon::loader::ILoader* loader) {
    // Process
    ASSERT(main_process == nullptr, Other, "Process already exists");
    main_process =
        os.getKernel().getProcessManager().createProcess("Guest process");
    loader->loadProcess(*this, main_process);

    // Check for firmware applets
    horizon::services::am::internal::LibraryAppletController controller(
        horizon::LibraryAppletMode::AllForeground);
    // TODO: correct?
    u64 system_tick;
    os.getKernel().getSystemTick(system_tick);
    switch (loader->getTitleId()) {
    case 0x0100000000001003: { // controller
        // Common args
        horizon::applets::CommonArguments common_args{
            .version = 1,
            .size = sizeof(horizon::applets::CommonArguments),
            .library_applet_api_version = 1, // TODO: correct?
            .theme_color = 0,                // HACK
            .play_startup_sound = false,     // HACK
            .system_tick = system_tick,
        };
        controller.pushInData(new horizon::services::am::IStorage(common_args));

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
        horizon::applets::controller::ArgPrivate private_arg{
            .size = sizeof(horizon::applets::controller::ArgPrivate),
            .controller_support_arg_size = sizeof(arg),
            .flag0 = 0,
            .flag1 = 0,
            .mode = horizon::applets::controller::Mode::ShowControllerSupport,
            .caller = horizon::applets::controller::Caller::Application,
            .npad_style_set = horizon::services::hid::NpadStyleSet::JoyDual,
            .npad_joy_hold_type =
                horizon::services::hid::NpadJoyHoldType::Vertical,
        };
        controller.pushInData(new horizon::services::am::IStorage(private_arg));

        controller.pushInData(new horizon::services::am::IStorage(arg));

        break;
    }
    case 0x0100000000001005: { // error
        // Common args
        horizon::applets::CommonArguments common_args{
            .version = 1,
            .size = sizeof(horizon::applets::CommonArguments),
            .library_applet_api_version = 1, // TODO: correct?
            .theme_color = 0,                // HACK
            .play_startup_sound = false,     // HACK
            .system_tick = system_tick,
        };
        controller.pushInData(new horizon::services::am::IStorage(common_args));

        // Param common
        horizon::applets::error::ParamCommon param_common{
            .type = horizon::applets::error::ErrorType::ApplicationError,
            .is_jump_enabled = false,
        };
        controller.pushInData(
            new horizon::services::am::IStorage(param_common));

        // Param for application error
        horizon::applets::error::ParamForApplicationError
            param_for_application_error{
                .version = 1,
                .error_code_number = MAKE_RESULT(Svc, 0),
                .language_code = horizon::toLanguageCode(
                    CONFIG_INSTANCE.getSystemLanguage()),
                .dialog_message = "Dialog message",
                .fullscreen_message = "Fullscreen message",
        };
        controller.pushInData(
            new horizon::services::am::IStorage(param_for_application_error));

        break;
    }
    case 0x0100000000001008: { // swkbd
        // Common args
        horizon::applets::CommonArguments common_args{
            .version = 1,
            .size = sizeof(horizon::applets::CommonArguments),
            .library_applet_api_version = 1, // TODO: correct?
            .theme_color = 0,                // HACK
            .play_startup_sound = false,     // HACK
            .system_tick = system_tick,
        };
        controller.pushInData(new horizon::services::am::IStorage(common_args));

        // Config
        horizon::applets::software_keyboard::KeyboardConfigCommon config{
            .mode = horizon::applets::software_keyboard::KeyboardMode::Full,
            // TODO: more
        };
        controller.pushInData(new horizon::services::am::IStorage(config));

        break;
    }
    case 0x0100000000001009: { // miiEdit
        // Args
        horizon::applets::mii_edit::AppletInput args{
            ._unknown_x0 = 0x3,
            .mode = horizon::applets::mii_edit::AppletMode::ShowMiiEdit,
        };
        controller.pushInData(new horizon::services::am::IStorage(args));

        break;
    }
    case 0x010000000000100d: { // photoViewer
        // Common args
        horizon::applets::CommonArguments common_args{
            .version = 1,
            .size = sizeof(horizon::applets::CommonArguments),
            .library_applet_api_version = 1, // TODO: correct?
            .theme_color = 0,                // HACK
            .play_startup_sound = false,     // HACK
            .system_tick = system_tick,
        };
        controller.pushInData(new horizon::services::am::IStorage(common_args));

        // Arg
        auto arg = new horizon::applets::album::Arg{
            horizon::applets::album::Arg::ShowAllAlbumFilesForHomeMenu};
        controller.pushInData(new horizon::services::am::IStorage(arg));

        break;
    }
    default:
        break;
    }

    os.setLibraryAppletSelfController(std::move(controller));

    // Loading screen assets
    {
        std::unique_ptr<hw::tegra_x1::gpu::renderer::ICommandBuffer>
            command_buffer = nullptr;

        {
            u32 width;
            u32 height;
            if (auto data = loader->loadNintendoLogo(width, height)) {
                // Create texture
                const u32 stride = width * 4;
                const u32 size = height * stride;
                const hw::tegra_x1::gpu::renderer::TextureDescriptor descriptor(
                    0x0, hw::tegra_x1::gpu::renderer::TextureType::_2D,
                    hw::tegra_x1::gpu::renderer::TextureFormat::RGBA8Unorm,
                    true, stride, width, height, 1, 1, 1, 0x0, 0x0, 0x0);
                const auto texture =
                    gpu.getRenderer().createTexture(descriptor);

                const auto view_descriptor =
                    hw::tegra_x1::gpu::renderer::TextureViewDescriptor(
                        descriptor.type, descriptor.format,
                        ztd::Range<u32>(0, 1), ztd::Range<u32>(0, 1));
                const auto texture_view = texture->createView(view_descriptor);
                nintendo_logo = {.base = texture, .view = texture_view};

                // Command buffer
                command_buffer.reset(gpu.getRenderer().createCommandBuffer());

                // Copy data
                auto tmp_buffer =
                    gpu.getRenderer().allocateTemporaryBuffer(size);
                std::memcpy(reinterpret_cast<void*>(tmp_buffer->getPtr()), data,
                            size);
                // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
                free(data);
                texture->copyFrom(command_buffer.get(), tmp_buffer);
                gpu.getRenderer().freeTemporaryBuffer(tmp_buffer);
            }
        }
        {
            u32 width;
            u32 height;
            u32 frame_count;
            if (auto data = loader->loadStartupMovie(
                    startup_movie_delays, width, height, frame_count)) {
                const u32 stride = width * 4;
                const u32 size = height * stride;
                hw::tegra_x1::gpu::renderer::TextureDescriptor descriptor(
                    0x0, hw::tegra_x1::gpu::renderer::TextureType::_2D,
                    hw::tegra_x1::gpu::renderer::TextureFormat::RGBA8Unorm,
                    true, stride, width, height, 1, 1, 1, 0x0, 0x0, 0x0);
                const auto view_descriptor =
                    hw::tegra_x1::gpu::renderer::TextureViewDescriptor(
                        descriptor.type, descriptor.format,
                        ztd::Range<u32>(0, 1), ztd::Range<u32>(0, 1));
                startup_movie.reserve(frame_count);

                // Command buffer
                if (command_buffer == nullptr)
                    command_buffer.reset(
                        gpu.getRenderer().createCommandBuffer());

                for (u32 i = 0; i < frame_count; i++) {
                    // Create texture
                    const auto texture =
                        gpu.getRenderer().createTexture(descriptor);
                    const auto texture_view =
                        texture->createView(view_descriptor);

                    // Copy data
                    auto tmp_buffer =
                        gpu.getRenderer().allocateTemporaryBuffer(size);
                    std::memcpy(reinterpret_cast<void*>(tmp_buffer->getPtr()),
                                data + static_cast<usize>(i) * height * width,
                                size);
                    texture->copyFrom(command_buffer.get(), tmp_buffer);
                    gpu.getRenderer().freeTemporaryBuffer(tmp_buffer);
                    startup_movie.push_back(
                        {.base = texture, .view = texture_view});
                }
                // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
                free(data);

                // Extend the last frame's time
                startup_movie_delays.back() = 5s;
            }
        }
    }

    LOG_INFO(Other, "-------- Title info --------");
    LOG_INFO(Other, "Title ID: {:016x}", loader->getTitleId());

    // Patch
    const auto target_patch_filename =
        fmt::format("{:016x}.hatch", loader->getTitleId());
    // TODO: iterate recursively
    for (const auto& patch_path : CONFIG_INSTANCE.getPatchPaths()) {
        if (!std::filesystem::exists(patch_path)) {
            LOG_ERROR(Other, "Patch path does not exist: {}", patch_path);
            continue;
        }

        if (!std::filesystem::is_directory(patch_path)) {
            // File
            tryApplyPatch(main_process, target_patch_filename, patch_path);
        } else {
            // Directory
            // TODO: iterate recursively
            for (const auto& dir_entry :
                 std::filesystem::directory_iterator{patch_path}) {
                tryApplyPatch(main_process, target_patch_filename,
                              dir_entry.path().string());
            }
        }
    }

    LOG_INFO(Other, "-------- Config --------");
    CONFIG_INSTANCE.log();

    LOG_INFO(Other, "-------- Run --------");

    // Enter focus
    // HACK: games expect focus change to be the second message?
    main_process->getAppletState().sendMessage(
        horizon::kernel::AppletMessage::Resume);
    main_process->getAppletState().setFocusState(
        horizon::kernel::AppletFocusState::InFocus);

    // Preselected user
    auto user_id = CONFIG_INSTANCE.getUserId();
    if (user_id == horizon::services::account::internal::INVALID_USER_ID) {
        // If there is just a single user, use that
        if (os.getUserManager().getUserCount() == 1) {
            user_id = os.getUserManager().getUserIDs()[0];
        } else {
            // TODO: launch a select user applet in case the game requires it
            LOG_FATAL(Other, "Multiple user accounts");
        }
    }

    if (user_id != horizon::services::account::internal::INVALID_USER_ID) {
        main_process->getAppletState().pushPreselectedUser(user_id);
        LOG_INFO(Other, "Preselected user with ID {:032x}", user_id);
    }

    main_process->start();

    // Activate GDB server
    if (CONFIG_INSTANCE.getGdbEnabled()) {
        if (CONFIG_INSTANCE.getGdbWaitForClient())
            main_process->getMainThread()->supervisorPause();

        // HACK: spinlock until the main thread is running
        while (!main_process->isRunning())
            std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        DEBUGGER_MANAGER_INSTANCE.getDebugger(main_process)
            .activateGdbServer(*this);
    }

    // Loading screen
    loading = true;

    const auto crnt_time = clock_t::now();
    next_startup_movie_frame_time = crnt_time + STARTUP_MOVIE_FADE_IN_DURATION +
                                    STARTUP_MOVIE_BREAK_AFTER_FADE_IN_DURATION;
    startup_movie_fade_in_time = crnt_time + STARTUP_MOVIE_FADE_IN_DURATION;
}

void System::requestStop() {
    // We don't request the processes to stop yet, instead we send a message to
    // all of them and give them some time to react
    for (auto& it : os.getKernel().getProcessManager())
        it->getAppletState().sendMessage(horizon::kernel::AppletMessage::Exit);
}

void System::forceStop() {
    // Request all processes to stop immediately
    for (auto& it : os.getKernel().getProcessManager())
        it->stop();

    // Wait a small amount of time for all threads to catch up
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if all processes have stopped
    if (isRunning()) {
        // If some processes are still running, just abort
        LOG_FATAL(Other, "Failed to stop all processes");
    }
}

void System::pause() {
    for (auto& it : os.getKernel().getProcessManager())
        it->supervisorPause();
}

void System::resume() {
    for (auto& it : os.getKernel().getProcessManager())
        it->supervisorResume();
}

void System::progressFrame(u32 width, u32 height,
                           bool& out_dt_average_updated) {
    // Pump input events
    input_device_manager.pumpEvents();

    // Set the resolution for OS
    os.setSurfaceResolution({width, height});

    // Input
    os.getHidResourceManager().update();

    // Present

    // Acquire surface
    auto compositor = gpu.getRenderer().acquireNextSurface();
    if (compositor == nullptr)
        return;

    // Delta time
    {
        auto layer =
            os.getDisplayDriver().getFirstLayerForProcess(main_process);
        if (layer != nullptr)
            accumulated_dt += layer->getAccumulatedDt();
    }

    // Command buffer
    auto command_buffer = gpu.getRenderer().createCommandBuffer();

    // Acquire present textures
    bool acquired =
        os.getDisplayDriver().acquirePresentTextures(command_buffer);

    // Render pass
    os.getDisplayDriver().present(command_buffer, compositor, width, height);

    if (loading) {
        if (acquired) {
            // TODO: till when should the loading screen be shown?
            // Stop the loading screen on the first present
            loading = false;

            // Free loading assets
            if (nintendo_logo)
                nintendo_logo = std::nullopt;
            if (!startup_movie.empty()) {
                startup_movie.clear();
                startup_movie.shrink_to_fit();
                startup_movie_delays.clear();
                startup_movie_delays.shrink_to_fit();
            }
        } else {
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
                const auto tex = *nintendo_logo;
                int2 size = {
                    static_cast<i32>(tex.base->getDescriptor().width),
                    static_cast<i32>(tex.base->getDescriptor().height)};
                int2 dst_offset = {32, 32};
                compositor->drawTexture(
                    command_buffer, tex.view, IntRect2D({0, 0}, size),
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
                int2 size = {
                    static_cast<i32>(frame.base->getDescriptor().width),
                    static_cast<i32>(frame.base->getDescriptor().height)};
                int2 dst_offset = {static_cast<i32>(width) - size.x() - 32,
                                   static_cast<i32>(height) - size.y() - 32};
                compositor->drawTexture(
                    command_buffer, frame.view, IntRect2D({0, 0}, size),
                    IntRect2D(dst_offset, size), true, opacity);
            }
        }
    } else {
        // Delta time
        const auto now = clock_t::now();
        const auto time_since_last_dt_averaging = now - last_dt_averaging_time;
        if (time_since_last_dt_averaging > 1s) {
            if (static_cast<bool>(accumulated_dt))
                last_dt_average = static_cast<f32>(accumulated_dt);
            else
                last_dt_average = 0.f;
            accumulated_dt = {};
            last_dt_averaging_time = now;

            out_dt_average_updated = true;
        } else {
            out_dt_average_updated = false;
        }
    }

    compositor->present(command_buffer);

    delete command_buffer;
    delete compositor;

    // Signal V-Sync
    os.getDisplayDriver().signalVSync();
}

bool System::isRunning() const {
    if (main_process == nullptr)
        return false;

    switch (main_process->getState()) {
    case horizon::kernel::ProcessState::Started:
    case horizon::kernel::ProcessState::Exiting:
        return true;
    default:
        return false;
    };
}

void System::takeScreenshot() {
    auto layer = os.getDisplayDriver().getFirstLayerForProcess(main_process);
    if (layer == nullptr)
        return;

    ZTD_ASSIGN_OR_RETURN(auto texture, layer->getPresentTexture());

    std::thread thread([layer, texture, this] {
        // Get the image data
        auto rect = layer->getSrcRect();

        // Check if the image is flipped
        ASSERT(rect.size.x() > 0, Other, "Invalid width {}", rect.size.x());
        bool flip_y = false;
        if (rect.size.y() < 0) {
            rect.origin.y() += rect.size.y();
            rect.size.y() = -rect.size.y();
            flip_y = true;
        }

        // Copy to a buffer
        auto command_buffer = gpu.getRenderer().createCommandBuffer();
        auto buffer = gpu.getRenderer().allocateTemporaryBuffer(
            static_cast<u32>(rect.size.y() * rect.size.x() * 4));
        buffer->copyFrom(command_buffer, texture, rect.origin, rect.size,
                         ztd::Range<u32>(0, 1), ztd::Range<u32>(0, 1));
        delete command_buffer;

        // TODO: wait for the command buffer to finish

        // Save the image to file
        auto now = std::chrono::system_clock::now();
        // TODO: use title name in the filename
        std::string filename =
            fmt::format("{}/screenshot_{:%Y-%m-%d_%H-%M-%S}.jpg",
                        CONFIG_INSTANCE.getPicturesPath(), now);

        stbi_flip_vertically_on_write(flip_y);
        if (!stbi_write_jpg(filename.c_str(), rect.size.x(), rect.size.y(), 4,
                            reinterpret_cast<void*>(buffer->getPtr()), 100))
            LOG_ERROR(Other, "Failed to save screenshot to {}", filename);
        stbi_flip_vertically_on_write(false);

        // Free the buffer
        gpu.getRenderer().freeTemporaryBuffer(buffer);
    });
    thread.detach();
}

void System::captureGpuFrame() {
    // TODO: allow multiple frames
    gpu.getRenderer().captureFrames(1);
}

void System::tryApplyPatch(horizon::kernel::Process* process,
                           const std::string_view target_filename,
                           const std::filesystem::path& path) {
    if (toLower(path.filename().string()) != target_filename)
        return;

    LOG_INFO(Other, "Applying patch \"{}\"", path.string());

    std::ifstream ifs(path);

    // Deserialize
    Hatch::Deserializer deserializer;
    deserializer.Deserialize(ifs);

    const auto& hatch = deserializer.GetHatch();

    // Memory patch
    for (const auto& entry : hatch.GetMemoryPatch())
        process->getMmu()->write<u32>(entry.addr, entry.value);

    ifs.close();
}

} // namespace hydra
