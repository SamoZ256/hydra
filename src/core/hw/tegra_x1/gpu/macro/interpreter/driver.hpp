#pragma once

#include "core/hw/tegra_x1/gpu/macro/driver_base.hpp"

namespace hydra::hw::tegra_x1::gpu::macro::interpreter {

class Driver : public DriverBase {
  public:
    explicit Driver(engines::ThreeD& engine_3d) : DriverBase(engine_3d) {}

  protected:
    void executeImpl(u32 pc_, u32 param1) override;

    u32 instAlu(AluOperation op, u8 rA, u8 rB) override;
    u32 instAddImmediate(u8 rA, i32 imm) override;
    u32 instExtractInsert(u8 bA, u8 rA, u8 bB, u8 rB, u8 size) override;
    u32 instExtractShiftLeftImmediate(u8 bA, u8 rA, u8 rB, u8 size) override;
    u32 instExtractShiftLeftRegister(u8 rA, u8 bB, u8 rB, u8 size) override;
    u32 instRead(u8 rA, u32 imm) override;
    void instBranch(BranchCondition cond, u8 rA, i32 imm,
                    bool& branched) override;
    void instResult(ResultOperation op, u8 rD, u32 value) override;

  private:
    u32 pc;
    u32 regs[REG_COUNT] = {0};

    i8 carry = 0;

    u32 branch_after = invalid<u32>();
    u32 branch_addr;

    // Helpers
    u32& getRegRaw(u8 reg) {
        ASSERT_DEBUG(reg < REG_COUNT, Macro, "Invalid register {}", reg);
        return regs[reg];
    }

    u32 getRegU32(u8 reg) {
        ASSERT_DEBUG(reg < REG_COUNT, Macro, "Invalid register {}", reg);
        return getRegRaw(reg);
    }

    void setRegU32(u8 reg, u32 value) {
        ASSERT_DEBUG(reg < REG_COUNT, Macro, "Invalid register {}", reg);

        if (reg == 0)
            return;

        regs[reg] = value;
    }

    i32 getRegI32(u8 reg) {
        ASSERT_DEBUG(reg < REG_COUNT, Macro, "Invalid register {}", reg);
        return std::bit_cast<i32>(getRegRaw(reg));
    }

    void setRegI32(u8 reg, i32 value) {
        ASSERT_DEBUG(reg < REG_COUNT, Macro, "Invalid register {}", reg);

        if (reg == 0)
            return;

        regs[reg] = std::bit_cast<u32>(value);
    }
};

} // namespace hydra::hw::tegra_x1::gpu::macro::interpreter
