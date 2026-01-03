#include "core/horizon/os.hpp"

#include "core/horizon/filesystem/content_archive.hpp"
#include "core/horizon/filesystem/disk_file.hpp"
#include "core/horizon/firmware.hpp"
#include "core/horizon/kernel/hipc/client_port.hpp"
#include "core/horizon/kernel/hipc/port.hpp"
#include "core/horizon/kernel/hipc/server_port.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/horizon/loader/nsp_loader.hpp"
#include "core/horizon/services/account/account_service_for_administrator.hpp"
#include "core/horizon/services/account/account_service_for_application.hpp"
#include "core/horizon/services/account/account_service_for_system_service.hpp"
#include "core/horizon/services/account/baas_access_token_accessor.hpp"
#include "core/horizon/services/am/all_system_applet_proxies_service.hpp"
#include "core/horizon/services/am/apm_manager.hpp"
#include "core/horizon/services/am/application_proxy_service.hpp"
#include "core/horizon/services/aocsrv/add_on_content_manager.hpp"
#include "core/horizon/services/apm/manager_privileged.hpp"
#include "core/horizon/services/audio/audio_out_manager.hpp"
#include "core/horizon/services/audio/audio_renderer_manager.hpp"
#include "core/horizon/services/codec/hardware_opus_decoder_manager.hpp"
#include "core/horizon/services/err/context/writer_for_application.hpp"
#include "core/horizon/services/fatalsrv/service.hpp"
#include "core/horizon/services/friends/service_creator.hpp"
#include "core/horizon/services/fssrv/filesystem_proxy.hpp"
#include "core/horizon/services/hid/hid_debug_server.hpp"
#include "core/horizon/services/hid/hid_server.hpp"
#include "core/horizon/services/hid/hid_system_server.hpp"
#include "core/horizon/services/hosbinder/hos_binder_driver.hpp"
#include "core/horizon/services/i2c/manager.hpp"
#include "core/horizon/services/ins/receiver_manager.hpp"
#include "core/horizon/services/irsensor/ir_sensor_server.hpp"
#include "core/horizon/services/lbl/lbl_controller.hpp"
#include "core/horizon/services/lm/log_service.hpp"
#include "core/horizon/services/mii/static_service.hpp"
#include "core/horizon/services/mmnv/request.hpp"
#include "core/horizon/services/ncm/content_manager.hpp"
#include "core/horizon/services/nfc/user_manager.hpp"
#include "core/horizon/services/nfp/user_manager.hpp"
#include "core/horizon/services/nifm/static_service.hpp"
#include "core/horizon/services/npns/npns_system.hpp"
#include "core/horizon/services/ns/application_manager_interface.hpp"
#include "core/horizon/services/ns/service_getter_interface.hpp"
#include "core/horizon/services/nsd/manager.hpp"
#include "core/horizon/services/nvdrv/nvdrv_services.hpp"
#include "core/horizon/services/omm/operation_mode_manager.hpp"
#include "core/horizon/services/ovln/sender_service.hpp"
#include "core/horizon/services/pctl/parental_control_service_factory.hpp"
#include "core/horizon/services/pcv/pcv_service.hpp"
#include "core/horizon/services/pdm/query_service.hpp"
#include "core/horizon/services/pl/detail/platform_service_manager_for_system.hpp"
#include "core/horizon/services/pl/sharedresource/platform_shared_resource_manager.hpp"
#include "core/horizon/services/pm/boot_mode_interface.hpp"
#include "core/horizon/services/pm/debug_monitor_interface.hpp"
#include "core/horizon/services/pm/information_interface.hpp"
#include "core/horizon/services/prepo/prepo_service.hpp"
#include "core/horizon/services/psc/pm_service.hpp"
#include "core/horizon/services/psm/psm_server.hpp"
#include "core/horizon/services/ro/detail/ro_interface.hpp"
#include "core/horizon/services/settings/factory_settings_server.hpp"
#include "core/horizon/services/settings/settings_server.hpp"
#include "core/horizon/services/settings/system_settings_server.hpp"
#include "core/horizon/services/sm/user_interface.hpp"
#include "core/horizon/services/socket/client.hpp"
#include "core/horizon/services/socket/resolver/resolver.hpp"
#include "core/horizon/services/spl/general_interface.hpp"
#include "core/horizon/services/spl/random_interface.hpp"
#include "core/horizon/services/spsm/power_state_interface.hpp"
#include "core/horizon/services/ssl/sf/ssl_service.hpp"
#include "core/horizon/services/timesrv/static_service.hpp"
#include "core/horizon/services/ts/measurement_server.hpp"
#include "core/horizon/services/usb/hs/client_root_session.hpp"
#include "core/horizon/services/visrv/application_root_service.hpp"
#include "core/horizon/services/visrv/manager_root_service.hpp"
#include "core/horizon/services/visrv/system_root_service.hpp"
#include "core/input/device_manager.hpp"

