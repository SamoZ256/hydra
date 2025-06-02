#include "core/horizon/services/am/library_applet_accessor.hpp"

#include "core/horizon/applets/error_applet.hpp"
#include "core/horizon/applets/software_keyboard.hpp"
#include "core/horizon/services/am/storage.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ILibraryAppletAccessor, 0,
                             GetAppletStateChangedEvent, 10, Start, 30,
                             GetResult, 100, PushInData, 101, PopOutData)

ILibraryAppletAccessor::ILibraryAppletAccessor(const AppletId id,
                                               const LibraryAppletMode mode) {
    switch (id) {
    case AppletId::LibraryAppletError:
        applet = new applets::ErrorApplet(mode);
        break;
    case AppletId::LibraryAppletSwkbd:
        applet = new applets::SoftwareKeyboard(mode);
        break;
    default:
        LOG_NOT_IMPLEMENTED(Services, "Applet ID {}", id);
        applet = nullptr;
        break;
    }
}

ILibraryAppletAccessor::~ILibraryAppletAccessor() { delete applet; }

result_t ILibraryAppletAccessor::GetAppletStateChangedEvent(
    OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = applet->GetStateChangedEventID();
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

    applet->PushInData(storage->GetData());
    return RESULT_SUCCESS;
}

result_t ILibraryAppletAccessor::PopOutData(add_service_fn_t add_service) {
    add_service(new IStorage(applet->PopOutData()));
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
