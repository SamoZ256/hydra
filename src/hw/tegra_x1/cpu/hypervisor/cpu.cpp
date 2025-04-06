#include "hw/tegra_x1/cpu/hypervisor/cpu.hpp"

#include "common/common.hpp"
#include "horizon/os.hpp"
#include "hw/tegra_x1/cpu/hypervisor/mmu.hpp"
#include "hw/tegra_x1/cpu/hypervisor/thread.hpp"

namespace Hydra::HW::TegraX1::CPU::Hypervisor {

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
    thread->SetupVTimer();

    return thread;
}

MMUBase* CPU::GetMMU() const { return mmu; }

} // namespace Hydra::HW::TegraX1::CPU::Hypervisor
