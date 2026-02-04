#include "core/horizon/services/ro/detail/ro_interface.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::horizon::services::ro::detail {

DEFINE_SERVICE_COMMAND_TABLE(IRoInterface, 0, MapManualLoadModuleMemory, 2,
                             RegisterModuleInfo, 4, RegisterProcessHandle)

result_t IRoInterface::MapManualLoadModuleMemory(kernel::Process* process,
                                                 u64 pid_reserved, u64 nro_addr,
                                                 u64 nro_size, u64 bss_addr,
                                                 u64 bss_size, u64* out_addr) {
    (void)pid_reserved;

    auto mmu = process->GetMmu();
    const auto base =
        mmu->FindFreeMemory(kernel::EXECUTABLE_REGION, nro_size + bss_size);
    mmu->Map(base, Range<vaddr_t>::FromSize(nro_addr, nro_size));
    mmu->Map(base + nro_size, Range<vaddr_t>::FromSize(bss_addr, bss_size));

    *out_addr = base;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::ro::detail
