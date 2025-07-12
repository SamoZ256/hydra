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
    Thread(IMmu* mmu, const svc_handler_fn_t& svc_handler,
           const stop_requested_fn_t& stop_requested, IMemory* tls_mem,
           vaddr_t tls_mem_base, vaddr_t stack_mem_end);
    ~Thread() override;

    void Run() override;

    u64 GetRegX(u8 reg) const override { return jit->GetRegister(reg); }
    void SetRegX(u8 reg, u64 value) override { jit->SetRegister(reg, value); }
    u64 GetPC() override { return jit->GetPC(); }
    void SetPC(u64 value) override { jit->SetPC(value); }
    u64 GetFP() override { return jit->GetRegister(29); }
    u64 GetLR() override { return jit->GetRegister(30); }
    u64 GetSP() override { return jit->GetSP(); }
    u64 GetElr() override {
        // TODO
        return 0x0;
    }

    // Debug
    void LogRegisters(bool simd = false, u32 count = 32) override;

  private:
    u64 tpidrro_el0;

    Dynarmic::A64::Jit* jit;
    u64 ticks_left = 1000;

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

    void InterpreterFallback(u64 pc, size_t num_instructions) override {
        LOG_FATAL(Dynarmic, "Interpreter");
    }

    void CallSVC(u32 svc) override;

    void ExceptionRaised(u64 pc, Dynarmic::A64::Exception exception) override;

    u64 GetCNTPCT() override;

    void AddTicks(u64 ticks) override {
        if (ticks > ticks_left) {
            ticks_left = 0;
            return;
        }
        ticks_left -= ticks;
    }

    u64 GetTicksRemaining() override { return ticks_left; }

    // Helpers
    void CheckForStopRequest() {
        if (stop_requested())
            jit->HaltExecution();
    }
};

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
