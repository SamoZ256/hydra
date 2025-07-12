#include "core/hw/tegra_x1/cpu/hypervisor/cpu.hpp"

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

void create_vm() {
    // hv_vm_config_t vm_config = hv_vm_config_create();
    // hv_vm_config_set_el2_enabled(config, true);
    HV_ASSERT_SUCCESS(hv_vm_create(nullptr));
}

} // namespace

Cpu::Cpu()
    : kernel_mem((create_vm(), pa_mapper),
                 align(KERNEL_MEM_SIZE, APPLE_PAGE_SIZE)),
      kernel_page_table(KERNEL_PAGE_TABLE_REGION_BASE) {
    // Create GIC
    // hv_gic_config_t gic_config = hv_gic_config_create();
    // hv_gic_config_set_msi_region_base(&gic_config, TODO);
    // hv_gic_config_set_msi_interrupt_range(&gic_config, TODO);
    // HYP_ASSERT_SUCCESS(hv_gic_create(gic_config));

    // NOTE: this does not really belong to the CPU class, but it's here for
    // simplicity reasons

    // Kernel memory
    kernel_page_table.Map(0x0, pa_mapper.GetPA(kernel_mem.GetPtr()),
                          KERNEL_MEM_SIZE,
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
    DEBUGGER_INSTANCE.GetModuleTable().RegisterSymbol(
        {"Hypervisor::handler",
         range<vaddr_t>(KERNEL_REGION_BASE,
                        KERNEL_REGION_BASE + EXCEPTION_TRAMPOLINE_OFFSET)});
    DEBUGGER_INSTANCE.GetModuleTable().RegisterSymbol(
        {"Hypervisor::trampoline",
         range<vaddr_t>(KERNEL_REGION_BASE + EXCEPTION_TRAMPOLINE_OFFSET,
                        KERNEL_REGION_BASE + EXCEPTION_TRAMPOLINE_OFFSET +
                            sizeof(exception_trampoline))});
}

Cpu::~Cpu() { hv_vm_destroy(); }

IMmu* Cpu::CreateMmu() { return new Mmu(); }

IThread* Cpu::CreateThread(IMmu* mmu, const svc_handler_fn_t& svc_handler,
                           const stop_requested_fn_t& stop_requested,
                           IMemory* tls_mem, vaddr_t tls_mem_base,
                           vaddr_t stack_mem_end) {
    return new Thread(mmu, svc_handler, stop_requested, tls_mem, tls_mem_base,
                      stack_mem_end);
}

IMemory* Cpu::AllocateMemory(usize size) { return new Memory(pa_mapper, size); }

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
