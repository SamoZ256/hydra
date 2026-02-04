#include "core/hw/tegra_x1/cpu/hypervisor/cpu.hpp"

#include "core/debugger/debugger_manager.hpp"
#include "core/horizon/os.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/mmu.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/thread.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

namespace {

constexpr usize KERNEL_MEM_SIZE = 0x1000;

const u32 exception_handler[] = {
    0xd41fffe2u, // hvc #0xFFFF
    // 0xd69f03e0u, // eret
    // 0xD2B00000, // mov x0, #0x80000000
    // 0xD61F0000, // br  x0
    // Shouldn't happen
    0xd4200000u, // brk #0
};

const u32 exception_trampoline[] = {
    0xd508831fu, // msr spsel, xzr

    // 0x910003e0,  // mov x0, sp
    // 0xd5384241,  // TODO
    // 0xd5384202,  // mrs x2, spsel
    // 0xD4200000u, // brk #0

    0xd69f03e0u, // eret
    // Shouldn't happen
    0xd4200000u, // brk #0
};

} // namespace

VirtualMachine::VirtualMachine() { HV_ASSERT_SUCCESS(hv_vm_create(nullptr)); }

VirtualMachine::~VirtualMachine() { HV_ASSERT_SUCCESS(hv_vm_destroy()); }

Cpu::Cpu()
    : kernel_mem(align(KERNEL_MEM_SIZE, APPLE_PAGE_SIZE)),
      kernel_page_table(KERNEL_PAGE_TABLE_REGION_BASE) {
    // Create GIC
    // hv_gic_config_t gic_config = hv_gic_config_create();
    // hv_gic_config_set_msi_region_base(&gic_config, TODO);
    // hv_gic_config_set_msi_interrupt_range(&gic_config, TODO);
    // HYP_ASSERT_SUCCESS(hv_gic_create(gic_config));

    // NOTE: this does not really belong to the CPU class, but it's here for
    // simplicity reasons

    // Kernel memory
    kernel_page_table.Map(
        0x0, Range<uptr>::FromSize(kernel_mem.GetPtr(), KERNEL_MEM_SIZE),
        {horizon::kernel::MemoryType::Kernel,
         horizon::kernel::MemoryAttribute::None,
         horizon::kernel::MemoryPermission::Execute},
        ApFlags::UserNoneKernelReadExecute);

    for (u64 offset = 0; offset < 0x780; offset += 0x80) {
        memcpy(reinterpret_cast<void*>(kernel_mem.GetPtr() + offset),
               exception_handler, sizeof(exception_handler));
    }
    memcpy(reinterpret_cast<void*>(kernel_mem.GetPtr() +
                                   EXCEPTION_TRAMPOLINE_OFFSET),
           exception_trampoline, sizeof(exception_trampoline));

    // Symbols
    // TODO: this should be registered by the process
    /*
    GET_CURRENT_PROCESS_DEBUGGER().GetModuleTable().RegisterSymbol(
        {"Hypervisor::handler",
         Range<vaddr_t>(KERNEL_REGION_BASE,
                        KERNEL_REGION_BASE + EXCEPTION_TRAMPOLINE_OFFSET)});
    GET_CURRENT_PROCESS_DEBUGGER().GetModuleTable().RegisterSymbol(
        {"Hypervisor::trampoline",
         Range<vaddr_t>(KERNEL_REGION_BASE + EXCEPTION_TRAMPOLINE_OFFSET,
                        KERNEL_REGION_BASE + EXCEPTION_TRAMPOLINE_OFFSET +
                            sizeof(exception_trampoline))});
    */

    // Features
    features = {.supports_native_breakpoints = true,
                .supports_synchronous_single_step = false};
}

Cpu::~Cpu() {}

IMmu* Cpu::CreateMmu() { return new Mmu(); }

IThread* Cpu::CreateThread(IMmu* mmu, const ThreadCallbacks& callbacks,
                           IMemory* tls_mem, vaddr_t tls_mem_base) {
    return new Thread(mmu, callbacks, tls_mem, tls_mem_base);
}

IMemory* Cpu::AllocateMemory(usize size) { return new Memory(size); }

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
