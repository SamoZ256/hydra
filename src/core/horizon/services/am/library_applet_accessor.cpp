#include "core/horizon/services/am/library_applet_accessor.hpp"

#include "core/horizon/applets/controller/applet.hpp"
#include "core/horizon/applets/error/applet.hpp"
#include "core/horizon/applets/player_select/applet.hpp"
#include "core/horizon/applets/software_keyboard/applet.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/am/storage.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ILibraryAppletAccessor, 0,
                             getAppletStateChangedEvent, 10, start, 30,
                             getResult, 100, pushInData, 101, popOutData, 103,
                             pushInteractiveInData, 104, popInteractiveOutData,
                             106, getPopInteractiveOutDataEvent)

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

result_t ILibraryAppletAccessor::getAppletStateChangedEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->addHandle(&controller.getStateChangedEvent());
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::start(System* system) {
    applet->start(*system);
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::getResult() { return applet->getResult(); }

result_t ILibraryAppletAccessor::pushInData(IService* storage_) {
    auto storage = static_cast<IStorage*>(storage_);
    controller.pushInData(storage);
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::popOutData(RequestContext* ctx) {
    addService(*ctx, controller.popOutData()->retain());
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::pushInteractiveInData(IService* storage_) {
    auto storage = static_cast<IStorage*>(storage_);
    controller.pushInteractiveInData(storage);
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::popInteractiveOutData(RequestContext* ctx) {
    addService(*ctx, controller.popInteractiveOutData()->retain());
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::getPopInteractiveOutDataEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->addHandle(&controller.getInteractiveOutDataEvent());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
