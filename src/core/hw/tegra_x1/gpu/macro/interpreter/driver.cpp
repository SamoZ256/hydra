#include "core/hw/tegra_x1/gpu/macro/interpreter/driver.hpp"

namespace hydra::hw::tegra_x1::gpu::macro::interpreter {

void Driver::executeImpl(u32 pc_, u32 param1) {
    pc = pc_;
    setRegU32(1, param1);

    while (true) {
        if (parseInstruction(pc))
            break;

        if (pc == branch_after) {
            pc = branch_addr;
            branch_after = invalid<u32>();
        } else {
            pc++;
        }
    }
}

u32 Driver::instAlu(AluOperation op, u8 rA, u8 rB) {
    LOG_DEBUG(Macro, "op: {}, r{}: 0x{:08x}, r{}: 0x{:08x}", op, rA,
              getRegU32(rA), rB, getRegU32(rB));

    i32 valueA = getRegI32(rA);
    i32 valueB = getRegI32(rB);
#define RET(v) return std::bit_cast<u32>(v)
    // TODO: is carry correct?
    switch (op) {
    case AluOperation::Add: {
        i32 result = valueA + valueB;
        carry = static_cast<i8>(result < valueA);
        RET(result);
    }
    case AluOperation::AddWithCarry: {
        i32 result = valueA + valueB + carry;
        carry = static_cast<i8>(result < valueA);
        RET(result);
    }
    case AluOperation::Subtract: {
        i32 result = valueA - valueB;
        carry = static_cast<i8>(result < 0);
        RET(result);
    }
    case AluOperation::SubtractWithBorrow: {
        i32 result = valueA - valueB - carry;
        carry = static_cast<i8>(result < 0);
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

u32 Driver::instAddImmediate(u8 rA, i32 imm) {
    LOG_DEBUG(Macro, "r{}: 0x{:08x}, imm: 0x{:08x}", rA, getRegU32(rA), imm);
    return std::bit_cast<u32>(getRegI32(rA) + imm);
}

u32 Driver::instExtractInsert(u8 bA, u8 rA, u8 bB, u8 rB, u8 size) {
    LOG_DEBUG(Macro, "b{}: {}, r{}: 0x{:08x}, b{}: {}, r{}: 0x{:08x}, size: {}",
              rA, bA, rA, getRegU32(rA), rB, bB, rB, getRegU32(rB), size);
    u32 mask = (1 << size) - 1;

    // TODO: correct?
    u32 value = (getRegU32(rB) >> bB) & mask;

    return getRegU32(rA) + (value << bA);
}

u32 Driver::instExtractShiftLeftImmediate(u8 bA, u8 rA, u8 rB, u8 size) {
    LOG_FUNC_WITH_ARGS_STUBBED(
        Macro, "b{}: {}, r{}: 0x{:08x}, r{}: 0x{:08x}, size: {}", rA, bA, rA,
        getRegU32(rA), rB, getRegU32(rB), size);

    return 0;
}

u32 Driver::instExtractShiftLeftRegister(u8 rA, u8 bB, u8 rB, u8 size) {
    LOG_FUNC_WITH_ARGS_STUBBED(
        Macro, "r{}: 0x{:08x}, b{}: {}, r{}: 0x{:08x}, size: {}", rA,
        getRegU32(rA), rB, bB, rB, getRegU32(rB), size);

    return 0;
}

u32 Driver::instRead(u8 rA, u32 imm) {
    LOG_DEBUG(Macro, "r{}: 0x{:08x}, imm: 0x{:08x}", rA, getRegU32(rA), imm);
    return get3DReg(getRegU32(rA) + imm);
}

void Driver::instBranch(BranchCondition cond, u8 rA, i32 imm, bool& branched) {
    LOG_DEBUG(Macro, "cond: {}, r{}: 0x{:08x}, imm: {}", cond, rA,
              getRegU32(rA), imm);

    bool branch = false;
    bool execute_one_more = false;
    switch (cond) {
    case BranchCondition::Zero:
        branch = (getRegU32(rA) == 0);
        execute_one_more = true;
        break;
    case BranchCondition::NotZero:
        branch = (getRegU32(rA) != 0);
        execute_one_more = true;
        break;
    case BranchCondition::ZeroAnnul:
        branch = (getRegU32(rA) == 0);
        break;
    case BranchCondition::NotZeroAnnul:
        branch = (getRegU32(rA) != 0);
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

void Driver::instResult(ResultOperation op, u8 rD, u32 value) {
    LOG_DEBUG(Macro, "result op: {}, r{}, value: 0x{:08x}", op, rD, value);

    switch (op) {
    case ResultOperation::IgnoreAndFetch:
        setRegU32(rD, fetchParam());
        break;
    case ResultOperation::Move:
        setRegU32(rD, value);
        break;
    case ResultOperation::MoveAndSetMethod:
        setRegU32(rD, value);
        setMethod(value);
        break;
    case ResultOperation::FetchAndSend:
        setRegU32(rD, fetchParam());
        send(value);
        break;
    case ResultOperation::MoveAndSend:
        setRegU32(rD, value);
        send(value);
        break;
    case ResultOperation::FetchAndSetMethod:
        setRegU32(rD, fetchParam());
        setMethod(value);
        break;
    case ResultOperation::MoveAndSetMethodFetchAndSend:
        setRegU32(rD, value);
        setMethod(value);
        send(fetchParam());
        break;
    case ResultOperation::MoveAndSetMethodSend:
        setRegU32(rD, value);
        setMethod(value);
        send((value >> 12) & 0x3f);
        break;
    }
}

} // namespace hydra::hw::tegra_x1::gpu::macro::interpreter
