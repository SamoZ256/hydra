#include "core/horizon/services/am/library_applet_creator.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/am/storage.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ILibraryAppletCreator, 0, createLibraryApplet, 10,
                             createStorage, 11, createTransferMemoryStorage)

result_t ILibraryAppletCreator::createLibraryApplet(RequestContext* ctx,
                                                    AppletId id,
                                                    LibraryAppletMode mode) {
    LOG_DEBUG(Services, "ID: {}, mode: {}", id, mode);

    addService(*ctx, new ILibraryAppletAccessor(id, mode));
    return RESULT_SUCCESS;
}

result_t ILibraryAppletCreator::createStorage(RequestContext* ctx, i64 size) {
    LOG_DEBUG(Services, "Size: {}", size);

    addService(*ctx, new IStorage(std::vector<u8>(static_cast<usize>(size))));
    return RESULT_SUCCESS;
}

result_t ILibraryAppletCreator::createTransferMemoryStorage(
    kernel::Process* process, RequestContext* ctx,
    InHandle<HandleAttr::Copy> tmem_handle, bool writable, i64 size) {
    (void)writable;

    auto tmem = process->getHandle<kernel::TransferMemory>(tmem_handle);
    const auto ptr =
        reinterpret_cast<u8*>(process->getMmu()->unmapAddr(tmem->getAddress()));
    std::vector<u8> data(ptr, ptr + static_cast<usize>(size));
    addService(*ctx, new IStorage(std::move(data)));
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
