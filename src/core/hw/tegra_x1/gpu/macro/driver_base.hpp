#pragma once

#include "core/hw/tegra_x1/gpu/macro/const.hpp"

namespace hydra::hw::tegra_x1::gpu {
class GMmu;
}

namespace hydra::hw::tegra_x1::gpu::engines {
class ThreeD;
}

namespace hydra::hw::tegra_x1::gpu::macro {

struct result_t {
    ResultOperation op;
    u8 rD;
};

class DriverBase {
  public:
    explicit DriverBase(engines::ThreeD& engine_3d_) : engine_3d{engine_3d_} {}
    virtual ~DriverBase() = default;

    void execute();

    void loadInstructionRamPointer(u32 ptr);
    void loadInstructionRam(u32 data);
    void loadStartAddressRamPointer(u32 ptr);
    void loadStartAddressRam(u32 data);

    void setIndex(u32 index_) { index = index_; }
    void loadParam1(u32 data) { param1 = data; }
    void loadParam(u32 data) { param_queue.push(data); }

  protected:
    virtual void executeImpl(u32 pc, u32 param1) = 0;

    virtual u32 instAlu(AluOperation op, u8 rA, u8 rB) = 0;
    virtual u32 instAddImmediate(u8 rA, i32 imm) = 0;
    virtual u32 instExtractInsert(u8 bA, u8 rA, u8 bB, u8 rB, u8 size) = 0;
    virtual u32 instExtractShiftLeftImmediate(u8 bA, u8 rA, u8 rB, u8 size) = 0;
    virtual u32 instExtractShiftLeftRegister(u8 rA, u8 bB, u8 rB, u8 size) = 0;
    virtual u32 instRead(u8 rA, u32 imm) = 0;
    virtual void instBranch(BranchCondition cond, u8 rA, i32 imm,
                            bool& branched) = 0;
    virtual void instResult(ResultOperation op, u8 rD, u32 value) = 0;

    bool parseInstruction(u32 pc);

    u32 fetchParam() {
        ASSERT_DEBUG(!param_queue.empty(), Macro, "Parameter queue is empty");

        u32 param = param_queue.front();
        param_queue.pop();

        return param;
    }

    u32 get3DReg(u32 reg_3d);
    void setMethod(u32 value);
    void send(u32 arg);

  private:
    engines::ThreeD& engine_3d;

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
    u32 exit_after = invalid<u32>();

    // Method
    u32 method = invalid<u32>();
    u32 increment = 0;
};

} // namespace hydra::hw::tegra_x1::gpu::macro
