#include "hw/tegra_x1/cpu/cpu_base.hpp"

namespace Hydra::HW::TegraX1::CPU {

SINGLETON_DEFINE_GET_INSTANCE(CPUBase, CPU, "CPU")

CPUBase::CPUBase() { SINGLETON_SET_INSTANCE(CPU, "CPU"); }

CPUBase::~CPUBase() { SINGLETON_UNSET_INSTANCE(); }

} // namespace Hydra::HW::TegraX1::CPU
