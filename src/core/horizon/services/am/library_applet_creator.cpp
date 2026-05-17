#include "core/horizon/services/am/library_applet_creator.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/am/storage.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ILibraryAppletCreator, 0, CreateLibraryApplet, 10,
                             CreateStorage, 11, CreateTransferMemoryStorage)

result_t ILibraryAppletCreator::CreateLibraryApplet(RequestContext* ctx,
                                                    AppletId id,
                                                    LibraryAppletMode mode) {
    LOG_DEBUG(Services, "ID: {}, mode: {}", id, mode);

    AddService(*ctx, new ILibraryAppletAccessor(id, mode));
    return RESULT_SUCCESS;
}

result_t ILibraryAppletCreator::CreateStorage(RequestContext* ctx, i64 size) {
    LOG_DEBUG(Services, "Size: {}", size);

    const auto ptr = reinterpret_cast<u8*>(malloc(static_cast<usize>(size)));
    AddService(*ctx, new IStorage(std::span{ptr, static_cast<usize>(size)}));
    return RESULT_SUCCESS;
}

result_t ILibraryAppletCreator::CreateTransferMemoryStorage(
    kernel::Process* process, RequestContext* ctx,
    InHandle<HandleAttr::Copy> tmem_handle, bool writable, i64 size) {
    (void)writable;

    auto tmem = process->GetHandle<kernel::TransferMemory>(tmem_handle);
    const auto ptr =
        reinterpret_cast<u8*>(process->GetMmu()->UnmapAddr(tmem->GetAddress()));
    AddService(*ctx, new IStorage(std::span{ptr, static_cast<usize>(size)}));
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
