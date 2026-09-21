#include "core/horizon/kernel/host_thread.hpp"

#include "core/debugger/debugger_manager.hpp"

namespace hydra::horizon::kernel {

void HostThread::Run() {
    run_callback([this] {
        ProcessMessages();
        return GetState() == ThreadState::Stopping;
    });
}

} // namespace hydra::horizon::kernel
