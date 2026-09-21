#include "core/horizon/services/psc/pm_module.hpp"

#include "core/horizon/kernel/event.hpp"
#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::psc {

DEFINE_SERVICE_COMMAND_TABLE(IPmModule, 0, initialize)

IPmModule::IPmModule() : event{new kernel::Event(false, "IPmModule event")} {}

result_t IPmModule::initialize(kernel::Process* process, PmModuleId module_id,
                               InBuffer<BufferAttr::MapAlias> in_dep_buffer,
                               OutHandle<HandleAttr::Copy> out_event_handle) {
    (void)in_dep_buffer;
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "module ID: {}", module_id);

    // TODO: what are the dependencies?

    out_event_handle = process->addHandle(event);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::psc
