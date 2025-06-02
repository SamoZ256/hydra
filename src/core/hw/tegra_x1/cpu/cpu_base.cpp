#include "core/hw/tegra_x1/cpu/cpu_base.hpp"

namespace hydra::hw::tegra_x1::cpu {

SINGLETON_DEFINE_GET_INSTANCE(CPUBase, CPU)

CPUBase::CPUBase() { SINGLETON_SET_INSTANCE(CPU, CPU); }

CPUBase::~CPUBase() { SINGLETON_UNSET_INSTANCE(); }

} // namespace hydra::hw::tegra_x1::cpu
