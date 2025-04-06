#include "horizon/services/sm/user_interface.hpp"

#include "horizon/services/account/account_service_for_application.hpp"
#include "horizon/services/account/account_service_for_system_service.hpp"
#include "horizon/services/am/apm_manager.hpp"
#include "horizon/services/am/application_proxy_service.hpp"
#include "horizon/services/aocsrv/add_on_content_manager.hpp"
#include "horizon/services/apm/manager_privileged.hpp"
#include "horizon/services/audio/audio_out_manager.hpp"
#include "horizon/services/fssrv/filesystem_proxy.hpp"
#include "horizon/services/hid/hid_server.hpp"
#include "horizon/services/lm/log_service.hpp"
#include "horizon/services/nvdrv/nvdrv_services.hpp"
#include "horizon/services/pctl/ipc/parental_control_service_factory.hpp"
#include "horizon/services/pl/sharedresource/platform_shared_resource_manager.hpp"
#include "horizon/services/psm/psm_server.hpp"
#include "horizon/services/settings/settings_server.hpp"
#include "horizon/services/settings/system_settings_server.hpp"
#include "horizon/services/socket/client.hpp"
#include "horizon/services/timesrv/static_service.hpp"
#include "horizon/services/visrv/manager_root_service.hpp"

namespace Hydra::Horizon::Services::Sm {

DEFINE_SERVICE_COMMAND_TABLE(IUserInterface, 0, RegisterProcess, 1,
                             GetServiceHandle)

void IUserInterface::GetServiceHandle(REQUEST_COMMAND_PARAMS) {
    u64 name = readers.reader.Read<u64>();

    if (name == 0) {
        result = MAKE_KERNEL_RESULT(NotFound);
        return;
    }

#define SERVICE_CASE_CASE(str) case str_to_u64(str):
#define SERVICE_CASE(name, ...)                                                \
    FOR_EACH_0_1(SERVICE_CASE_CASE, __VA_ARGS__)                               \
    add_service(new name());                                                   \
    break;

    switch (name) {
        SERVICE_CASE(Hid::IHidServer, "hid")
        SERVICE_CASE(Fssrv::IFileSystemProxy, "fsp-srv")
        SERVICE_CASE(TimeSrv::IStaticService, "time:u", "time:a", "time:s")
        SERVICE_CASE(NvDrv::INvDrvServices, "nvdrv")
        SERVICE_CASE(Settings::ISystemSettingsServer, "set:sys")
        SERVICE_CASE(Am::IApmManager, "apm", "apm:am")
        SERVICE_CASE(Am::IApplicationProxyService, "appletOE")
        SERVICE_CASE(ViSrv::IManagerRootService, "vi:m")
        SERVICE_CASE(Pl::SharedResource::IPlatformSharedResourceManager, "pl:u")
        SERVICE_CASE(Psm::IPsmServer, "psm")
        SERVICE_CASE(Lm::ILogService, "lm")
        SERVICE_CASE(AocSrv::IAddOnContentManager, "aoc:u")
        SERVICE_CASE(Pctl::Ipc::IParentalControlServiceFactory, "pctl:s",
                     "pctl:r", "pctl:a", "pctl")
        SERVICE_CASE(Account::IAccountServiceForApplication, "acc:u0")
        SERVICE_CASE(Apm::IManagerPrivileged, "apm:p")
        SERVICE_CASE(Socket::IClient, "bsd:u", "bsd:s", "bsd:a")
        SERVICE_CASE(Settings::ISettingsServer, "set")
        SERVICE_CASE(Account::IAccountServiceForSystemService, "acc:u1")
        SERVICE_CASE(Audio::IAudioOutManager, "audout:u")
    default:
        LOG_WARNING(HorizonServices, "Unknown service \"{}\"",
                    u64_to_str(name));
        result = MAKE_KERNEL_RESULT(NotFound);
        // TODO: don't throw
        throw;
    }
}

} // namespace Hydra::Horizon::Services::Sm
