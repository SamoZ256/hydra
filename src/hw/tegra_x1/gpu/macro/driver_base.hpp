#pragma once

#include "common/logging/log.hpp"
#include "hw/tegra_x1/gpu/macro/const.hpp"

namespace Hydra::HW::TegraX1::GPU::Macro {

struct Result {
    ResultOperation op;
    u8 rD;
};

class DriverBase {
  public:
    DriverBase(u32* regs_3d_) : regs_3d{regs_3d_} {}

    void Execute();

    void LoadInstructionRamPointer(u32 ptr);
    void LoadInstructionRam(u32 data);
    void LoadStartAddressRamPointer(u32 ptr);
    void LoadStartAddressRam(u32 data);

    void SetIndex(u32 index_) { index = index_; }
    void LoadParam1(u32 data) { param1 = data; }
    void LoadParam(u32 data) { param_queue.push(data); }

  protected:
    virtual void ExecuteImpl(u32 pc, u32 param1) = 0;

    virtual u32 InstAlu(AluOperation op, u8 rA, u8 rB) = 0;
    virtual u32 InstAddImmediate(u8 rA, i32 imm) = 0;
    virtual u32 InstExtractInsert(u8 bA, u8 rA, u8 bB, u8 rB, u8 size) = 0;
    virtual u32 InstExtractShiftLeftImmediate(u8 bA, u8 rA, u8 rB, u8 size) = 0;
    virtual u32 InstExtractShiftLeftRegister(u8 rA, u8 bB, u8 rB, u8 size) = 0;
    virtual u32 InstRead(u8 rA, u32 imm) = 0;
    virtual void InstBranch(BranchCondition cond, u8 rA, i32 imm) = 0;
    virtual void InstResult(ResultOperation op, u8 rD, u32 value) = 0;

    bool ParseInstruction(u32 pc);

    u32 FetchParam() {
        ASSERT_DEBUG(!param_queue.empty(), Macro, "Parameter queue is empty");

        u32 param = param_queue.front();
        param_queue.pop();

        return param;
    }

    u32& Get3DReg(u32 reg_3d) {
        ASSERT_DEBUG(reg_3d < MACRO_METHODS_REGION, Macro,
                     "Invalid 3D register {}", reg_3d);
        return regs_3d[reg_3d];
    }

  private:
    u32* regs_3d;

    // Memory
    u32 instruction_ram[0x1000] = {0}; // TODO: what should be the size?
    u32 instruction_ram_ptr;
    u32 start_address_ram[0x100] = {0}; // TODO: what should be the size?
    u32 start_address_ram_ptr;

    // Parameters
    u32 index;
    u32 param1;
    std::queue<u32> param_queue;

    // Exit
    u32 exit_after = 0;
};

} // namespace Hydra::HW::TegraX1::GPU::Macro
