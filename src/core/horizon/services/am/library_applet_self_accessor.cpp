#include "core/horizon/services/am/library_applet_self_accessor.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/am/internal/library_applet_controller.hpp"
#include "core/system.hpp"

#define CONTROLLER system->GetOS().GetLibraryAppletSelfController()

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ILibraryAppletSelfAccessor, 0, PopInData, 1,
                             PushOutData, 2, PopInteractiveInData, 3,
                             PushInteractiveOutData, 10, ExitProcessAndReturn,
                             11, GetLibraryAppletInfo, 14,
                             GetCallerAppletIdentityInfo, 160,
                             GetLibraryAppletInfoEx)

result_t ILibraryAppletSelfAccessor::PopInData(RequestContext* ctx,
                                               System* system) {
    AddService(*ctx, CONTROLLER->PopInData()->Retain());
    return RESULT_SUCCESS;
}

result_t ILibraryAppletSelfAccessor::PushOutData(System* system,
                                                 IService* storage_) {
    auto storage = static_cast<IStorage*>(storage_);
    CONTROLLER->PushOutData(storage);
    return RESULT_SUCCESS;
}

result_t ILibraryAppletSelfAccessor::PopInteractiveInData(RequestContext* ctx,
                                                          System* system) {
    AddService(*ctx, CONTROLLER->PopInteractiveInData()->Retain());
    return RESULT_SUCCESS;
}

result_t
ILibraryAppletSelfAccessor::PushInteractiveOutData(System* system,
                                                   IService* storage_) {
    auto storage = static_cast<IStorage*>(storage_);
    CONTROLLER->PushInteractiveOutData(storage);
    return RESULT_SUCCESS;
}

result_t
ILibraryAppletSelfAccessor::ExitProcessAndReturn(kernel::Process* process) {
    // TODO: correct?
    process->Stop();

    return RESULT_SUCCESS;
}

result_t
ILibraryAppletSelfAccessor::GetLibraryAppletInfo(LibraryAppletInfo* out_info) {
    LOG_FUNC_STUBBED(Services);

    // HACK: hardcoded for Mii Edit
    *out_info = {
        .id = AppletId::LibraryAppletMiiEdit,
        .mode = LibraryAppletMode::AllForeground,
    };
    return RESULT_SUCCESS;
}

result_t ILibraryAppletSelfAccessor::GetCallerAppletIdentityInfo(
    AppletIdentityInfo* out_info) {
    // TODO: don't hardcode
    *out_info = {
        .id = AppletId::None,
        .application_id = 0x1,
    };
    return RESULT_SUCCESS;
}

result_t ILibraryAppletSelfAccessor::GetLibraryAppletInfoEx(u64* out_info) {
    // Always returns 0
    *out_info = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
