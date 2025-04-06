#include "horizon/state_manager.hpp"

namespace Hydra::Horizon {

SINGLETON_DEFINE_GET_INSTANCE(StateManager, Horizon, "State manager")

StateManager::StateManager() : msg_event(new SynchronizationHandle()) {
    SINGLETON_SET_INSTANCE(Horizon, "State manager");
}

StateManager::~StateManager() { SINGLETON_UNSET_INSTANCE(); }

} // namespace Hydra::Horizon
