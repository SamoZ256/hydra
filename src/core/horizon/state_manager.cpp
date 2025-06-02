#include "core/horizon/state_manager.hpp"

namespace hydra::horizon {

SINGLETON_DEFINE_GET_INSTANCE(StateManager, Horizon)

// TODO: autoclear event?
StateManager::StateManager() : msg_event(new kernel::Event()) {
    SINGLETON_SET_INSTANCE(StateManager, Horizon);
}

StateManager::~StateManager() { SINGLETON_UNSET_INSTANCE(); }

} // namespace hydra::horizon
