#include "hw/tegra_x1/cpu/mmu_base.hpp"

namespace Hydra::HW::TegraX1::CPU {

SINGLETON_DEFINE_GET_INSTANCE(MMUBase, MMU, "MMU")

MMUBase::MMUBase() { SINGLETON_SET_INSTANCE(MMU, "MMU"); }

MMUBase::~MMUBase() { SINGLETON_UNSET_INSTANCE(); }

} // namespace Hydra::HW::TegraX1::CPU
