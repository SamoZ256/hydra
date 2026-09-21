#include "core/horizon/services/hid/hid_system_server.hpp"

#include "core/horizon/kernel/event.hpp"
#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::hid {

DEFINE_SERVICE_COMMAND_TABLE(IHidSystemServer, 101,
                             acquireHomeButtonEventHandle, 121,
                             acquireSleepButtonEventHandle, 303,
                             applyNpadSystemCommonPolicy, 321,
                             getUniquePadsFromNpad, 503, enableAppletToGetInput)

IHidSystemServer::IHidSystemServer()
    : home_button_event{new kernel::Event(false, "Home button event")},
      sleep_button_event{new kernel::Event(false, "Sleep button event")} {}

result_t IHidSystemServer::acquireHomeButtonEventHandle(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    LOG_FUNC_STUBBED(Services);

    out_handle = process->addHandle(home_button_event);
    return RESULT_SUCCESS;
}

result_t IHidSystemServer::acquireSleepButtonEventHandle(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    LOG_FUNC_STUBBED(Services);

    out_handle = process->addHandle(sleep_button_event);
    return RESULT_SUCCESS;
}

result_t IHidSystemServer::getUniquePadsFromNpad(
    NpadIdType npad_id, i64* out_count,
    OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    (void)out_buffer;

    LOG_FUNC_WITH_ARGS_STUBBED(Services, "npad_id: {}", npad_id);

    // HACK
    *out_count = 0;
    return RESULT_SUCCESS;
}

result_t IHidSystemServer::enableAppletToGetInput(u8 enable, u64 aruid) {
    (void)aruid;

    LOG_FUNC_WITH_ARGS_STUBBED(Services, "enable: {}", enable);

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::hid