namespace hydra::horizon {

namespace {

template <typename Key>
void RegisterServiceToPort(services::Server* server,
                           kernel::hipc::ServiceManager<Key>& service_manager,
                           const Key& port_name,
                           services::create_service_fn_t service_creator) {
    // Debug
    std::string debug_name;
    if constexpr (std::is_same_v<Key, std::string>)
        debug_name = port_name;
    else
        debug_name = u64_to_str(port_name);

    // Session
    auto server_port = new kernel::hipc::ServerPort(
        fmt::format("\"{}\" server port", debug_name));
    auto client_port = new kernel::hipc::ClientPort(
        fmt::format("\"{}\" client port", debug_name));
    new kernel::hipc::Port(server_port, client_port,
                           fmt::format("\"{}\" port", debug_name));

    // Register server side
    server->RegisterPort(server_port, service_creator);

    // Register client side
    service_manager.RegisterPort(port_name, client_port);
}

uint2 round_up_to_nearest_standard_resolution(uint2 surface_resolution) {
    // TODO: constexpr
    static uint2 standard_resolutions[] = {
        {1280, 720}, {1920, 1080}, {2560, 1440}, {3840, 2160}, {7680, 4320}};
    for (u32 i = 0; i < sizeof_array(standard_resolutions); i++) {
        const auto& resolution = standard_resolutions[i];
        if (surface_resolution.x() <= resolution.x() &&
            surface_resolution.y() <= resolution.y())
            return resolution;
    }

    return standard_resolutions[sizeof_array(standard_resolutions) - 1];
}

} // namespace

SINGLETON_DEFINE_GET_INSTANCE(OS, Horizon)

OS::OS(audio::ICore& audio_core_, ui::HandlerBase& ui_handler_)
    : audio_core{audio_core_}, ui_handler{ui_handler_} {
    SINGLETON_SET_INSTANCE(OS, Horizon);

    // Firmware
    try_install_firmware_to_filesystem(kernel.GetFilesystem());

    // Sysmodules
    const auto& sysmodules_path = CONFIG_INSTANCE.GetSysmodulesPath();
    if (std::filesystem::exists(sysmodules_path)) {
        auto res = KERNEL_INSTANCE.GetFilesystem().AddEntry(
            FS_SYSMODULES_PATH, sysmodules_path, true);
        ASSERT(res == horizon::filesystem::FsResult::Success, Other,
               "Failed to add sysmodules", res);
    }

    // System avatars
    user_manager.LoadSystemAvatars(kernel.GetFilesystem());

    // Shared font
    shared_font_manager.LoadFonts();

    // Connect npads
    INPUT_DEVICE_MANAGER_INSTANCE.ConnectNpads();

    // Services

    // Only some services have dedicated servers so as to avoid creating
    // unnecessary processes and threads
    // TODO: adjust the number of servers based on host CPU core count

    // SM
    RegisterServiceToPort<std::string>(
        &others_server, kernel.GetServiceManager(), "sm:", [this]() {
            auto s = new services::sm::IUserInterface();
            s->SetServer(&others_server);
            return s;
        });

#define REGISTER_SERVICE_CASE(server, service, name)                           \
    RegisterServiceToPort(server, service_manager, name##_u64, [this]() {      \
        auto s = new services::service();                                      \
        s->SetServer(server);                                                  \
        return s;                                                              \
    });
#define REGISTER_SERVICE(server_name, service, ...)                            \
    FOR_EACH_2_1(REGISTER_SERVICE_CASE, &server_name##_server, service,        \
                 __VA_ARGS__)

    // HID
    REGISTER_SERVICE(others, hid::IHidServer, "hid");
    REGISTER_SERVICE(others, hid::IHidDebugServer, "hid:dbg");
    REGISTER_SERVICE(others, hid::IHidSystemServer, "hid:sys");
    REGISTER_SERVICE(others, irsensor::IIrSensorServer, "irs");

