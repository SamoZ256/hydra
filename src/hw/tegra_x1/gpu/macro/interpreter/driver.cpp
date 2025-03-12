#include "hw/tegra_x1/gpu/macro/interpreter/driver.hpp"

namespace Hydra::HW::TegraX1::GPU::Macro::Interpreter {

void Driver::ExecuteImpl(u32 pc_, u32 param1) {
    pc = pc_;
    SetRegU32(1, param1);

    while (true) {
        if (ParseInstruction(pc))
            break;

        if (increment_pc)
            pc++;
        else
            increment_pc = true;
    }
}

u32 Driver::InstAlu(AluOperation op, u8 rA, u8 rB) {
    LOG_FUNC_NOT_IMPLEMENTED(Macro);
    LOG_DEBUG(Macro, "op: {}, r{}: 0x{:08x}, r{}: 0x{:08x}", op, rA,
              GetRegU32(rA), rB, GetRegU32(rB));

    return 0;
}

u32 Driver::InstAddImmediate(u8 rA, i32 imm) {
    LOG_DEBUG(Macro, "r{}: 0x{:08x}, imm: 0x{:08x}", rA, GetRegU32(rA), imm);
    return bit_cast<u32>(GetRegI32(rA) + imm);
}

u32 Driver::InstExtractInsert(u8 bA, u8 rA, u8 bB, u8 rB, u8 size) {
    LOG_DEBUG(Macro, "b{}: {}, r{}: 0x{:08x}, b{}: {}, r{}: 0x{:08x}, size: {}",
              rA, bA, rA, GetRegU32(rA), rB, bB, rB, GetRegU32(rB), size);
    u32 mask = (1 << size) - 1;
    u32 valueA = (GetRegU32(rA) >> bA) & mask; // TODO: correct?
    u32 valueB = (GetRegU32(rB) >> bB) & mask;

    // TODO: how to combine these?
    return valueA | valueB;
}

u32 Driver::InstExtractShiftLeftImmediate(u8 bA, u8 rA, u8 rB, u8 size) {
    LOG_FUNC_NOT_IMPLEMENTED(Macro);
    LOG_DEBUG(Macro, "b{}: {}, r{}: 0x{:08x}, r{}: 0x{:08x}, size: {}", rA, bA,
              rA, GetRegU32(rA), rB, GetRegU32(rB), size);

    return 0;
}

u32 Driver::InstExtractShiftLeftRegister(u8 rA, u8 bB, u8 rB, u8 size) {
    LOG_FUNC_NOT_IMPLEMENTED(Macro);
    LOG_DEBUG(Macro, "r{}: 0x{:08x}, b{}: {}, r{}: 0x{:08x}, size: {}", rA,
              GetRegU32(rA), rB, bB, rB, GetRegU32(rB), size);

    return 0;
}

u32 Driver::InstRead(u8 rA, u32 imm) {
    LOG_DEBUG(Macro, "r{}: 0x{:08x}, imm: 0x{:08x}", rA, GetRegU32(rA), imm);
    return Get3DReg(imm) << GetRegU32(rA); // TODO: correct?
}

void Driver::InstBranch(BranchCondition cond, u8 rA, i32 imm) {
    LOG_DEBUG(Macro, "cond: {}, r{}: 0x{:08x}, imm: {}", cond, rA,
              GetRegU32(rA), imm);

    bool branch = false;
    switch (cond) {
    case BranchCondition::Zero:
        branch = (GetRegU32(rA) == 0);
        break;
    case BranchCondition::NotZero:
        branch = (GetRegU32(rA) != 0);
        break;
    case BranchCondition::ZeroAnnul:
        LOG_NOT_IMPLEMENTED(Macro, "ZeroAnnul");
        break;
    case BranchCondition::NotZeroAnnul:
        LOG_NOT_IMPLEMENTED(Macro, "NotZeroAnnul");
        break;
    }

    if (branch) {
        pc = bit_cast<u32>(bit_cast<i32>(pc) + imm);
        increment_pc = false;
    }
}

void Driver::InstResult(ResultOperation op, u8 rD, u32 value) {
    LOG_DEBUG(Macro, "result op: {}, r{}, value: 0x{:08x}", op, rD, value);

    switch (op) {
    case ResultOperation::IgnoreAndFetch:
        SetRegU32(rD, FetchParam());
        break;
    case ResultOperation::Move:
        SetRegU32(rD, value);
        break;
    case ResultOperation::MoveAndSetMethod:
        SetRegU32(rD, value);
        Method(value);
        break;
    case ResultOperation::FetchAndSend:
        LOG_NOT_IMPLEMENTED(Macro, "FetchAndSend");
        SetRegU32(rD, FetchParam());
        break;
    case ResultOperation::MoveAndSend:
        LOG_NOT_IMPLEMENTED(Macro, "MoveAndSend");
        break;
    case ResultOperation::FetchAndSetMethod:
        SetRegU32(rD, FetchParam());
        Method(value);
        break;
    case ResultOperation::MoveAndSetMethodFetchAndSend:
        LOG_NOT_IMPLEMENTED(Macro, "MoveAndSetMethodFetchAndSend");
        break;
    case ResultOperation::MoveAndSetMethodSend:
        LOG_NOT_IMPLEMENTED(Macro, "MoveAndSetMethodSend");
        break;
    }
}

} // namespace Hydra::HW::TegraX1::GPU::Macro::Interpreter
