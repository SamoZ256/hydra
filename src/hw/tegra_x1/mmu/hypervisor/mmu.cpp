#include "hw/tegra_x1/mmu/hypervisor/mmu.hpp"

#include "hw/tegra_x1/mmu/memory.hpp"
#include "hypervisor/const.hpp"

namespace Hydra::HW::TegraX1::MMU::Hypervisor {

inline hv_memory_flags_t PermisionToHV(Horizon::Permission permission) {
    hv_memory_flags_t flags = 0;
    if (any(permission & Horizon::Permission::Read))
        flags |= HV_MEMORY_READ;
    if (any(permission & Horizon::Permission::Write))
        flags |= HV_MEMORY_WRITE;
    if (any(permission & Horizon::Permission::Execute))
        flags |= HV_MEMORY_EXEC;

    return flags;
}

MMU::MMU() {}

MMU::~MMU() {}

void MMU::ReprotectMemory(Memory* mem) {
    HYP_ASSERT_SUCCESS(hv_vm_protect(mem->GetBase(), mem->GetSize(),
                                     PermisionToHV(mem->GetPermission())));
}

void MMU::MapMemoryImpl(Memory* mem) {
    HYP_ASSERT_SUCCESS(hv_vm_map(mem->GetPtrU8(), mem->GetBase(),
                                 mem->GetSize(),
                                 PermisionToHV(mem->GetPermission())));
}

void MMU::UnmapMemoryImpl(Memory* mem) {
    HYP_ASSERT_SUCCESS(hv_vm_unmap(mem->GetBase(), mem->GetSize()));
}

} // namespace Hydra::HW::TegraX1::MMU::Hypervisor
