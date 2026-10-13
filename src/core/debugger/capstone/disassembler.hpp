#pragma once

#include <capstone/capstone.h>

#include "core/debugger/disassembler.hpp"

namespace hydra::debugger::capstone {

class Disassembler final : public IDisassembler {
  public:
    static std::unique_ptr<IDisassembler> create();

    Disassembler(csh handle_, cs_insn* insn_) : handle{handle_}, insn{insn_} {}
    ~Disassembler() override {
        cs_free(insn, 1);
        cs_close(&handle);
    }

    ZTD_MAKE_NON_COPYABLE(Disassembler);
    ZTD_MAKE_NON_MOVABLE(Disassembler);

    std::vector<Instruction> disassemble(vaddr_t addr,
                                         std::span<const u8> code) override;

  private:
    csh handle;
    cs_insn* insn;

    void decode(std::span<const u8> word_bytes, Instruction& instruction);
};

} // namespace hydra::debugger::capstone
