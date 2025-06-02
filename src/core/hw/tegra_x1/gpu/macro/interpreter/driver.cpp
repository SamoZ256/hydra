#include "core/hw/tegra_x1/gpu/macro/interpreter/driver.hpp"

namespace hydra::hw::tegra_x1::gpu::macro::interpreter {

void Driver::ExecuteImpl(u32 pc_, u32 param1) {
    pc = pc_;
    SetRegU32(1, param1);

    while (true) {
        if (ParseInstruction(pc))
            break;

        if (pc == branch_after) {
            pc = branch_addr;
            branch_after = invalid<u32>();
        } else {
            pc++;
        }
    }
}

u32 Driver::InstAlu(AluOperation op, u8 rA, u8 rB) {
    LOG_DEBUG(Macro, "op: {}, r{}: 0x{:08x}, r{}: 0x{:08x}", op, rA,
              GetRegU32(rA), rB, GetRegU32(rB));

    i32 valueA = GetRegI32(rA);
    i32 valueB = GetRegI32(rB);
#define RET(v) return std::bit_cast<u32>(v)
    // TODO: is carry correct?
    switch (op) {
    case AluOperation::Add: {
        i32 result = valueA + valueB;
        carry = result < valueA;
        RET(result);
    }
    case AluOperation::AddWithCarry: {
        i32 result = valueA + valueB + carry;
        carry = result < valueA;
        RET(result);
    }
    case AluOperation::Subtract: {
        i32 result = valueA - valueB;
        carry = result < 0;
        RET(result);
    }
    case AluOperation::SubtractWithBorrow: {
        i32 result = valueA - valueB - carry;
        carry = result < 0;
        RET(result);
    }
    case AluOperation::Xor:
        RET(valueA ^ valueB);
    case AluOperation::Or:
        RET(valueA | valueB);
    case AluOperation::And:
        RET(valueA & valueB);
    case AluOperation::AndNot:
        RET(valueA & ~valueB);
    case AluOperation::Nand:
        RET(~(valueA & valueB));
    }
}

u32 Driver::InstAddImmediate(u8 rA, i32 imm) {
    LOG_DEBUG(Macro, "r{}: 0x{:08x}, imm: 0x{:08x}", rA, GetRegU32(rA), imm);
    return std::bit_cast<u32>(GetRegI32(rA) + imm);
}

u32 Driver::InstExtractInsert(u8 bA, u8 rA, u8 bB, u8 rB, u8 size) {
    LOG_DEBUG(Macro, "b{}: {}, r{}: 0x{:08x}, b{}: {}, r{}: 0x{:08x}, size: {}",
              rA, bA, rA, GetRegU32(rA), rB, bB, rB, GetRegU32(rB), size);
    u32 mask = (1 << size) - 1;

    // TODO: correct?
    u32 value = (GetRegU32(rB) >> bB) & mask;

    return GetRegU32(rA) + (value << bA);
}

u32 Driver::InstExtractShiftLeftImmediate(u8 bA, u8 rA, u8 rB, u8 size) {
    LOG_FUNC_STUBBED(Macro);
    LOG_DEBUG(Macro, "b{}: {}, r{}: 0x{:08x}, r{}: 0x{:08x}, size: {}", rA, bA,
              rA, GetRegU32(rA), rB, GetRegU32(rB), size);

    return 0;
}

u32 Driver::InstExtractShiftLeftRegister(u8 rA, u8 bB, u8 rB, u8 size) {
    LOG_FUNC_STUBBED(Macro);
    LOG_DEBUG(Macro, "r{}: 0x{:08x}, b{}: {}, r{}: 0x{:08x}, size: {}", rA,
              GetRegU32(rA), rB, bB, rB, GetRegU32(rB), size);

    return 0;
}

u32 Driver::InstRead(u8 rA, u32 imm) {
    LOG_DEBUG(Macro, "r{}: 0x{:08x}, imm: 0x{:08x}", rA, GetRegU32(rA), imm);
    return Get3DReg(GetRegU32(rA) + imm);
}

void Driver::InstBranch(BranchCondition cond, u8 rA, i32 imm, bool& branched) {
    LOG_DEBUG(Macro, "cond: {}, r{}: 0x{:08x}, imm: {}", cond, rA,
              GetRegU32(rA), imm);

    bool branch = false;
    bool execute_one_more = false;
    switch (cond) {
    case BranchCondition::Zero:
        branch = (GetRegU32(rA) == 0);
        execute_one_more = true;
        break;
    case BranchCondition::NotZero:
        branch = (GetRegU32(rA) != 0);
        execute_one_more = true;
        break;
    case BranchCondition::ZeroAnnul:
        branch = (GetRegU32(rA) == 0);
        break;
    case BranchCondition::NotZeroAnnul:
        branch = (GetRegU32(rA) != 0);
        break;
    }

    if (branch) {
        branch_after = pc + (execute_one_more ? 1 : 0);
        branch_addr = std::bit_cast<u32>(std::bit_cast<i32>(pc) + imm);
        branched = true;
    } else {
        branched = false;
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
        SetMethod(value);
        break;
    case ResultOperation::FetchAndSend:
        SetRegU32(rD, FetchParam());
        Send(value);
        break;
    case ResultOperation::MoveAndSend:
        SetRegU32(rD, value);
        Send(value);
        break;
    case ResultOperation::FetchAndSetMethod:
        SetRegU32(rD, FetchParam());
        SetMethod(value);
        break;
    case ResultOperation::MoveAndSetMethodFetchAndSend:
        SetRegU32(rD, value);
        SetMethod(value);
        Send(FetchParam());
        break;
    case ResultOperation::MoveAndSetMethodSend:
        SetRegU32(rD, value);
        SetMethod(value);
        Send((value >> 12) & 0x3f);
        break;
    }
}

} // namespace hydra::hw::tegra_x1::gpu::macro::interpreter
