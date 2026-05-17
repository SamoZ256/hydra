#include "core/horizon/applets/applet_base.hpp"

#include "core/debugger/debugger_manager.hpp"
#include "core/horizon/applets/const.hpp"

namespace hydra::horizon::applets {

void AppletBase::Start(System& system) {
    // TODO: not every applet uses common args (for instance
    // LibraryAppletMiiEdit)
    const auto common_args = PopInData<CommonArguments>();
    ASSERT(common_args.version == 1, Applets, "Unsupported version {}",
           common_args.version); // TODO: support version 0
    ASSERT(common_args.size == sizeof(CommonArguments), Applets,
           "Invalid struct size 0x{:x}", common_args.size);

    // TODO: create process

    thread = new std::thread([&]() {
        GET_CURRENT_PROCESS_DEBUGGER().RegisterThisThread("Applet");
        result = Run(system);
        controller.GetStateChangedEvent()->Signal();
        GET_CURRENT_PROCESS_DEBUGGER().UnregisterThisThread();
    });
}

} // namespace hydra::horizon::applets
