#include "core/horizon/services/am/library_applet_self_accessor.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/am/internal/library_applet_controller.hpp"
#include "core/system.hpp"

#define CONTROLLER system->getOs().getLibraryAppletSelfController()

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ILibraryAppletSelfAccessor, 0, popInData, 1,
                             pushOutData, 2, popInteractiveInData, 3,
                             pushInteractiveOutData, 10, exitProcessAndReturn,
                             11, getLibraryAppletInfo, 14,
                             getCallerAppletIdentityInfo, 160,
                             getLibraryAppletInfoEx)

result_t ILibraryAppletSelfAccessor::popInData(RequestContext* ctx,
                                               System* system) {
    addService(*ctx, CONTROLLER.popInData()->retain());
    return RESULT_SUCCESS;
}

result_t ILibraryAppletSelfAccessor::pushOutData(System* system,
                                                 IService* storage_) {
    auto storage = static_cast<IStorage*>(storage_);
    CONTROLLER.pushOutData(storage);
    return RESULT_SUCCESS;
}

result_t ILibraryAppletSelfAccessor::popInteractiveInData(RequestContext* ctx,
                                                          System* system) {
    addService(*ctx, CONTROLLER.popInteractiveInData()->retain());
    return RESULT_SUCCESS;
}

result_t
ILibraryAppletSelfAccessor::pushInteractiveOutData(System* system,
                                                   IService* storage_) {
    auto storage = static_cast<IStorage*>(storage_);
    CONTROLLER.pushInteractiveOutData(storage);
    return RESULT_SUCCESS;
}

result_t
ILibraryAppletSelfAccessor::exitProcessAndReturn(kernel::Process* process) {
    // TODO: correct?
    process->stop();

    return RESULT_SUCCESS;
}

result_t
ILibraryAppletSelfAccessor::getLibraryAppletInfo(LibraryAppletInfo* out_info) {
    LOG_FUNC_STUBBED(Services);

    // HACK: hardcoded for Mii Edit
    *out_info = {
        .id = AppletId::LibraryAppletMiiEdit,
        .mode = LibraryAppletMode::AllForeground,
    };
    return RESULT_SUCCESS;
}

result_t ILibraryAppletSelfAccessor::getCallerAppletIdentityInfo(
    AppletIdentityInfo* out_info) {
    // TODO: don't hardcode
    *out_info = {
        .id = AppletId::None,
        .application_id = 0x1,
    };
    return RESULT_SUCCESS;
}

result_t ILibraryAppletSelfAccessor::getLibraryAppletInfoEx(u64* out_info) {
    // Always returns 0
    *out_info = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
