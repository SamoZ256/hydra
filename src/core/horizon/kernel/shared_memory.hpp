#pragma once

#include "core/horizon/kernel/auto_object.hpp"
#include "core/horizon/kernel/const.hpp"
#include "core/hw/tegra_x1/cpu/memory_base.hpp"

namespace hydra::hw::tegra_x1::cpu {
class MMUBase;
} // namespace hydra::hw::tegra_x1::cpu

namespace hydra::horizon::kernel {

class SharedMemory : public AutoObject {
  public:
    SharedMemory(usize size,
                 const std::string_view debug_name = "SharedMemory");
    ~SharedMemory() override;

    void MapToRange(const range<uptr> range_, MemoryPermission perm);

    // Getters
    uptr GetPtr() const;

  private:
    hw::tegra_x1::cpu::MemoryBase* memory;
};

} // namespace hydra::horizon::kernel
