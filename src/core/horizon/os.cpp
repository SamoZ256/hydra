#include "core/horizon/os.hpp"

#include "core/horizon/services/sm/user_interface.hpp"
#include "core/hw/tegra_x1/cpu/mmu_base.hpp"

namespace hydra::horizon {

SINGLETON_DEFINE_GET_INSTANCE(OS, Horizon)

OS::OS(hw::Bus& bus, hw::tegra_x1::cpu::MMUBase* mmu_,
       audio::CoreBase& audio_core_, ui::HandlerBase& ui_handler_)
    : mmu{mmu_}, audio_core{audio_core_}, ui_handler{ui_handler_},
      kernel(bus, mmu_) {
    SINGLETON_SET_INSTANCE(OS, Horizon);

    // Services
    sm_user_interface = new services::sm::IUserInterface();

    kernel.ConnectServiceToPort("sm:", sm_user_interface);
}

OS::~OS() { SINGLETON_UNSET_INSTANCE(); }

} // namespace hydra::horizon
