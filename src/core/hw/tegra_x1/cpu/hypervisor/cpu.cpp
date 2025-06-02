#include "core/hw/tegra_x1/cpu/hypervisor/cpu.hpp"

#include "core/horizon/os.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/mmu.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/thread.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

CPU::CPU() {
    // Create VM
    hv_vm_config_t vm_config = hv_vm_config_create();
    // hv_vm_config_set_el2_enabled(config, true);
    HV_ASSERT_SUCCESS(hv_vm_create(vm_config));

    // Create GIC
    // hv_gic_config_t gic_config = hv_gic_config_create();
    // hv_gic_config_set_msi_region_base(&gic_config, TODO);
    // hv_gic_config_set_msi_interrupt_range(&gic_config, TODO);
    // HYP_ASSERT_SUCCESS(hv_gic_create(gic_config));

    // MMU
    mmu = new MMU();
}

CPU::~CPU() { hv_vm_destroy(); }

ThreadBase* CPU::CreateThread(MemoryBase* tls_mem) {
    Thread* thread = new Thread(mmu, tls_mem);
    return thread;
}

MMUBase* CPU::GetMMU() const { return mmu; }

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