    // AM
    REGISTER_SERVICE(others, am::IApmManager, "apm", "apm:am");
    REGISTER_SERVICE(others, am::IApplicationProxyService, "appletOE");
    REGISTER_SERVICE(others, am::IAllSystemAppletProxiesService, "appletAE");
    REGISTER_SERVICE(others, spsm::IPowerStateInterface, "spsm");

    // NS
    REGISTER_SERVICE(others, ns::IApplicationManagerInterface, "ns:am");
    REGISTER_SERVICE(others, ns::IServiceGetterInterface, "ns:am2", "ns:ec",
                     "ns:rid", "ns:rt", "ns:web", "ns:ro", "ns:sweb");
    REGISTER_SERVICE(others, aocsrv::IAddOnContentManager, "aoc:u");
    REGISTER_SERVICE(others, account::IAccountServiceForApplication, "acc:u0");
    REGISTER_SERVICE(others, account::IAccountServiceForSystemService,
                     "acc:u1");
    REGISTER_SERVICE(others, account::IAccountServiceForAdministrator,
                     "acc:su");
    REGISTER_SERVICE(others, account::IBaasAccessTokenAccessor, "acc:aa");
    REGISTER_SERVICE(others, pdm::IQueryService, "pdm:qry");

    // PPC
    REGISTER_SERVICE(others, apm::IManagerPrivileged, "apm:p");

    // Glue
    REGISTER_SERVICE(others, timesrv::IStaticService, "time:u", "time:a",
                     "time:s");
    REGISTER_SERVICE(others, pl::sharedresource::IPlatformSharedResourceManager,
                     "pl:u");
    REGISTER_SERVICE(others, err::context::IWriterForApplication, "ectx:aw");

    // Audio
    REGISTER_SERVICE(others, audio::IAudioOutManager, "audout:u");
    REGISTER_SERVICE(others, audio::IAudioRendererManager, "audren:u");
    REGISTER_SERVICE(others, codec::IHardwareOpusDecoderManager, "hwopus");

    // Bcat
    REGISTER_SERVICE(others, prepo::IPrepoService, "prepo:a", "prepo:a2",
                     "prepo:m", "prepo:u", "prepo:s");

    // PCV
    REGISTER_SERVICE(others, pcv::IPcvService, "pcv");

    // Socket
    REGISTER_SERVICE(others, socket::IClient, "bsd:u", "bsd:s", "bsd:a");
    REGISTER_SERVICE(others, nsd::IManager, "nsd:u", "nsd:a");
    REGISTER_SERVICE(others, socket::Resolver::IResolver, "sfdnsres");

    // Capsrv
    REGISTER_SERVICE(others, mmnv::IRequest, "mm:u");

    // VI
    REGISTER_SERVICE(others, visrv::IApplicationRootService, "vi:u");
    REGISTER_SERVICE(others, visrv::ISystemRootService, "vi:s");
    REGISTER_SERVICE(others, visrv::IManagerRootService, "vi:m");
    REGISTER_SERVICE(others, lbl::ILblController, "lbl");

    // Nvnflinger
    REGISTER_SERVICE(others, hosbinder::IHOSBinderDriver, "dispdrv");

    // PTM
    REGISTER_SERVICE(others, psm::IPsmServer, "psm");
    REGISTER_SERVICE(others, ts::IMeasurementServer, "ts");

    // Fatal
    REGISTER_SERVICE(others, fatalsrv::IService, "fatal:u");

    // Friends
    REGISTER_SERVICE(others, friends::IServiceCreator, "friend:u", "friend:v",
                     "friend:m", "friend:s", "friend:a");

    // FS
    REGISTER_SERVICE(others, fssrv::IFileSystemProxy, "fsp-srv");

    // Bus
    REGISTER_SERVICE(others, i2c::IManager, "i2c", "i2c:pcv");

    // NFC
    REGISTER_SERVICE(others, nfc::IUserManager, "nfc:user");
    REGISTER_SERVICE(others, nfp::IUserManager, "nfp:user");

    // NIFM
    REGISTER_SERVICE(others, nifm::IStaticService, "nifm:a", "nifm:s",
                     "nifm:u");

    // NPNS
    REGISTER_SERVICE(others, npns::INpnsSystem, "npns:s");

    // Nvservices
    REGISTER_SERVICE(nvservices, nvdrv::INvDrvServices, "nvdrv", "nvdrv:a",
                     "nvdrv:s", "nvdrv:t");
    nvservices_server.Start();

