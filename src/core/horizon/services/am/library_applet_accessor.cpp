#include "core/horizon/services/am/library_applet_accessor.hpp"

#include "core/horizon/applets/controller/applet.hpp"
#include "core/horizon/applets/error/applet.hpp"
#include "core/horizon/applets/player_select/applet.hpp"
#include "core/horizon/applets/software_keyboard/applet.hpp"
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
        applet = new applets::error::Applet(controller);
        break;
    case AppletId::LibraryAppletSwkbd:
        applet = new applets::software_keyboard::Applet(controller);
        break;
    case AppletId::LibraryAppletPlayerSelect:
        applet = new applets::player_select::Applet(controller);
        break;
    case AppletId::LibraryAppletController:
        applet = new applets::controller::Applet(controller);
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

result_t ILibraryAppletAccessor::Start(System* system) {
    applet->Start(*system);
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::GetResult() { return applet->GetResult(); }

result_t ILibraryAppletAccessor::PushInData(IService* storage_) {
    auto storage = dynamic_cast<IStorage*>(storage_);
    ASSERT_DEBUG(storage, Services, "Storage is not of type IStorage");

    controller.PushInData(storage);
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::PopOutData(RequestContext* ctx) {
    AddService(*ctx, controller.PopOutData()->Retain());
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::PushInteractiveInData(IService* storage_) {
    auto storage = dynamic_cast<IStorage*>(storage_);
    ASSERT_DEBUG(storage, Services, "Storage is not of type IStorage");

    controller.PushInteractiveInData(storage);
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::PopInteractiveOutData(RequestContext* ctx) {
    AddService(*ctx, controller.PopInteractiveOutData()->Retain());
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::GetPopInteractiveOutDataEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(controller.GetInteractiveOutDataEvent());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
