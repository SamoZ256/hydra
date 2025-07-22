#include "core/horizon/services/am/library_applet_creator.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/am/storage.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ILibraryAppletCreator, 0, CreateLibraryApplet, 10,
                             CreateStorage, 11, CreateTransferMemoryStorage)

result_t ILibraryAppletCreator::CreateLibraryApplet(
    add_service_fn_t add_service, AppletId id, LibraryAppletMode mode) {
    LOG_DEBUG(Services, "ID: {}, mode: {}", id, mode);

    add_service(new ILibraryAppletAccessor(id, mode));
    return RESULT_SUCCESS;
}

result_t ILibraryAppletCreator::CreateStorage(add_service_fn_t add_service,
                                              i64 size) {
    LOG_DEBUG(Services, "Size: {}", size);

    add_service(new IStorage(sized_ptr(malloc(size), size)));
    return RESULT_SUCCESS;
}

result_t ILibraryAppletCreator::CreateTransferMemoryStorage(
    kernel::Process* process, add_service_fn_t add_service,
    InHandle<HandleAttr::Copy> tmem_handle, bool writable, i64 size) {
    auto tmem = process->GetHandle<kernel::TransferMemory>(tmem_handle);

    add_service(new IStorage(
        sized_ptr(process->GetMmu()->UnmapAddr(tmem->GetAddress()), size)));
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
