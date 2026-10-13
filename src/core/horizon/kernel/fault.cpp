#include "core/horizon/kernel/fault.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/kernel/thread.hpp"

namespace hydra::horizon::kernel {

void reportFaultOnThisThread(const std::string_view reason) {
    auto* thread = tls_current_thread;
    if (thread == nullptr || thread->getProcess() == nullptr) {
        LOG_ERROR(Kernel, "BREAK on a thread without a process: {}", reason);
        return;
    }

    thread->getProcess()->reportFault(thread, reason);
}

} // namespace hydra::horizon::kernel
