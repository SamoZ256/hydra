#include "core/debugger/capstone/disassembler.hpp"

namespace hydra::debugger::capstone {

namespace {

FlowKind flowKindFromInstruction(aarch64_insn id, const cs_aarch64& detail) {
    switch (id) {
    case AARCH64_INS_B:
        return detail.cc == AArch64CC_Invalid ? FlowKind::Branch
                                              : FlowKind::ConditionalBranch;
    case AARCH64_INS_BC:
    case AARCH64_INS_CBZ:
    case AARCH64_INS_CBNZ:
    case AARCH64_INS_TBZ:
    case AARCH64_INS_TBNZ:
        return FlowKind::ConditionalBranch;
    case AARCH64_INS_BL:
        return FlowKind::Call;
    case AARCH64_INS_BR:
    case AARCH64_INS_BRAA:
    case AARCH64_INS_BRAAZ:
    case AARCH64_INS_BRAB:
    case AARCH64_INS_BRABZ:
        return FlowKind::IndirectBranch;
    case AARCH64_INS_BLR:
    case AARCH64_INS_BLRAA:
    case AARCH64_INS_BLRAAZ:
    case AARCH64_INS_BLRAB:
    case AARCH64_INS_BLRABZ:
        return FlowKind::IndirectCall;
    case AARCH64_INS_RET:
    case AARCH64_INS_RETAA:
    case AARCH64_INS_RETAB:
    case AARCH64_INS_ERET:
    case AARCH64_INS_ERETAA:
    case AARCH64_INS_ERETAB:
    case AARCH64_INS_DRPS:
        return FlowKind::Return;
    case AARCH64_INS_BRK:
    case AARCH64_INS_UDF:
    case AARCH64_INS_HLT:
        return FlowKind::Trap;
    case AARCH64_INS_SVC:
    case AARCH64_INS_HVC:
    case AARCH64_INS_SMC:
        return FlowKind::Syscall;
    default:
        return FlowKind::None;
    }
}

bool hasDirectTarget(FlowKind flow) {
    return flow == FlowKind::Branch || flow == FlowKind::ConditionalBranch ||
           flow == FlowKind::Call;
}

std::optional<vaddr_t> lastImmediate(const cs_aarch64& detail) {
    for (u8 i = detail.op_count; i > 0; i--) {
        const auto& operand = detail.operands[i - 1];
        if (operand.type == AARCH64_OP_IMM)
            return static_cast<vaddr_t>(operand.imm);
    }

    return std::nullopt;
}

std::optional<vaddr_t> literalAddress(const cs_aarch64& detail) {
    for (u8 i = 0; i < detail.op_count; i++) {
        const auto& operand = detail.operands[i];
        if (operand.type != AARCH64_OP_MEM)
            continue;

        if (operand.mem.base != AARCH64_REG_INVALID ||
            operand.mem.index != AARCH64_REG_INVALID)
            return std::nullopt;

        return static_cast<vaddr_t>(operand.mem.disp);
    }

    return std::nullopt;
}

std::optional<vaddr_t> dataReference(aarch64_insn id,
                                     const cs_aarch64& detail) {
    switch (id) {
    case AARCH64_INS_ADR:
    case AARCH64_INS_ADRP:
        return lastImmediate(detail);
    case AARCH64_INS_LDR:
    case AARCH64_INS_LDRSW:
    case AARCH64_INS_PRFM:
        return literalAddress(detail);
    default:
        return std::nullopt;
    }
}

} // namespace

std::unique_ptr<IDisassembler> Disassembler::create() {
    csh handle;
    const auto err = cs_open(CS_ARCH_AARCH64, CS_MODE_LITTLE_ENDIAN, &handle);
    if (err != CS_ERR_OK) {
        LOG_ERROR(Debugger, "Failed to open capstone: {}", cs_strerror(err));
        return nullptr;
    }

    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
    auto insn = cs_malloc(handle);
    if (insn == nullptr) {
        cs_close(&handle);
        return nullptr;
    }

    return std::make_unique<Disassembler>(handle, insn);
}

std::vector<Instruction> Disassembler::disassemble(vaddr_t addr,
                                                   std::span<const u8> code) {
    std::vector<Instruction> instructions;
    instructions.reserve(code.size() / INSTRUCTION_SIZE);
    for (usize offset = 0; offset + INSTRUCTION_SIZE <= code.size();
         offset += INSTRUCTION_SIZE) {
        const auto word_bytes = code.subspan(offset, INSTRUCTION_SIZE);
        u32 word;
        std::memcpy(&word, word_bytes.data(), sizeof(word));

        Instruction instruction{.addr = addr + offset, .word = word};
        decode(word_bytes, instruction);
        instructions.push_back(std::move(instruction));
    }

    return instructions;
}

void Disassembler::decode(std::span<const u8> word_bytes,
                          Instruction& instruction) {
    const u8* code = word_bytes.data();
    usize size = word_bytes.size();
    u64 address = instruction.addr;
    if (!cs_disasm_iter(handle, &code, &size, &address, insn)) {
        instruction.mnemonic = ".inst";
        instruction.operands = fmt::format("{:#010x}", instruction.word);
        return;
    }

    instruction.valid = true;
    instruction.mnemonic = insn->mnemonic;
    instruction.operands = insn->op_str;

    const auto id = static_cast<aarch64_insn>(insn->id);
    const auto& detail = insn->detail->aarch64;
    instruction.flow = flowKindFromInstruction(id, detail);
    if (hasDirectTarget(instruction.flow))
        instruction.target = lastImmediate(detail);
    instruction.data_ref = dataReference(id, detail);
}

} // namespace hydra::debugger::capstone
