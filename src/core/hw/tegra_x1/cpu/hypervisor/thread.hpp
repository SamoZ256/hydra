#pragma once

#include "core/hw/tegra_x1/cpu/hypervisor/const.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"

namespace hydra::horizon {
class OS;
}

namespace hydra::hw::tegra_x1::cpu {
class IMemory;
}

namespace hydra::hw::tegra_x1::cpu::hypervisor {

class Cpu;
class Mmu;

constexpr u32 MAX_BREAKPOINTS = 16;

enum class ThreadMessageType {
    InsertBreakpoint,
    RemoveBreakpoint,
    SingleStep,
};

struct ThreadMessage {
    ThreadMessageType type;
    union {
        // Insert breakpoint
        struct {
            vaddr_t addr;
        } insert_breakpoint;

        // Remove breakpoint
        struct {
            vaddr_t addr;
        } remove_breakpoint;
    } payload{};
};

class Thread : public IThread {
  public:
    Thread(WallClock& wall_clock, Cpu& cpu_, IMmu* mmu,
           const ThreadCallbacks& callbacks, IMemory* tls_mem,
           vaddr_t tls_mem_base);
    ~Thread() override;

    void run() override;

    void setupVTimer();

    void updateVTimer();

    // Debug
    void insertBreakpoint(vaddr_t addr) override {
        sendMessage({.type = ThreadMessageType::InsertBreakpoint,
                     .payload = {.insert_breakpoint = {addr}}});
    }
    void removeBreakpoint(vaddr_t addr) override {
        sendMessage({.type = ThreadMessageType::RemoveBreakpoint,
                     .payload = {.remove_breakpoint = {addr}}});
    }
    void singleStep() override {
        sendMessage({.type = ThreadMessageType::SingleStep});
    }

  private:
    Cpu& cpu;

    hv_vcpu_t vcpu;
    hv_vcpu_exit_t* exit;
    bool exception{false};

    u64 interrupt_time_delta_ticks;

    // Debug
    vaddr_t breakpoints[MAX_BREAKPOINTS] = {0x0};

    // Messages
    std::mutex msg_mutex;
    std::queue<ThreadMessage> msg_queue;

    // State
    void serializeState();
    void deserializeState();

    void instructionTrap(u32 esr);

    // Helpers
    u64 getReg(hv_reg_t reg) const {
        u64 value;
        HV_ASSERT_SUCCESS(hv_vcpu_get_reg(vcpu, reg, &value));

        return value;
    }

    void setReg(hv_reg_t reg, u64 value) const {
        HV_ASSERT_SUCCESS(hv_vcpu_set_reg(vcpu, reg, value));
    }

    u128 getSimdFpReg(u8 reg) const {
        hv_simd_fp_uchar16_t value;
        HV_ASSERT_SUCCESS(hv_vcpu_get_simd_fp_reg(
            vcpu, static_cast<hv_simd_fp_reg_t>(HV_SIMD_FP_REG_Q0 + reg),
            &value));

        // TODO: correct?
        return std::bit_cast<u128>(value);
    }

    void setSimdFpReg(u8 reg, u128 value) const {
        // TODO: correct?
        HV_ASSERT_SUCCESS(hv_vcpu_set_simd_fp_reg(
            vcpu, static_cast<hv_simd_fp_reg_t>(HV_SIMD_FP_REG_Q0 + reg),
            std::bit_cast<hv_simd_fp_uchar16_t>(value)));
    }

    u64 getSysReg(hv_sys_reg_t reg) const {
        u64 value;
        HV_ASSERT_SUCCESS(hv_vcpu_get_sys_reg(vcpu, reg, &value));

        return value;
    }

    void setSysReg(hv_sys_reg_t reg, u64 value) const {
        HV_ASSERT_SUCCESS(hv_vcpu_set_sys_reg(vcpu, reg, value));
    }

    // Messages
    void sendMessage(const ThreadMessage& message) {
        std::scoped_lock lock(msg_mutex);
        msg_queue.push(message);
    }

    void processMessages();
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
