#include "core/hw/tegra_x1/cpu/dynarmic/cpu.hpp"

#include "core/hw/tegra_x1/cpu/dynarmic/memory.hpp"
#include "core/hw/tegra_x1/cpu/dynarmic/mmu.hpp"
#include "core/hw/tegra_x1/cpu/dynarmic/thread.hpp"

namespace hydra::hw::tegra_x1::cpu::dynarmic {

IMmu* Cpu::CreateMmu() { return new Mmu(); }

IThread* Cpu::CreateThread(IMmu* mmu, const svc_handler_fn_t& svc_handler,
                           const stop_requested_fn_t& stop_requested,
                           IMemory* tls_mem, vaddr_t tls_mem_base,
                           vaddr_t stack_mem_end) {
    return new Thread(mmu, svc_handler, stop_requested, tls_mem, tls_mem_base,
                      stack_mem_end);
}

IMemory* Cpu::AllocateMemory(usize size) {
    size = align(size, GUEST_PAGE_SIZE);
    auto memory = new Memory(size);

    return memory;
}

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
