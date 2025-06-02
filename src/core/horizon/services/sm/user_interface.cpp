#include "core/horizon/services/sm/user_interface.hpp"

#include "core/horizon/services/account/account_service_for_application.hpp"
#include "core/horizon/services/account/account_service_for_system_service.hpp"
#include "core/horizon/services/am/apm_manager.hpp"
#include "core/horizon/services/am/application_proxy_service.hpp"
#include "core/horizon/services/aocsrv/add_on_content_manager.hpp"
#include "core/horizon/services/apm/manager_privileged.hpp"
#include "core/horizon/services/audio/audio_out_manager.hpp"
#include "core/horizon/services/audio/audio_renderer_manager.hpp"
#include "core/horizon/services/codec/hardware_opus_decoder_manager.hpp"
#include "core/horizon/services/friends/service_creator.hpp"
#include "core/horizon/services/fssrv/filesystem_proxy.hpp"
#include "core/horizon/services/hid/hid_debug_server.hpp"
#include "core/horizon/services/hid/hid_server.hpp"
#include "core/horizon/services/hid/hid_system_server.hpp"
#include "core/horizon/services/lm/log_service.hpp"
#include "core/horizon/services/nfc/user_manager.hpp"
#include "core/horizon/services/nifm/static_service.hpp"
#include "core/horizon/services/nvdrv/nvdrv_services.hpp"
#include "core/horizon/services/pctl/parental_control_service_factory.hpp"
#include "core/horizon/services/pcv/pcv_service.hpp"
#include "core/horizon/services/pl/sharedresource/platform_shared_resource_manager.hpp"
#include "core/horizon/services/prepo/prepo_service.hpp"
#include "core/horizon/services/psm/psm_server.hpp"
#include "core/horizon/services/settings/settings_server.hpp"
#include "core/horizon/services/settings/system_settings_server.hpp"
#include "core/horizon/services/socket/client.hpp"
#include "core/horizon/services/socket/resolver/resolver.hpp"
#include "core/horizon/services/spl/general_interface.hpp"
#include "core/horizon/services/spl/random_interface.hpp"
#include "core/horizon/services/ssl/sf/ssl_service.hpp"
#include "core/horizon/services/timesrv/static_service.hpp"
#include "core/horizon/services/ts/measurement_server.hpp"
#include "core/horizon/services/visrv/application_root_service.hpp"
#include "core/horizon/services/visrv/manager_root_service.hpp"
#include "core/horizon/services/visrv/system_root_service.hpp"

namespace hydra::horizon::services::sm {

DEFINE_SERVICE_COMMAND_TABLE(IUserInterface, 0, RegisterProcess, 1,
                             GetServiceHandle)

result_t IUserInterface::GetServiceHandle(add_service_fn_t add_service,
                                          u64 name) {
    LOG_DEBUG(Services, "Service name: \"{}\"", u64_to_str(name));

#define SERVICE_CASE_CASE(str) case str_to_u64(str):
// TODO: don't instantiate the services?
#define SERVICE_CASE(name, ...)                                                \
    FOR_EACH_0_1(SERVICE_CASE_CASE, __VA_ARGS__)                               \
    add_service(new name());                                                   \
    break;

    switch (name) {
        SERVICE_CASE(hid::IHidServer, "hid")
        SERVICE_CASE(hid::IHidDebugServer, "hid:dbg")
        SERVICE_CASE(hid::IHidSystemServer, "hid:sys")
        SERVICE_CASE(fssrv::IFileSystemProxy, "fsp-srv")
        SERVICE_CASE(timesrv::IStaticService, "time:u", "time:a", "time:s")
        SERVICE_CASE(nvdrv::INvDrvServices, "nvdrv")
        SERVICE_CASE(settings::ISystemSettingsServer, "set:sys")
        SERVICE_CASE(am::IApmManager, "apm", "apm:am")
        SERVICE_CASE(am::IApplicationProxyService, "appletOE")
        SERVICE_CASE(visrv::IApplicationRootService, "vi:u")
        SERVICE_CASE(visrv::ISystemRootService, "vi:s")
        SERVICE_CASE(visrv::IManagerRootService, "vi:m")
        SERVICE_CASE(pl::shared_resource::IPlatformSharedResourceManager,
                     "pl:u")
        SERVICE_CASE(psm::IPsmServer, "psm")
        SERVICE_CASE(lm::ILogService, "lm")
        SERVICE_CASE(aocsrv::IAddOnContentManager, "aoc:u")
        SERVICE_CASE(pctl::IParentalControlServiceFactory, "pctl:s", "pctl:r",
                     "pctl:a", "pctl")
        SERVICE_CASE(account::IAccountServiceForApplication, "acc:u0")
        SERVICE_CASE(account::IAccountServiceForSystemService, "acc:u1")
        SERVICE_CASE(apm::IManagerPrivileged, "apm:p")
        SERVICE_CASE(socket::IClient, "bsd:u", "bsd:s", "bsd:a")
        SERVICE_CASE(settings::ISettingsServer, "set")
        SERVICE_CASE(audio::IAudioOutManager, "audout:u")
        SERVICE_CASE(audio::IAudioRendererManager, "audren:u")
        SERVICE_CASE(ssl::sf::ISslService, "ssl")
        SERVICE_CASE(spl::IRandomInterface, "csrng")
        SERVICE_CASE(nifm::IStaticService, "nifm:a", "nifm:s", "nifm:u")
        SERVICE_CASE(pcv::IPcvService, "pcv")
        SERVICE_CASE(socket::Resolver::IResolver, "sfdnsres")
        SERVICE_CASE(spl::IGeneralInterface, "spl:")
        SERVICE_CASE(friends::IServiceCreator, "friend:u", "friend:v",
                     "friend:m", "friend:s", "friend:a")
        SERVICE_CASE(ts::IMeasurementServer, "ts")
        SERVICE_CASE(nfc::IUserManager, "nfp:user")
        SERVICE_CASE(prepo::IPrepoService, "prepo:a", "prepo:a2", "prepo:m",
                     "prepo:u", "prepo:s")
        SERVICE_CASE(codec::IHardwareOpusDecoderManager, "hwopus")
    default:
        LOG_WARN(Services, "Unknown service name \"{}\"", u64_to_str(name));
        return MAKE_RESULT(Svc, kernel::Error::NotFound); // TODO: module
    }

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::sm
