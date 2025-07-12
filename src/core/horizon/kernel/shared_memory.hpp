#pragma once

#include "core/horizon/kernel/auto_object.hpp"
#include "core/horizon/kernel/const.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
} // namespace hydra::hw::tegra_x1::cpu

namespace hydra::horizon::kernel {

class SharedMemory : public AutoObject {
  public:
    SharedMemory(usize size,
                 const std::string_view debug_name = "SharedMemory");
    ~SharedMemory() override;

    void MapToRange(hw::tegra_x1::cpu::IMmu* mmu, const range<uptr> range_,
                    MemoryPermission perm);

    // Getters
    uptr GetPtr() const;

  private:
    hw::tegra_x1::cpu::IMemory* memory;
};

} // namespace hydra::horizon::kernel
