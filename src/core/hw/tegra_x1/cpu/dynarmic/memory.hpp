#pragma once

#include "core/hw/tegra_x1/cpu/memory.hpp"

namespace hydra::hw::tegra_x1::cpu::dynarmic {

class Memory : public IMemory {
  public:
    Memory(usize size) : IMemory(size) {
        ptr = reinterpret_cast<uptr>(malloc(size));
    }
    ~Memory() override { free(reinterpret_cast<void*>(ptr)); }

    void Resize(usize new_size) {
        free(reinterpret_cast<void*>(ptr));
        ptr = reinterpret_cast<uptr>(malloc(new_size));
    }

    // Getters
    uptr GetPtr() const override { return ptr; }

  private:
    uptr ptr;
};

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