    // PM
    REGISTER_SERVICE(others, pm::IBootModeInterface, "pm:bm");
    REGISTER_SERVICE(others, pm::IInformationInterface, "pm:info");
    REGISTER_SERVICE(others, pm::IDebugMonitorInterface, "pm:dmnt");

    // PCTL
    REGISTER_SERVICE(others, pctl::IParentalControlServiceFactory, "pctl:s",
                     "pctl:r", "pctl:a", "pctl");

    // PSC
    REGISTER_SERVICE(others, psc::IPmService, "psc:m");
    REGISTER_SERVICE(others, ovln::ISenderService, "ovln:snd");
    REGISTER_SERVICE(others, ins::IReceiverManager, "ins:r");

    // Settings
    REGISTER_SERVICE(others, settings::ISettingsServer, "set");
    REGISTER_SERVICE(others, settings::ISystemSettingsServer, "set:sys");
    REGISTER_SERVICE(others, settings::IFactorySettingsServer, "set:cal");

    // Ssl
    REGISTER_SERVICE(others, ssl::sf::ISslService, "ssl");

    // Spl
    REGISTER_SERVICE(others, spl::IRandomInterface, "csrng");
    REGISTER_SERVICE(others, spl::IGeneralInterface, "spl:");

    // Ncm
    REGISTER_SERVICE(others, ncm::IContentManager, "ncm");

    // Omm
    REGISTER_SERVICE(others, omm::IOperationModeManager, "omm");

    // Usb
    REGISTER_SERVICE(others, usb::hs::IClientRootSession, "usb:hs");

    // Shared database
    REGISTER_SERVICE(others, pl::detail::IPlatformServiceManagerForSystem,
                     "pl:s");

    // RO
    REGISTER_SERVICE(others, ro::detail::IRoInterface, "ldr:ro", "ro:1");

    // Unknown
    REGISTER_SERVICE(others, lm::ILogService, "lm");
    REGISTER_SERVICE(others, mii::IStaticService, "mii:u", "mii:e");

    others_server.Start();

    // Sysmodules
    if (std::filesystem::exists(sysmodules_path)) {
        for (const auto& entry :
             std::filesystem::directory_iterator(sysmodules_path)) {
            // Load the sysmodule
            auto exefs_path =
                fmt::format("{}/exefs.nsp", entry.path().string());
            if (!std::filesystem::exists(exefs_path))
                continue;

            filesystem::DiskFile file(exefs_path);
            loader::NspLoader loader(&file);

            // Start a new process
            // TODO: get the name from toolbox.json
            // TODO: get title ID from toolbox.json
            auto process =
                kernel.GetProcessManager().CreateProcess("Sysmodule");
            loader.LoadProcess(process);
            process->Start();
        }
    }
}

OS::~OS() { SINGLETON_UNSET_INSTANCE(); }

void OS::NotifyOperationModeChanged() {
    // Disconnect and connect npads
    input_manager.DisconnectAllNpads();
    INPUT_DEVICE_MANAGER_INSTANCE.ConnectNpads();

    // Send a message to all processes
    for (auto it = kernel.GetProcessManager().Begin();
         it != kernel.GetProcessManager().End(); it++) {
        (*it)->GetAppletState().SendMessage(
            kernel::AppletMessage::OperationModeChanged);
    }
}

void OS::SetSurfaceResolution(uint2 resolution) {
    // TODO: signal resolution change event if changed
    surface_resolution = resolution;
}

uint2 OS::GetDisplayResolution() const {
    if (CONFIG_INSTANCE.GetHandheldMode()) {
        return {1280, 720}; // Handheld display resolution is fixed
    } else {
        switch (CONFIG_INSTANCE.GetDisplayResolution()) {
        case Resolution::Auto:
            return round_up_to_nearest_standard_resolution(surface_resolution);
        case Resolution::_720p:
            return {1280, 720};
        case Resolution::_1080p:
            return {1920, 1080};
        case Resolution::_1440p:
            return {2560, 1440};
        case Resolution::_2160p:
            return {3840, 2160};
        case Resolution::_4320p:
            return {7680, 4320};
        case Resolution::AutoExact:
            return surface_resolution;
        case Resolution::Custom:
            return CONFIG_INSTANCE.GetCustomDisplayResolution();
        default:
            unreachable();
        }
    }
}

} // namespace hydra::horizon
