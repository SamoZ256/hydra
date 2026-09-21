#include "core/horizon/applets/applet_base.hpp"

#include "core/debugger/debugger_manager.hpp"
#include "core/horizon/applets/const.hpp"

namespace hydra::horizon::applets {

void AppletBase::start(System& system) {
    // TODO: not every applet uses common args (for instance
    // LibraryAppletMiiEdit)
    const auto common_args = popInData<CommonArguments>();
    ASSERT(common_args.version == 1, Applets, "Unsupported version {}",
           common_args.version); // TODO: support version 0
    ASSERT(common_args.size == sizeof(CommonArguments), Applets,
           "Invalid struct size 0x{:x}", common_args.size);

    // TODO: create process

    thread.emplace([&] {
        GET_CURRENT_PROCESS_DEBUGGER().registerThisThread("Applet");
        result = run(system);
        controller.getStateChangedEvent().signal();
        GET_CURRENT_PROCESS_DEBUGGER().unregisterThisThread();
    });
}

} // namespace hydra::horizon::applets
