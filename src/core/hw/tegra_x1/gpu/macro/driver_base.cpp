#include "core/hw/tegra_x1/gpu/macro/driver_base.hpp"
#include "core/hw/tegra_x1/gpu/engines/3d.hpp"
#include "core/hw/tegra_x1/gpu/macro/const.hpp"

namespace hydra::hw::tegra_x1::gpu::macro {

void DriverBase::Execute() {
    ExecuteImpl(start_address_ram[index], param1);

    // TODO: what should happen when there are still parameters in the queue?
    if (!param_queue.empty()) {
        LOG_ERROR(Macro, "There are still parameters ({}) in the queue",
                  param_queue.size());
        std::queue<u32>().swap(param_queue);
    }
}

void DriverBase::LoadInstructionRamPointer(u32 ptr) {
    instruction_ram_ptr = ptr;
}

void DriverBase::LoadInstructionRam(u32 data) {
    instruction_ram[instruction_ram_ptr++] = data;
}

void DriverBase::LoadStartAddressRamPointer(u32 ptr) {
    start_address_ram_ptr = ptr;
}

void DriverBase::LoadStartAddressRam(u32 data) {
    start_address_ram[start_address_ram_ptr++] = data;
}

bool DriverBase::ParseInstruction(u32 pc) {
    u32 instruction = instruction_ram[pc];
    // LOG_DEBUG(Macro, "PC: 0x{:08x}, instruction: 0x{:08x}", pc, instruction);

#define GET_DATA_IMPL(inst, shift, bit_count)                                  \
    ((inst >> shift) & ((1 << bit_count) - 1))
#define GET_DATA_U32(shift, mask) GET_DATA_IMPL(instruction, shift, mask)
#define GET_DATA_I32(shift, mask)                                              \
    GET_DATA_IMPL(std::bit_cast<i32>(instruction), shift, mask)
#define GET_REG(shift) GET_DATA_U32(shift, 3)
// TODO: rename
#define GET_B(shift) GET_DATA_U32(shift, 5)
#define GET_IMM_U32() GET_DATA_U32(14, 18)
#define GET_IMM_I32() sign_extend<i32, 18>(GET_DATA_I32(14, 18))
#define GET_SIZE(shift) GET_DATA_U32(shift, 5)

    // Operation
    Operation op = static_cast<Operation>(instruction & 0x7);
    u32 value;
    bool branched = false;
    switch (op) {
    case Operation::Alu: {
        auto alu_op = static_cast<AluOperation>(GET_DATA_U32(17, 4));
        u8 rA = GET_REG(11);
        u8 rB = GET_REG(14);
        value = InstAlu(alu_op, rA, rB);
        break;
    }
    case Operation::AddImmediate: {
        u8 rA = GET_REG(11);
        i32 imm = GET_IMM_I32();
        value = InstAddImmediate(rA, imm);
        break;
    }
    case Operation::ExtractInsert: {
        u8 bA = GET_B(27);
        u8 rA = GET_REG(11);
        u8 bB = GET_B(17);
        u8 rB = GET_REG(14);
        u8 size = GET_SIZE(22);
        value = InstExtractInsert(bA, rA, bB, rB, size);
        break;
    }
    case Operation::ExtractShiftLeftImmediate: {
        u8 bA = GET_B(27);
        u8 rA = GET_REG(11);
        u8 rB = GET_REG(14);
        u8 size = GET_SIZE(22);
        value = InstExtractShiftLeftImmediate(bA, rA, rB, size);
        break;
    }
    case Operation::ExtractShiftLeftRegister: {
        u8 rA = GET_REG(11);
        u8 bB = GET_B(17);
        u8 rB = GET_REG(14);
        u8 size = GET_SIZE(22);
        value = InstExtractShiftLeftRegister(rA, bB, rB, size);
        break;
    }
    case Operation::Read: {
        u8 rA = GET_REG(11);
        u32 imm = GET_IMM_U32();
        value = InstRead(rA, imm);
        break;
    }
    case Operation::Branch: {
        auto cond = static_cast<BranchCondition>(GET_DATA_U32(4, 2));
        u8 rA = GET_REG(11);
        i32 imm = GET_IMM_I32();
        InstBranch(cond, rA, imm, branched);
        break;
    }
    }

    // result_t operation
    if (op != Operation::Branch) {
        ResultOperation result_op =
            static_cast<ResultOperation>(GET_DATA_U32(4, 3));
        u8 rD = GET_REG(8);
        InstResult(result_op, rD, value);
    }

    // Check if exit
    // TODO: why is this so weird?
    if (instruction & EXIT_BIT && !branched)
        exit_after = pc + 1;

    return pc == exit_after;
}

u32 DriverBase::Get3DReg(u32 reg_3d) { return engine_3d->GetReg(reg_3d); }

void DriverBase::SetMethod(u32 value) {
    method = value & 0xfff;
    increment = (value >> 12) & 0x3f;
}

void DriverBase::Send(u32 arg) {
    engine_3d->Method(method, arg);
    method += increment;
}

} // namespace hydra::hw::tegra_x1::gpu::macro
