#include "core/hw/tegra_x1/cpu/hypervisor/cpu.hpp"

#include <sys/sysctl.h>

#include "core/horizon/os.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/mmu.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/thread.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

namespace {

constexpr u64 KERNEL_MEM_SIZE = 0x1000;
constexpr u32 MIN_DEBUG_SLOTS = 2;
constexpr u32 MAX_DEBUG_SLOTS = 16;
constexpr u32 DFR0_BRPS_SHIFT = 12;
constexpr u32 DFR0_WRPS_SHIFT = 20;

u32 debugSlotCount(u32 dfr0_shift, const char* sysctl_name) {
    std::optional<u32> count;

    auto config = hv_vcpu_config_create();
    u64 dfr0;
    if (hv_vcpu_config_get_feature_reg(config, HV_FEATURE_REG_ID_AA64DFR0_EL1,
                                       &dfr0) == HV_SUCCESS)
        count = static_cast<u32>((dfr0 >> dfr0_shift) & 0xf) + 1;
    os_release(config);

    i32 host_count;
    usize size = sizeof(host_count);
    if (sysctlbyname(sysctl_name, &host_count, &size, nullptr, 0) == 0 &&
        host_count > 0)
        count = std::min(count.value_or(MAX_DEBUG_SLOTS),
                         static_cast<u32>(host_count));

    return std::min(count.value_or(MIN_DEBUG_SLOTS), MAX_DEBUG_SLOTS);
}

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

VirtualMachine::VirtualMachine() {
    // HACK: since we're mapping our address space to the HV's "physical"
    // space 1:1, we have to extend their space to include all of ours
    // (or as close as possible as we can)
    auto config = hv_vm_config_create();
    uint32_t max_bits;
    HV_ASSERT_SUCCESS(hv_vm_config_get_max_ipa_size(&max_bits));
    HV_ASSERT_SUCCESS(hv_vm_config_set_ipa_size(config, max_bits));

    HV_ASSERT_SUCCESS(hv_vm_create(config));
    os_release(config);
}

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
    kernel_page_table.map(
        0x0, ztd::Range<uptr>::fromSize(kernel_mem.getPtr(), KERNEL_MEM_SIZE),
        {.type = horizon::kernel::MemoryType::Kernel,
         .attr = horizon::kernel::MemoryAttribute::None,
         .perm = horizon::kernel::MemoryPermission::Execute},
        ApFlags::UserNoneKernelReadExecute);

    for (u64 offset = 0; offset < 0x780; offset += 0x80) {
        memcpy(reinterpret_cast<void*>(kernel_mem.getPtr() + offset),
               exception_handler, sizeof(exception_handler));
    }
    memcpy(reinterpret_cast<void*>(kernel_mem.getPtr() +
                                   EXCEPTION_TRAMPOLINE_OFFSET),
           exception_trampoline, sizeof(exception_trampoline));

    // Symbols
    // TODO: this should be registered by the process
    /*
    GET_CURRENT_PROCESS_DEBUGGER().GetModuleTable().RegisterSymbol(
        {"Hypervisor::handler",
         ztd::Range<vaddr_t>(KERNEL_REGION_BASE,
                        KERNEL_REGION_BASE + EXCEPTION_TRAMPOLINE_OFFSET)});
    GET_CURRENT_PROCESS_DEBUGGER().GetModuleTable().RegisterSymbol(
        {"Hypervisor::trampoline",
         ztd::Range<vaddr_t>(KERNEL_REGION_BASE + EXCEPTION_TRAMPOLINE_OFFSET,
                        KERNEL_REGION_BASE + EXCEPTION_TRAMPOLINE_OFFSET +
                            sizeof(exception_trampoline))});
    */

    // Features
    features = {.breakpoint_slots =
                    debugSlotCount(DFR0_BRPS_SHIFT, "hw.optional.breakpoint"),
                .watchpoint_slots =
                    debugSlotCount(DFR0_WRPS_SHIFT, "hw.optional.watchpoint")};
    LOG_INFO(Hypervisor, "Debug slots: {} breakpoints, {} watchpoints",
             features.breakpoint_slots, features.watchpoint_slots);
}

IMmu* Cpu::createMmu(System& system) { return new Mmu(system); }

IThread* Cpu::createThread(WallClock& wall_clock, IMmu* mmu,
                           const ThreadCallbacks& callbacks, IMemory* tls_mem,
                           vaddr_t tls_mem_base) {
    return new Thread(wall_clock, *this, mmu, callbacks, tls_mem, tls_mem_base);
}

IMemory* Cpu::allocateMemory(u64 size) { return new Memory(size); }

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
