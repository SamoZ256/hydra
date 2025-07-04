#pragma once

#include "core/horizon/display/driver.hpp"
#include "core/horizon/input_manager.hpp"
#include "core/horizon/services/account/user_manager.hpp"
#include "core/horizon/state_manager.hpp"

#define OS_INSTANCE horizon::OS::GetInstance()
#define INPUT_MANAGER_INSTANCE OS_INSTANCE.GetInputManager()

namespace hydra::audio {
class CoreBase;
}

namespace hydra::horizon {

namespace services::am {
class LibraryAppletController;
}

namespace services::sm {
class IUserInterface;
}

namespace ui {
class HandlerBase;
}

class OS {
  public:
    static OS& GetInstance();

    OS(hw::tegra_x1::cpu::MMUBase* mmu_, audio::CoreBase& audio_core_,
       ui::HandlerBase& ui_handler_);
    ~OS();

    // Getters
    audio::CoreBase& GetAudioCore() { return audio_core; }
    ui::HandlerBase& GetUiHandler() { return ui_handler; }
    kernel::Kernel& GetKernel() { return kernel; }
    StateManager& GetStateManager() { return state_manager; }
    display::Driver& GetDisplayDriver() { return display_driver; }
    InputManager& GetInputManager() { return input_manager; }

    bool IsInHandheldMode() const {
        // TODO: make this configurable
        return true;
    }

  private:
    hw::tegra_x1::cpu::MMUBase* mmu;
    audio::CoreBase& audio_core;
    ui::HandlerBase& ui_handler;

    kernel::Kernel kernel;

    // Services
    services::sm::IUserInterface* sm_user_interface;

    // Managers
    StateManager state_manager;
    display::Driver display_driver;
    InputManager input_manager;
    services::account::UserManager user_manager;

    services::am::LibraryAppletController* library_applet_self_controller{
        nullptr};

  public:
    GETTER_AND_SETTER(library_applet_self_controller,
                      GetLibraryAppletSelfController,
                      SetLibraryAppletSelfController);
};

} // namespace hydra::horizon
