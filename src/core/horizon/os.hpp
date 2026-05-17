#pragma once

#include "core/horizon/display/driver.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/services/account/internal/user_manager.hpp"
#include "core/horizon/services/am/internal/library_applet_controller.hpp"
#include "core/horizon/services/hid/internal/resource_manager.hpp"
#include "core/horizon/services/irsensor/internal/ir_sensor_manager.hpp"
#include "core/horizon/services/pl/internal/shared_font_manager.hpp"
#include "core/horizon/services/server.hpp"
#include "core/horizon/services/timesrv/internal/time_manager.hpp"

namespace hydra::audio {
class ICore;
}

namespace hydra::horizon {

namespace services::am {
class LibraryAppletController;
}

namespace ui {
class IHandler;
}

class OS {
  public:
    OS(System& system_);

    void NotifyOperationModeChanged();

    void SetSurfaceResolution(uint2 resolution);
    uint2 GetDisplayResolution() const;

  private:
    System& system;

    kernel::Kernel kernel;
    filesystem::Filesystem filesystem;

    // Services
    kernel::hipc::ServiceManager<u64> service_manager;
    services::Server nvservices_server;
    services::Server others_server;

    // Managers
    display::Driver display_driver;
    services::hid::internal::ResourceManager hid_resource_manager;
    services::account::internal::UserManager user_manager;
    services::pl::internal::SharedFontManager shared_font_manager;
    services::timesrv::internal::TimeManager time_manager;
    services::irsensor::internal::IrSensorManager ir_sensor_manager;

    services::am::internal::LibraryAppletController*
        library_applet_self_controller{nullptr};

    // Display
    uint2 surface_resolution;

  public:
    REF_GETTER(kernel, GetKernel);
    REF_GETTER(filesystem, GetFilesystem);
    REF_GETTER(service_manager, GetServiceManager);
    REF_GETTER(display_driver, GetDisplayDriver);
    REF_GETTER(hid_resource_manager, GetHidResourceManager);
    REF_GETTER(user_manager, GetUserManager);
    REF_GETTER(shared_font_manager, GetSharedFontManager);
    REF_GETTER(time_manager, GetTimeManager);
    REF_GETTER(ir_sensor_manager, GetIrSensorManager);
    GETTER_AND_SETTER(library_applet_self_controller,
                      GetLibraryAppletSelfController,
                      SetLibraryAppletSelfController);
};

} // namespace hydra::horizon
