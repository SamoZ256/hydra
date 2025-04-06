#pragma once

#include "common/common.hpp"

namespace Hydra::HW::TegraX1::CPU {

class MemoryBase;

class ThreadBase {
  public:
    ThreadBase(MemoryBase* tls_mem_) : tls_mem{tls_mem_} {}
    virtual ~ThreadBase() {}

    virtual void Configure(const std::function<bool(ThreadBase*, u64)>&
                               svc_handler,
                           uptr tls_mem_base /*,
      uptr rom_mem_base*/, uptr stack_mem_end) = 0;

    virtual void Run() = 0;

    virtual u64 GetRegX(u8 reg) const = 0;
    virtual void SetRegX(u8 reg, u64 value) = 0;
    virtual void SetRegPC(u64 value) = 0;

    u32 GetRegW(u8 reg) const { return static_cast<u32>(GetRegX(reg)); }
    void SetRegW(u8 reg, u32 value) { SetRegX(reg, static_cast<u64>(value)); }

    virtual void LogRegisters(bool simd = false, u32 count = 32) = 0;

    // Getters
    MemoryBase* GetTlsMemory() const { return tls_mem; }

  private:
    MemoryBase* tls_mem;
};

} // namespace Hydra::HW::TegraX1::CPU
