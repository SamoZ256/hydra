#pragma once

#include <memory>

namespace hydra::debugger {

constexpr usize INSTRUCTION_SIZE = sizeof(u32);

enum class FlowKind {
    None,
    Branch,
    ConditionalBranch,
    Call,
    IndirectBranch,
    IndirectCall,
    Return,
    Trap,
    Syscall,
};

struct Instruction {
    vaddr_t addr;
    u32 word;
    bool valid{false};
    std::string mnemonic;
    std::string operands;
    FlowKind flow{FlowKind::None};
    std::optional<vaddr_t> target{std::nullopt};
    std::optional<vaddr_t> data_ref{std::nullopt};
};

class IDisassembler {
  public:
    virtual ~IDisassembler() = default;

    virtual std::vector<Instruction> disassemble(vaddr_t addr,
                                                 std::span<const u8> code) = 0;
};

std::unique_ptr<IDisassembler> createDisassembler();

} // namespace hydra::debugger

ENABLE_ENUM_FORMATTING(hydra::debugger::FlowKind, None, "none", Branch,
                       "branch", ConditionalBranch, "conditional branch", Call,
                       "call", IndirectBranch, "indirect branch", IndirectCall,
                       "indirect call", Return, "return", Trap, "trap", Syscall,
                       "syscall")
