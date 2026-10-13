#include "core/horizon/kernel/host_thread.hpp"


namespace hydra::horizon::kernel {

void HostThread::run() {
    run_callback([this] {
        processMessages();
        return getState() == ThreadState::Stopping;
    });
}

} // namespace hydra::horizon::kernel
