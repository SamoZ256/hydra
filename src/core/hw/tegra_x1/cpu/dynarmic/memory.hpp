#pragma once

#include "core/hw/tegra_x1/cpu/memory.hpp"

namespace hydra::hw::tegra_x1::cpu::dynarmic {

class Memory : public IMemory {
  public:
    Memory(u64 size) : IMemory(size) { Allocate(); }
    ~Memory() override { Free(); }

    uptr GetPtr() const override { return ptr; }

  protected:
    void ResizeImpl() override {
        Free();
        Allocate();
    }

  private:
    uptr ptr;

    // Helpers
    void Allocate() { ptr = reinterpret_cast<uptr>(malloc(GetSize())); }
    void Free() { free(reinterpret_cast<void*>(ptr)); }
};

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
