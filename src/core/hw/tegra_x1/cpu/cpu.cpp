#include "core/hw/tegra_x1/cpu/cpu.hpp"

namespace hydra::hw::tegra_x1::cpu {

SINGLETON_DEFINE_GET_INSTANCE(ICpu, Cpu)

ICpu::ICpu() { SINGLETON_SET_INSTANCE(Cpu, Cpu); }

ICpu::~ICpu() { SINGLETON_UNSET_INSTANCE(); }

} // namespace hydra::hw::tegra_x1::cpu
