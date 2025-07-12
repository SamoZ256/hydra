#include "core/horizon/services/am/library_applet_accessor.hpp"

#include "core/horizon/applets/err/error_applet.hpp"
#include "core/horizon/applets/swkbd/software_keyboard.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/am/storage.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ILibraryAppletAccessor, 0,
                             GetAppletStateChangedEvent, 10, Start, 30,
                             GetResult, 100, PushInData, 101, PopOutData, 103,
                             PushInteractiveInData, 104, PopInteractiveOutData,
                             106, GetPopInteractiveOutDataEvent)

ILibraryAppletAccessor::ILibraryAppletAccessor(const AppletId id,
                                               const LibraryAppletMode mode)
    : controller(mode) {
    switch (id) {
    case AppletId::LibraryAppletError:
        applet = new applets::err::ErrorApplet(controller);
        break;
    case AppletId::LibraryAppletSwkbd:
        applet = new applets::swkbd::SoftwareKeyboard(controller);
        break;
    default:
        LOG_NOT_IMPLEMENTED(Services, "Applet ID {}", id);
        applet = nullptr;
        break;
    }
}

ILibraryAppletAccessor::~ILibraryAppletAccessor() { delete applet; }

result_t ILibraryAppletAccessor::GetAppletStateChangedEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(controller.GetStateChangedEvent());
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::Start() {
    applet->Start();
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::GetResult() { return applet->GetResult(); }

result_t ILibraryAppletAccessor::PushInData(ServiceBase* storage_) {
    auto storage = dynamic_cast<IStorage*>(storage_);
    ASSERT_DEBUG(storage, Services, "Storage is not of type IStorage");

    controller.PushInData(storage);
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::PopOutData(add_service_fn_t add_service) {
    add_service(controller.PopOutData());
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::PushInteractiveInData(ServiceBase* storage_) {
    auto storage = dynamic_cast<IStorage*>(storage_);
    ASSERT_DEBUG(storage, Services, "Storage is not of type IStorage");

    controller.PushInteractiveInData(storage);
    return RESULT_SUCCESS;
}

result_t
ILibraryAppletAccessor::PopInteractiveOutData(add_service_fn_t add_service) {
    add_service(controller.PopInteractiveOutData());
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::GetPopInteractiveOutDataEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(controller.GetInteractiveOutDataEvent());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
