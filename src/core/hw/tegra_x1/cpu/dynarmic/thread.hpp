#pragma once

#include "core/hw/tegra_x1/cpu/dynarmic/const.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"

namespace hydra::horizon {
class OS;
}

namespace hydra::hw::tegra_x1::cpu {
class Memory;
}

namespace hydra::hw::tegra_x1::cpu::dynarmic {

class Cpu;
class Mmu;

class Thread final : public IThread, private Dynarmic::A64::UserCallbacks {
  public:
    Thread(IMmu* mmu, const ThreadCallbacks& callbacks, IMemory* tls_mem,
           vaddr_t tls_mem_base);
    ~Thread() override;

    void Run() override;

    void NotifyMemoryChanged(Range<vaddr_t> mem_range) override {
        jit->InvalidateCacheRange(mem_range.GetBegin(), mem_range.GetSize());
    }

    // Debug
    void InsertBreakpoint([[maybe_unused]] vaddr_t addr) override {
        LOG_FATAL(Dynarmic, "This should not happen");
    }
    void RemoveBreakpoint([[maybe_unused]] vaddr_t addr) override {
        LOG_FATAL(Dynarmic, "This should not happen");
    }
    void SingleStep() override {
        DeserializeState();
        jit->Step();
        SerializeState();
    }

  private:
    vaddr_t tpidrro_el0;
    vaddr_t tpidr_el0;

    Dynarmic::A64::Jit* jit;
    u64 ticks_left{1000};

    // Dynarmic
    u8 MemoryRead8(u64 addr) override;
    u16 MemoryRead16(u64 addr) override;
    u32 MemoryRead32(u64 addr) override;
    u64 MemoryRead64(u64 addr) override;
    Dynarmic::A64::Vector MemoryRead128(u64 addr) override;
    std::optional<u32> MemoryReadCode(u64 addr) override;

    void MemoryWrite8(u64 addr, u8 value) override;
    void MemoryWrite16(u64 addr, u16 value) override;
    void MemoryWrite32(u64 addr, u32 value) override;
    void MemoryWrite64(u64 addr, u64 value) override;
    void MemoryWrite128(u64 addr, Dynarmic::A64::Vector value) override;

    bool MemoryWriteExclusive8(u64 addr, u8 value, u8) override;
    bool MemoryWriteExclusive16(u64 addr, u16 value, u16) override;
    bool MemoryWriteExclusive32(u64 addr, u32 value, u32) override;
    bool MemoryWriteExclusive64(u64 addr, u64 value, u64 expected) override;
    bool MemoryWriteExclusive128(u64 addr, Dynarmic::A64::Vector value,
                                 Dynarmic::A64::Vector expected) override;

    void
    InterpreterFallback([[maybe_unused]] u64 pc,
                        [[maybe_unused]] size_t num_instructions) override {
        LOG_FATAL(Dynarmic, "Interpreter");
    }

    void CallSVC(u32 svc) override;

    void ExceptionRaised([[maybe_unused]] u64 pc,
                         Dynarmic::A64::Exception exception) override;

    u64 GetCNTPCT() override;

    void AddTicks(u64 ticks) override {
        if (ticks > ticks_left) {
            ticks_left = 0;
            return;
        }
        ticks_left -= ticks;
    }

    u64 GetTicksRemaining() override { return ticks_left; }

    // State
    void SerializeState();
    void DeserializeState();
};

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
