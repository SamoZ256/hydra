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

namespace ui {
class IHandler;
}

class OS {
  public:
    explicit OS(System& system_);

    void notifyOperationModeChanged();

    void setSurfaceResolution(uint2 resolution);
    uint2 getDisplayResolution() const;

    services::am::internal::LibraryAppletController&
    getLibraryAppletSelfController() {
        return *library_applet_self_controller;
    }

    void setLibraryAppletSelfController(
        services::am::internal::LibraryAppletController
            library_applet_self_controller_) {
        library_applet_self_controller =
            std::move(library_applet_self_controller_);
    }

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

    std::optional<services::am::internal::LibraryAppletController>
        library_applet_self_controller;

    // Display
    uint2 surface_resolution;

  public:
    REF_GETTER(kernel, getKernel);
    REF_GETTER(filesystem, getFilesystem);
    REF_GETTER(service_manager, getServiceManager);
    REF_GETTER(display_driver, getDisplayDriver);
    REF_GETTER(hid_resource_manager, getHidResourceManager);
    REF_GETTER(user_manager, getUserManager);
    REF_GETTER(shared_font_manager, getSharedFontManager);
    REF_GETTER(time_manager, getTimeManager);
    REF_GETTER(ir_sensor_manager, getIrSensorManager);
};

} // namespace hydra::horizon
