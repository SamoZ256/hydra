#include "core/emulation_context.hpp"

#include "hatch/hatch.hpp"

#include "core/audio/cubeb/core.hpp"
#include "core/audio/null/core.hpp"
#include "core/horizon/applets/album/const.hpp"
#include "core/horizon/applets/const.hpp"
#include "core/horizon/applets/controller/const.hpp"
#include "core/horizon/applets/err/const.hpp"
#include "core/horizon/applets/mii/const.hpp"
#include "core/horizon/applets/swkbd/const.hpp"
#include "core/horizon/filesystem/host_file.hpp"
#include "core/horizon/loader/nca_loader.hpp"
#include "core/horizon/loader/nro_loader.hpp"
#include "core/horizon/loader/nso_loader.hpp"
#include "core/horizon/services/am/library_applet_controller.hpp"
#include "core/hw/tegra_x1/cpu/dynarmic/cpu.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/cpu.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_base.hpp"
#include "core/input/device_manager.hpp"

using namespace std::chrono_literals;

namespace hydra {

namespace {

constexpr auto STARTUP_MOVIE_FADE_IN_DURATION = 100ms;
constexpr auto STARTUP_MOVIE_BREAK_AFTER_FADE_IN_DURATION = 200ms;

} // namespace

EmulationContext::EmulationContext(horizon::ui::HandlerBase& ui_handler) {
    LOGGER_INSTANCE.SetOutput(CONFIG_INSTANCE.GetLogOutput());

    // Random
    srand(time(0));

    // Initialize
    switch (CONFIG_INSTANCE.GetCpuBackend()) {
    case CpuBackend::AppleHypervisor:
        cpu = new hw::tegra_x1::cpu::hypervisor::Cpu();
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
        audio_core = new audio::cubeb::Core();
        break;
    default:
        // TODO: return an error instead
        LOG_FATAL(Other, "Unknown audio backend {}",
                  CONFIG_INSTANCE.GetAudioBackend());
        break;
    }

    os = new horizon::OS(*audio_core, ui_handler);
    os->GetDisplayDriver().CreateDisplay();

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

    // Content

    FILESYSTEM_INSTANCE.Mount(FS_CONTENT_MOUNT);

    // Firmware
    std::map<u64, std::string> firmware_titles_map = {
        {0x010000000000080E, "TimeZoneBinary"},
        {0x0100000000000810, "FontNintendoExtension"},
        {0x0100000000000811, "FontStandard"},
        {0x0100000000000812, "FontKorean"},
        {0x0100000000000813, "FontChineseTraditional"},
        {0x0100000000000814, "FontChineseSimple"},
    };

    const auto& firmware_path = CONFIG_INSTANCE.GetFirmwarePath().Get();
    if (std::filesystem::exists(firmware_path)) {
        // Iterate over the directory
        for (const auto& entry :
             std::filesystem::directory_iterator(firmware_path)) {
            auto file =
                new horizon::filesystem::HostFile(entry.path().string());
            horizon::filesystem::ContentArchive content_archive(file);
            // TODO: find a better way to handle this
            if (content_archive.GetContentType() ==
                horizon::filesystem::ContentArchiveContentType::Meta)
                continue;

            auto it = firmware_titles_map.find(content_archive.GetTitleID());
            if (it == firmware_titles_map.end())
                continue;

            auto res = FILESYSTEM_INSTANCE.AddEntry(
                fmt::format(FS_FIRMWARE_PATH "/{}", it->second), file, true);
            ASSERT(res == horizon::filesystem::FsResult::Success, Other,
                   "Failed to add firmware entry: {}", res);
        }
    } else {
        LOG_ERROR(Other, "Firmware path does not exist");
    }
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
    // TODO: make sure the process is destroyed
    auto process =
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
        horizon::applets::controller::ControllerSupportArg0 arg0{
            .player_count_min = 0,
            .player_count_max = 2,
            .enable_take_over_connection = true,
            .enable_left_justify = false,
            .enable_permit_joy_dual = true,
            .enable_single_mode = true,
            .enable_identification_color = false,
        };

        horizon::applets::controller::ControllerSupportArg1 arg1{
            .enable_explain_text = false,
        };

        // Private arg
        auto private_arg =
            new horizon::applets::controller::ControllerSupportArgPrivate{
                .size = sizeof(
                    horizon::applets::controller::ControllerSupportArgPrivate),
                .controller_support_arg_size = sizeof(arg0) + sizeof(arg1),
                .flag0 = 0,
                .flag1 = 0,
                .mode = horizon::applets::controller::ControllerSupportMode::
                    ShowControllerSupport,
                .caller = horizon::applets::controller::
                    ControllerSupportCaller::Application,
                .npad_style_set = horizon::hid::NpadStyleSet::JoyDual,
                .npad_joy_hold_type = horizon::hid::NpadJoyHoldType::Vertical,
            };
        controller->PushInData(
            new horizon::services::am::IStorage(private_arg));

        auto arg = (u8*)malloc(sizeof(arg0) + sizeof(arg1));
        memcpy(arg, &arg0, sizeof(arg0));
        memcpy(arg + sizeof(arg0), &arg1, sizeof(arg1));
        controller->PushInData(new horizon::services::am::IStorage(arg));

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
        auto param_common = new horizon::applets::err::ParamCommon{
            .type = horizon::applets::err::ErrorType::ApplicationError,
            .is_jump_enabled = false,
        };
        controller->PushInData(
            new horizon::services::am::IStorage(param_common));

        // Param for application error
        auto param_for_application_error =
            new horizon::applets::err::ParamForApplicationError{
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
        auto config = new horizon::applets::swkbd::KeyboardConfigCommon{
            .mode = horizon::applets::swkbd::KeyboardMode::Full,
            // TODO: more
        };
        controller->PushInData(new horizon::services::am::IStorage(config));

        break;
    }
    case 0x0100000000001009: { // miiEdit
        // Args
        auto args = new horizon::applets::mii::AppletInput{
            ._unknown_x0 = 0x3,
            .mode = horizon::applets::mii::AppletMode::ShowMiiEdit,
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
        uchar4* data = nullptr;
        usize width, height;
        loader->LoadNintendoLogo(data, width, height);
        if (data) {
            hw::tegra_x1::gpu::renderer::TextureDescriptor descriptor(
                0x0, hw::tegra_x1::gpu::renderer::TextureFormat::RGBA8Unorm,
                hw::tegra_x1::gpu::NvKind::Generic_16BX2, width, height, 0x0,
                width * 4);
            nintendo_logo = gpu->GetRenderer()->CreateTexture(descriptor);
            nintendo_logo->CopyFrom(reinterpret_cast<uptr>(data));
            free(data);
        }
    }
    {
        uchar4* data = nullptr;
        usize width, height;
        u32 frame_count;
        loader->LoadStartupMovie(data, startup_movie_delays, width, height,
                                 frame_count);
        if (data) {
            hw::tegra_x1::gpu::renderer::TextureDescriptor descriptor(
                0x0, hw::tegra_x1::gpu::renderer::TextureFormat::RGBA8Unorm,
                hw::tegra_x1::gpu::NvKind::Generic_16BX2, width, height, 0x0,
                width * 4);
            startup_movie.reserve(frame_count);
            for (u32 i = 0; i < frame_count; i++) {
                auto frame = gpu->GetRenderer()->CreateTexture(descriptor);
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

    // Connect input devices
    INPUT_DEVICE_MANAGER_INSTANCE.ConnectDevices();

    // Enter focus
    // HACK: games expect focus change to be the second message?
    process->GetAppletState().SendMessage(
        horizon::kernel::AppletMessage::Resume);
    process->GetAppletState().SetFocusState(
        horizon::kernel::AppletFocusState::InFocus);

    // Preselected user
    auto user_id = CONFIG_INSTANCE.GetUserID().Get();
    if (user_id == horizon::services::account::INVALID_USER_ID) {
        // If there is just a single user, use that
        if (USER_MANAGER_INSTANCE.GetCount() == 1) {
            user_id = *USER_MANAGER_INSTANCE.Begin();
        } else {
            // TODO: launch a select user applet in case the game requires it
            LOG_FATAL(Other, "Multiple user accounts");
        }
    }

    if (user_id != horizon::services::account::INVALID_USER_ID) {
        process->GetAppletState().PushPreselectedUser(user_id);
        LOG_INFO(Other, "Preselected user with ID {:032x}", user_id);
    }

    process->Start();

    loading = true;

    const auto crnt_time = clock_t::now();
    next_startup_movie_frame_time = crnt_time + STARTUP_MOVIE_FADE_IN_DURATION +
                                    STARTUP_MOVIE_BREAK_AFTER_FADE_IN_DURATION;
    startup_movie_fade_in_time = crnt_time + STARTUP_MOVIE_FADE_IN_DURATION;
}

void EmulationContext::RequestStop() {
    // We don't request the processes to stop yet, instead we send a message to
    // all of them and give them some time to react
    // TODO: send an exit message to all processes
    for (auto it = os->GetKernel().GetProcessManager().Begin();
         it != os->GetKernel().GetProcessManager().End(); ++it)
        (*it)->GetAppletState().SendMessage(
            horizon::kernel::AppletMessage::Exit);
}

void EmulationContext::ForceStop() {
    // Request all processes to stop immediately
    for (auto it = os->GetKernel().GetProcessManager().Begin();
         it != os->GetKernel().GetProcessManager().End(); ++it)
        (*it)->RequestStop();

    // Wait a small amount of time for all threads to catch up
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if all processes have stopped
    if (IsRunning()) {
        // If some processes are still running, just abort
        LOG_FATAL(Other, "Failed to stop process all processes");
    }
}

void EmulationContext::ProgressFrame(u32 width, u32 height,
                                     bool& out_dt_average_updated) {
    // Input
    INPUT_DEVICE_MANAGER_INSTANCE.Poll();

    // Present
    std::vector<std::chrono::nanoseconds> dt_list;
    if (Present(width, height, dt_list) && loading) {
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

        auto renderer = gpu->GetRenderer();
        renderer->LockMutex();
        if (!renderer->AcquireNextSurface()) {
            renderer->UnlockMutex();
            return;
        }

        // Nintendo logo
        if (nintendo_logo) {
            int2 size = {(i32)nintendo_logo->GetDescriptor().width,
                         (i32)nintendo_logo->GetDescriptor().height};
            int2 dst_offset = {32, 32};
            renderer->DrawTextureToSurface(nintendo_logo, {{0, 0}, size},
                                           {dst_offset, size}, true, opacity);
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
            renderer->DrawTextureToSurface(frame, {{0, 0}, size},
                                           {dst_offset, size}, true, opacity);
        }

        renderer->PresentSurface();
        renderer->EndCommandBuffer();
        renderer->UnlockMutex();
        return;
    }

    // Delta time
    for (const auto dt : dt_list) {
        accumulated_dt += dt;
        dt_sample_count++;
    }

    const auto now = clock_t::now();
    const auto time_since_last_dt_averaging = now - last_dt_averaging_time;
    if (time_since_last_dt_averaging > 1s) {
        if (dt_sample_count != 0)
            last_dt_average =
                (f32)std::chrono::duration_cast<std::chrono::duration<f32>>(
                    accumulated_dt)
                    .count() /
                (f32)dt_sample_count;
        else
            last_dt_average = 0.f;
        accumulated_dt = 0ns;
        dt_sample_count = 0;
        last_dt_averaging_time = now;

        out_dt_average_updated = true;
    } else {
        out_dt_average_updated = false;
    }
}

// TODO: rework this to support multiple layers and multiple processes
bool EmulationContext::Present(
    u32 width, u32 height, std::vector<std::chrono::nanoseconds>& out_dt_list) {
    // TODO: don't hardcode the display id
    auto& display = os->GetDisplayDriver().GetDisplay(0);
    std::unique_lock display_lock(display.GetMutex());

    // Layer
    auto layer = display.GetPresentableLayer();
    if (!layer)
        return false;

    // Signal V-Sync
    display.GetVSyncEvent()->Signal();

    // Get the buffer to present
    u32 binder_id = layer->GetBinderID();
    auto& binder = os->GetDisplayDriver().GetBinder(binder_id);

    horizon::display::BqBufferInput input;
    i32 slot = binder.ConsumeBuffer(input, out_dt_list);
    if (slot == -1)
        return false;
    const auto& buffer = binder.GetBuffer(slot);

    // Src rect
    IntRect2D src_rect;
    src_rect.origin.x() = input.rect.left;
    src_rect.origin.y() =
        input.rect.top; // Convert from top left to bottom left origin
    src_rect.size.x() = input.rect.right - input.rect.left;
    src_rect.size.y() = input.rect.bottom - input.rect.top;

    // HACK
    if (src_rect.size.x() == 0) {
        src_rect.size.x() = buffer.nv_buffer.planes[0].width;
        ONCE(LOG_WARN(Other, "Invalid src width"));
    }
    if (src_rect.size.y() == 0) {
        src_rect.size.y() = buffer.nv_buffer.planes[0].height;
        ONCE(LOG_WARN(Other, "Invalid src height"));
    }

    if (any(input.transform_flags & horizon::display::TransformFlags::FlipH)) {
        src_rect.origin.x() += src_rect.size.x();
        src_rect.size.x() = -src_rect.size.x();
    }
    if (any(input.transform_flags & horizon::display::TransformFlags::FlipV)) {
        src_rect.origin.y() += src_rect.size.y();
        src_rect.size.y() = -src_rect.size.y();
    }
    if (any(input.transform_flags & horizon::display::TransformFlags::Rot90)) {
        // TODO: how does this work? Is the aspect ratio kept intact?
        ONCE(LOG_NOT_IMPLEMENTED(Other, "Rotating by 90 degrees"));
    }

    // Dst rect
    const auto src_width = abs(src_rect.size.x());
    const auto src_height = abs(src_rect.size.y());

    IntRect2D dst_rect;
    auto scale_x = (f32)width / (f32)src_width;
    auto scale_y = (f32)height / (f32)src_height;
    if (scale_x > scale_y) {
        const auto dst_width = static_cast<i32>(src_width * scale_y);
        dst_rect.origin = int2({static_cast<i32>(width - dst_width) / 2, 0});
        dst_rect.size = int2({dst_width, static_cast<i32>(height)});
    } else {
        const auto dst_height = static_cast<i32>(src_height * scale_x);
        dst_rect.origin = int2({0, static_cast<i32>(height - dst_height) / 2});
        dst_rect.size = int2({static_cast<i32>(width), dst_height});
    }

    // Present
    auto renderer = gpu->GetRenderer();

    renderer->LockMutex();
    auto texture = gpu->GetTexture(
        (*os->GetKernel().GetProcessManager().Begin())->GetMmu(),
        buffer.nv_buffer); // HACK
    if (!renderer->AcquireNextSurface()) {
        renderer->UnlockMutex();
        return false;
    }

    renderer->DrawTextureToSurface(texture, src_rect, dst_rect);

    renderer->PresentSurface();
    renderer->EndCommandBuffer();
    renderer->UnlockMutex();
    return true;
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
        process->GetMmu()->Store<u32>(entry.addr, entry.value);

    ifs.close();
}

} // namespace hydra
