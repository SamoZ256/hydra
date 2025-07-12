#include "core/horizon/os.hpp"

#include "core/horizon/services/sm/user_interface.hpp"

namespace hydra::horizon {

SINGLETON_DEFINE_GET_INSTANCE(OS, Horizon)

OS::OS(audio::ICore& audio_core_, ui::HandlerBase& ui_handler_)
    : audio_core{audio_core_}, ui_handler{ui_handler_} {
    SINGLETON_SET_INSTANCE(OS, Horizon);

    // Services
    sm_user_interface = new services::sm::IUserInterface();

    kernel.ConnectServiceToPort("sm:", sm_user_interface);
}

OS::~OS() { SINGLETON_UNSET_INSTANCE(); }

} // namespace hydra::horizon
